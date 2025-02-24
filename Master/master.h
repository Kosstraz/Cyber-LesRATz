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

#ifndef MASTER_H
#define MASTER_H

# define RATZ_PROMPT	"\e[1;32mRATBoard =>\e[0m "

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

# include <sys/socket.h>
# include <arpa/inet.h>

# include <sys/mman.h>
# include <sys/stat.h>

# include <signal.h>
# include <fcntl.h>
# include <string.h>
# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>

# include <readline/readline.h>
# include <readline/history.h>

typedef struct red
{
	int		killed;
	char*	buffer;		// prompt line
	s_ratz	ratz;		// other things
}	s_red;

#endif
