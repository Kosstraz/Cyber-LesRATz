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

int	main(void)
{
	int	in = socket(AF_INET, SOCK_STREAM, 0);
	int	out = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in	addri;
	addri.sin_port = htons(8081);
	addri.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.1.1", &addri.sin_addr);
	connect(in, (const struct sockaddr*)&addri, sizeof(addri));
	send(in, "ai", 2, 0);
	connect(out, (const struct sockaddr*)&addri, sizeof(addri));
	send(out, "ao", 2, 0);

	char	test[1024] = {0};
	write(STDOUT_FILENO, "Reception... --> ", 18);
	recv(out, test, 1024, 0);
	printf("received : %s\n", test);
	write(STDOUT_FILENO, "Ecrire : \n", 11);
	int n = read(STDIN_FILENO, test, 5);
	send(in, test, n, 0);
	return (0);
}