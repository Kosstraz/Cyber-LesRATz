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

// Set the struct addri to next 3 params
void	ratz_set_addri(s_sockaddr_in* addri, sa_family_t af, char* ip, in_port_t port)
{
	addri->sin_family = af;
	if (ip)
	{
		if (inet_pton(af, ip, &addri->sin_addr) == -1)
			strexit("inet_pton", 1);
	}
	else
		addri->sin_addr.s_addr = INADDR_ANY;
	addri->sin_port = htons(port);
}

void	strexit(const char* msg, int errcode)
{
	write(2, "errno : ", strlen("errno : "));
	write(2, strerror(errno), strlen(strerror(errno)));
	write(2, "\n", 1);
	write(2, msg, strlen(msg));
	exit(errcode);
}

int		ratz_write(fd_set* set, int fd, const void* buf, int nbytes)
{
	while (!FD_ISSET(fd, set))
		;
	return (write(fd, buf, nbytes));
}

int		ratz_read(fd_set* set, int fd, void* buf, int nbytes)
{
	while (!FD_ISSET(fd, set))
		;
	return (read(fd, buf, nbytes));
}

int	ratz_max(int count, int fds[count])
{
	int	tmp = fds[0];
	for (int i = 0 ; i < count ; ++i)
		if (tmp < fds[i])
			tmp = fds[i];
	return (tmp);
}

int	ratz_reset_fdset(fd_set* fset, int count, int fds[count])
{
	FD_ZERO(fset);
	for (int i = 0 ; i < count ; ++i)
		FD_SET(fds[i], fset);
	return (ratz_max(count, fds));
}

inline
int	ratz_select(s_polling* polling)
{
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	return (select(polling->fd_max + 1, &polling->rdset, &polling->wrset, NULL, &timeout));
}

// To replace with a best idea, or paste port used in source code (encrypted)
int	ratz__get_port_method(char* port)
{
	if (port)
		return (atoi(port));
	return (PORT);
}
