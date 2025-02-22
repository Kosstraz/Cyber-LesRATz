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

# include "rapido.h"

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
	socklen_t			len;
	struct sockaddr_in	addr;
	s_network			net;

	memset(net.prompt, 0, sizeof(net.prompt));
	net.rapido = socket(AF_INET, SOCK_STREAM, 0);
	memset(&addr, 0, sizeof(addr));
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(av[1]));
	if (bind(net.rapido, (const struct sockaddr*)&addr, sizeof(addr)) == -1)
		vclose("bind", 1, net.rapido);
	if (listen(net.rapido, 2) == -1)
		vclose("listen", 1, net.rapido);
	memset(&addr, 0, sizeof(addr));
	fcntl(net.rapido, F_SETFL, fcntl(net.rapido, F_GETFL) | ~O_NONBLOCK);
	if ((net.razmo_in = accept(net.rapido, (struct sockaddr*)&addr, &len)) == -1)
		vclose("accept rin", 1, net.rapido);
	if ((net.razmo_out = accept(net.rapido, (struct sockaddr*)&addr, &len)) == -1)
		vclose("accept rout", 2, net.rapido, net.razmo_in);
	//char _true = 1;
	//setsockopt(net.razmo, SOL_SOCKET, SO_KEEPALIVE, &_true, sizeof(_true));
	char	cmd[4096] = {0};
	int		bread = 0;

	//dup2(net.razmo, STDOUT_FILENO);
	fcntl(net.razmo_in, F_SETFL, fcntl(net.rapido, F_GETFL) | O_NONBLOCK);
	fcntl(net.razmo_out, F_SETFL, fcntl(net.rapido, F_GETFL) | ~O_NONBLOCK);
	while (1)
	{
		write(STDOUT_FILENO, PROMPT, strlen(PROMPT));
		memset(cmd, 0, sizeof(cmd));
		while ((bread += read(STDIN_FILENO, cmd, 4096)) == 0 && cmd[bread - 1] && cmd[bread - 1] != '\n')
			;
		printf("$\e[31m%s\e[0m$\n", cmd);
		//cmd[bread - 1] = '\0';
		if (send(net.razmo_in, cmd, strlen(cmd), 0) <= 0)
			vclose("send", 3, net.rapido, net.razmo_out, net.razmo_in);
		int	end = 0;
		while ((end = recv(net.razmo_out, cmd, 4096, 0)) <= 0 && !strcmp(cmd, REMOTE_PROMPT))
		{
		}
		if (end == -1)
			vclose("recv", 3, net.razmo_in, net.razmo_out, net.rapido);
		if (end > 0)
			cmd[end - 1] = '\0';
		write(STDOUT_FILENO, cmd, strlen(cmd));
		write(STDOUT_FILENO, "\n", 1);
		bread = 0;
	}
	close(net.razmo_in);
	close(net.razmo_out);
	close(net.rapido);
	return (0);
}
