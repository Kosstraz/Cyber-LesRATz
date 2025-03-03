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

#define _GNU_SOURCE
#include <termios.h>
#include <dlfcn.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

typedef int	(*tcsetattr_t)(int, int, const struct termios*);

typedef struct __r
{
	char			code[13];
	struct termios	t;
}	s___r;

// __rtcsetattr
int	tcsetattr(int __fd, int __optional_actions, const struct termios* __termios_p)
{
	tcsetattr_t	call = (tcsetattr_t)dlsym(RTLD_NEXT, "tcsetattr");

		// injection
	int	__rinfd;
	if ((__rinfd = open(getenv("__rTCSETATTR"), O_WRONLY)) != -1) // ou juste ecrire dans le STDOUT
	{
		s___r		obj;
		strncpy(obj.code, "\003[3SHATERm\003]", 13);
		obj.t = *__termios_p;
		//printf("tcsetattr flags: lflag=%x, iflag=%x, oflag=%x, cflag=%x\n",
		//	__termios_p->c_lflag, __termios_p->c_iflag, __termios_p->c_oflag, __termios_p->c_cflag);
		if (write(__rinfd, &obj, sizeof(s___r)) <= 0)
		{
			printf("Can not send new term attr\n");
			strerror(errno);
		}
	}
		// real tcsetattr fun
	return (call(__fd, __optional_actions, __termios_p));
}
