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

#ifndef PROXY_H
#define PROXY_H

# define PORT		8081
# define DEBUG_SIZE_MAX	1024

# define bool	char
# define true	1
# define false	0

# include <unistd.h>
# include <stdlib.h>
# include <string.h>
# include <stdio.h>
# include <fcntl.h>

# include <errno.h>

# include <sys/socket.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <sys/select.h>

typedef struct sockaddr_in	s_sockaddr_in;
typedef struct sockaddr		s_sockaddr;

typedef struct polling
{
	int		max_fd;
	fd_set	wr;
	fd_set	rd;
	fd_set	err;
}	s_polling;

typedef struct proxy
{
	s_sockaddr_in	addri;
	socklen_t		addri_len;
	s_polling		polling;
	int				v_in;
	int				v_out;
	int				a_in;
	int				a_out;
	int				serv;
	bool			dis;
}	s_proxy;

#endif
