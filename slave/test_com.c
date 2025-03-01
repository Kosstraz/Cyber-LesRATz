#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int	main(void)
{
	char	msg[1024] = {0};
	int		n = 0;

	while (1)
	{
		n = read(STDIN_FILENO, msg, 1024);
		msg[n] = 0;
		if (!strcmp(msg, "TEST 1!!\n"))
			break ;
	}
	printf("GG  ;)\n");
	while (1)
	{
		n = read(STDIN_FILENO, msg, 1024);
		msg[n] = 0;
		if (!strcmp(msg, "NEUIIILLE\n"))
			break ;
	}
	printf("Finito baby\n");
	return (0);
}