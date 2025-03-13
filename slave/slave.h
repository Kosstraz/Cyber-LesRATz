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

# define SHARE_TERM_ATTR	"\003[3SHATERm\003]"
# define END_OF_NETWORKING	"\003[3EONINGm\003]"
# define SHATER				SHARE_TERM_ATTR
# define EONING				END_OF_NETWORKING
# define ESCSEQ				13	// Size of escape sequences' ratz

# define PORT		8081
# define ADDRESS	"127.0.1.1"

# define DEBUG_SIZE_MAX	1024

# define _XOPEN_SOURCE	700

# include <sys/socket.h>
# include <arpa/inet.h>
# include <netdb.h>

# include <fcntl.h>
# include <unistd.h>
# include <string.h>
# include <stdio.h>
# include <stdlib.h>
# include <limits.h>

# include <sys/wait.h>
# include <sys/stat.h>
# include <sys/ioctl.h>

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
	char*	pname;
	char*	sname;
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
