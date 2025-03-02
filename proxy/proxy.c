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

#include "proxy.h"

static inline
void	closeconn(int* fd)
{
	if (*fd > 0)
		close(*fd);
	*fd = -1;
}

static inline
void	eperror(const char* msg)
{
	perror(msg);
	exit(1);
}

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

void	determine_connection(s_proxy* proxy)
{
	char	req_pass[3] = {0};
	int		tmp = -1;

	if ((tmp = accept(proxy->serv, (s_sockaddr*)&proxy->addri, &proxy->addri_len)) == -1 && errno != EAGAIN)
		eperror("accept");
	else if (errno != EAGAIN)
	{
		fcntl(tmp, F_SETFL, fcntl(tmp, F_GETFL) & ~O_NONBLOCK);
		recv(tmp, req_pass, 2, 0);
		if (req_pass[0] == 'v')
		{
			if (req_pass[1] == 'i')
			{
				closeconn(&proxy->v_out);
				proxy->v_out = tmp;
			}
			else if (req_pass[1] == 'o')
			{
				closeconn(&proxy->v_in);
				proxy->v_in = tmp;
			}
			printf("Client V connected his %s\n", (req_pass[1] == 'i' ? "input" : "output"));
		}
		else if (req_pass[0] == 'a')
		{
			if (req_pass[1] == 'i')
			{
				closeconn(&proxy->a_out);
				proxy->a_out = tmp;
			}
			else if (req_pass[1] == 'o')
			{
				closeconn(&proxy->a_in);
				proxy->a_in = tmp;
			}
			printf("Client A connected his %s\n", (req_pass[1] == 'i' ? "input" : "output"));
		}
	}
}

int	__max(int a, int b, int c, int d)
{
	int	m = a;

	if (b > m)
		m = b;
	if (c > m)
		m = c;
	if (d > m)
		m = d;
	return (m);
}

void	reset_fdset(fd_set* set, int fd1, int fd2)
{
	if (fd1 == -1 || fd2 == -1)
		eperror("wtf");
	FD_ZERO(set);
	FD_SET(fd1, set);
	FD_SET(fd2, set);
}

int	main(void)
{
	char	msg[DEBUG_SIZE_MAX];
	int		n = 0;
	s_proxy	proxy;

		// setup proxy server
	memset(&proxy, 0, sizeof(s_proxy));
	proxy.addri.sin_family = AF_INET;
	proxy.addri.sin_port = htons(PORT);
	proxy.addri.sin_addr.s_addr = INADDR_ANY;
	if ((proxy.serv = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		eperror("socket");
	else if (bind(proxy.serv, (const s_sockaddr*)&proxy.addri, sizeof(proxy.addri)) == -1)
		eperror("bind");
	else if (listen(proxy.serv, 8) == -1)
		eperror("listen");
	printf("%s:%d\n", get_own_addr(), PORT);

		// establish connections
	determine_connection(&proxy);
	determine_connection(&proxy);
	determine_connection(&proxy);
	determine_connection(&proxy);

		// pseudo-bidirectionnal communication	[A <--> PROXY <--> V]
	struct timeval	tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	//fcntl(proxy.serv, F_SETFL, O_NONBLOCK);
	while (true)
	{
		if (proxy.dis == true)
		{
			determine_connection(&proxy);
			determine_connection(&proxy);
			proxy.dis = false;
		}
		proxy.polling.max_fd = __max(proxy.v_in, proxy.v_out, proxy.a_in, proxy.a_out);
		reset_fdset(&proxy.polling.rd, proxy.a_out, proxy.v_out);
		reset_fdset(&proxy.polling.wr, proxy.a_in, proxy.v_in);
		reset_fdset(&proxy.polling.err, proxy.a_out, proxy.v_out);
		select(proxy.polling.max_fd + 1, &proxy.polling.rd, &proxy.polling.wr, &proxy.polling.err, &tv);
		if (FD_ISSET(proxy.a_out, &proxy.polling.err))
			proxy.dis = true;
		else if (FD_ISSET(proxy.v_out, &proxy.polling.err))
			proxy.dis = true;
		else
		{
			if (proxy.dis == false && FD_ISSET(proxy.a_out, &proxy.polling.rd))
			{
				n = recv(proxy.a_out, msg, DEBUG_SIZE_MAX, 0);
				if (n == 0)
					proxy.dis = true;
				else if (n == -1)
					eperror("recv");
				else
				{
					while (!FD_ISSET(proxy.v_in, &proxy.polling.wr))
						;
					send(proxy.v_in, msg, n, 0);
				}
			}
			if (proxy.dis == false && FD_ISSET(proxy.v_out, &proxy.polling.rd))
			{
				n = recv(proxy.v_out, msg, DEBUG_SIZE_MAX, 0);
				if (n == 0)
					proxy.dis = true;
				else if (n == -1)
					eperror("recv");
				else
				{
					while (!FD_ISSET(proxy.a_in, &proxy.polling.wr))
						;
					send(proxy.a_in, msg, n, 0);
				}
			}
		}
	}
	return (0);
}
