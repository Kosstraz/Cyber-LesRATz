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

#ifndef SLAVE_H
#define SLAVE_H

//# define DEBUG

# ifdef DEBUG
#  include <stdarg.h>
#  define P(...) printf(__VA_ARGS__);
# endif

# define bool	char
# define true	1
# define false	0

# define PORT		8081
# define ADDRESS	"127.0.1.1"

# define DEBUG_SIZE_MAX	1024

# define _XOPEN_SOURCE	700

# include <sys/socket.h>
# include <arpa/inet.h>
# include <netdb.h>

//# include <termios.h>

//# include <time.h>
# include <sys/ioctl.h>
# include <fcntl.h>
# include <unistd.h>
# include <string.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/wait.h>
# include <stdarg.h>

# include <signal.h>

typedef struct sockaddr_in	s_sockaddr_in;
typedef struct sockaddr		s_sockaddr;

typedef struct net
{
	s_sockaddr_in	addri;
	socklen_t		len;
	int				in;
	int				out;
}	s_net;

typedef struct auth
{
	int	pr;
	int	gr;
	int	ioctl;
	int	setsid;
}	s_auth;

typedef struct slave
{
	int		pid;	// own pid
	int		chd;	// child pid
	int		ptm;	// pt master
	int		pts;	// pt slave
	char	msg[DEBUG_SIZE_MAX];
	int		len;
	s_net	net;
	s_auth	auth;
}	s_slave;

#endif
