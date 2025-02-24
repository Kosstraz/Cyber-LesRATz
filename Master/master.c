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

static int	fclient = -1;

void	init_server(s_red* red)
{
	if ((red->ratz.server = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		strexit("socket", 1);
	ratz_set_addri(&red->ratz.addri, AF_INET, LOCALHOST, red->ratz.raw_port);
	if (bind(red->ratz.server, (const s_sockaddr*)&red->ratz.addri, sizeof(red->ratz.addri)) == -1)
		strexit("bind", 1);
	else if (listen(red->ratz.server, 1) == -1)
		strexit("listen", 1);
	else if ((red->client = accept(red->ratz.server, (s_sockaddr*)&red->ratz.addri, &red->ratz.addri_len)) == -1)
		strexit("accept", 1);
}

void	sig_handler(int signum)
{
	if (signum == SIGINT)
	{
		rl_on_new_line();
		rl_redisplay();
		write(STDOUT_FILENO, "\n", 1);
		write(STDOUT_FILENO, PROMPT, strlen(PROMPT));
	}
}

int	main(int, char** av)
{
	signal(SIGINT, sig_handler);
	s_red	red;
	memset(&red, 0, sizeof(red));

	red.ratz.raw_port = ratz__get_port_method(av[1]);
	init_server(&red);
	fclient = dup(red.client);
	fcntl(red.client, F_SETFL, fcntl(red.client, F_GETFL) | O_NONBLOCK);
	char	output[DEBUG_SIZE_MAX] = {0};
	int		n = 0;
	while (red.killed == 0)
	{
		red.buffer = readline(PROMPT);
		if (!red.buffer)
		{
			printf("exit\n");
			write(red.client, "exit\n", 6);
			red.killed = 1;
		}
		else if (red.buffer[0] && strcmp(red.buffer, "\003"))
		{
			if (write(red.client, red.buffer, strlen(red.buffer)) <= 0)
				strexit("send", 0);
			fcntl(red.client, F_SETFL, fcntl(red.client, F_GETFL) & ~O_NONBLOCK);
			n = read(red.client, output, sizeof(output));
			fcntl(red.client, F_SETFL, fcntl(red.client, F_GETFL) | O_NONBLOCK);
			write(STDOUT_FILENO, output, n);
			free(red.buffer);
		}
	}
	close(red.ratz.server);
	close(red.client);
	close(fclient);
	return (0);
}
