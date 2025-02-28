#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

int	main(void)
{
	char a[5];

	write(STDOUT_FILENO, "BASH t>> ", 10);
	fcntl(STDIN_FILENO, F_SETFL, ~O_NONBLOCK);
	read(STDIN_FILENO, a, 5);
	write(STDOUT_FILENO, "ZSH t>> ", 9);
	return (0);
}
