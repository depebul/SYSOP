#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>

#define PIPE "./squareFIFO"

int main(int argc, char *argv[])
{

	if (argc != 2)
	{
		printf("Not a suitable number of program parameters\n");
		return (1);
	}
	// utworz potok nazwany pod sciezka reprezentowana przez PIPE
	// zakladajac ze parametrem wywolania programu jest liczba calkowita
	// zapisz te wartosc jako int do potoku i posprzataj
	if (mkfifo(PIPE, 0666) == -1)
	{
		perror("mkfifo");
		exit(EXIT_FAILURE);
	}
	int fd = open(PIPE, O_WRONLY);
	if (fd == -1)
	{
		perror("open");
		exit(EXIT_FAILURE);
	}
	int val = atoi(argv[1]);
	if (write(fd, &val, sizeof(int)) != sizeof(int))
	{
		perror("write");
		close(fd);
		exit(EXIT_FAILURE);
	}
	close(fd);
	return 0;
}
