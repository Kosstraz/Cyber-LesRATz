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

static int	commandline_finished = 0;

char*	get_own_addr(void)
{
	char			*ipv4;
	char			buffer[256];
	struct hostent	*hostaddr;

	gethostname(buffer, sizeof(buffer));
	hostaddr = gethostbyname(buffer);
	ipv4 = inet_ntoa(*(struct in_addr *)hostaddr->h_addr_list[0]);
	return (ipv4);
}

void	init_server(s_blue* blue)
{
	if ((blue->ratz.server = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		strexit("socket", 1);
	write(1, blue->ratz.addr, strlen(blue->ratz.addr));
	write(1, "\n", 1);
	ratz_set_addri(&blue->ratz.addri, AF_INET, blue->ratz.addr, blue->ratz.raw_port);
	if (bind(blue->ratz.server, (const s_sockaddr*)&blue->ratz.addri, sizeof(blue->ratz.addri)) == -1)
		strexit("bind", 1);
	else if (listen(blue->ratz.server, 1) == -1)
		strexit("listen", 1);
	else if ((blue->client = accept(blue->ratz.server, (s_sockaddr*)&blue->ratz.addri, &blue->ratz.addri_len)) == -1)
		strexit("accept", 1);
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
int	set_ttctrl(int fd)
{
	return (ioctl(fd, TIOCSCTTY, 0));
}

void	setup_slave(s_blue* blue)
{
	if (setsid() == -1) // Détache le programme de l'ancienne session, auquel il devient maintenant le leader
		blue->auth.setsid = 0;
	open_pts(blue); // Ouvre le terminal esclave
	if (set_ttctrl(blue->slave) == -1) // Le terminal associé au 'fd' devient le terminal de controle de la session
		blue->auth.ioctl = 0;
}

void	set_slave_state(int slave_fd)
{
	dup2(slave_fd, STDIN_FILENO);
	dup2(slave_fd, STDOUT_FILENO);
	dup2(slave_fd, STDERR_FILENO);
	close(slave_fd);
}

void	try_set_root(s_auth* auth)
{
	if (setuid(0) == -1)
		auth->pr = 0;
	if (setgid(0) == -1)
		auth->gr = 0;
}

char*	strjoin(char* a, char* b)
{
	unsigned long	a_size = strlen(a);
	unsigned long	b_size = strlen(b);
	unsigned int	i = 0;
	char*			new = malloc((a_size + b_size + 1) * sizeof(char));

	for ( ; i < a_size ; ++i)
		new[i] = a[i];
	for (unsigned int j = 0 ; j < b_size ; ++j, ++i)
		new[i] = b[j];
	new[i] = 0;
	return (new);
}

void	sigusr1_handling(int sig)
{
	if (sig == SIGUSR1)
		commandline_finished = 1;	
}

void	init_auth(s_auth* auth)
{
	auth->pr = 1;
	auth->gr = 1;
	auth->ioctl = 1;
	auth->setsid = 1;
}

void	send_auth_error(s_blue blue)
{
	char	auth_msg[11] = {0};
	int		n = 0;

	if (blue.auth.pr == 0)
	{
		auth_msg[0] = 'p';
		auth_msg[1] = 'r';
		n += 2;
	}
	if (blue.auth.gr == 0)
	{
		auth_msg[n + 0] = 'g';
		auth_msg[n + 1] = 'r';
		n += 2;
	}
	if (blue.auth.setsid == 0)
	{
		auth_msg[n + 0] = 's';
		auth_msg[n + 1] = 'i';
		auth_msg[n + 2] = 'd';
		n += 3;
	}
	if (blue.auth.ioctl == 0)
	{
		auth_msg[n + 0] = 'c';
		auth_msg[n + 1] = 't';
		auth_msg[n + 2] = 'l';
		n += 3;
	}
	write(blue.client, auth_msg, 11);
	write(blue.client, "\1", 2);
}

void	wait_new_connection(s_blue* blue)
{
	
}

int	main(int unused, char** av)
{
	(void)unused;

	int		fdsave[2];
	int		chd;
	s_blue	blue;
	memset(&blue, 0, sizeof(blue));

	init_auth(&blue.auth);
	try_set_root(&blue.auth);
	blue.ratz.addr = get_own_addr();
	blue.ratz.raw_port = ratz__get_port_method(av[1]);
	init_server(&blue);
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
		setenv("PS1", "", 1);
		setenv("TERM", "dumb", 1);
		if (execlp("bash", "bash", "--posix", "--norc", "--noprofile", NULL) == -1)
			strexit("execlp", 2);
	}
	else
	{
		signal(SIGUSR1, sigusr1_handling);
		unlink("DEBUG.out");
		close(blue.slave);
		char	cmd[DEBUG_SIZE_MAX] = {0};
		int		n = 0;
		int		m = 0;
		fcntl(blue.client, F_SETFL, O_NONBLOCK);
		fcntl(blue.master, F_SETFL, O_NONBLOCK);
		fdsave[0] = blue.client;
		fdsave[1] = blue.master;
		ratz_reset_fdset(&blue.ratz.poll.rdset, 2, fdsave);
		ratz_reset_fdset(&blue.ratz.poll.wrset, 2, fdsave);
		ratz_select(&blue.ratz.poll);
		
		send_auth_error(blue);
		while (waitpid(chd, NULL, WNOHANG) == 0)
		{
			ratz_reset_fdset(&blue.ratz.poll.rdset, 2, fdsave);
			ratz_reset_fdset(&blue.ratz.poll.wrset, 2, fdsave);
			ratz_select(&blue.ratz.poll);
			memset(cmd, 0, DEBUG_SIZE_MAX);
			while (1)
			{
				n = read(blue.client, cmd, DEBUG_SIZE_MAX);
				if (fcntl(blue.client, F_GETFD) == -1)
				{
					printf("DECONNECTION!!!\n");
					exit(0);
				}
				else if (n > 0)
					break ;
			}
			if (strcmp(cmd, "exit\n"))
			{
				n = sprintf(cmd, "%s ; kill -10 %d\n", cmd, getpid());
				cmd[n] = 0;
			}
			//write(2, "command debug : ", 17);
			//write(2, cmd, strlen(cmd));
			write(blue.master, cmd, n);
			while (commandline_finished == 0)
				; // make a nanosleep
			n = read(blue.master, cmd, DEBUG_SIZE_MAX);
			cmd[n] = '\0';
			commandline_finished = 0;
			m = write(blue.client, cmd, n);
			if ((m == 0 && n > 0) || m < 0)
				printf("DECONNECTION!!!\n");
			if (write(blue.client, "\001", 2) <= 0)
				printf("DECONNECTION!!!\n");
		}
	}
	return (0);
}
