#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_BUF 1024
#define FIFO "/tmp/notifications-command.fifo"

int fifo_writer(char buffer[]);
int fifo_reader();
void notify_send(char buffer[]);

int fifo_writer(char buffer[])
{
	int fd;

	/* Create a FIFO named pipe if necessary */
	mkfifo(FIFO, 0666);

	/* Write to the FIFO pipe */
	fd = open(FIFO, O_WRONLY);
	write(fd, buffer, strlen(buffer) + 1);
	close(fd);

	return 0;
}


int fifo_reader()
{
	int fd;
	int read_bytes;
	char buf[MAX_BUF];

	/* Create a FIFO named pipe if necessary */
	mkfifo(FIFO, 0666);

	/* Continuously read data from named pipe. */
	while (true)
	{
		fd = open(FIFO, O_RDONLY);
		read_bytes = read(fd, buf, MAX_BUF);
		buf[read_bytes] = '\0';
		printf("%s\n", buf);
		close(fd);
	}
	return 0;
}


void notify_send(char buffer[])
{
	int buflen = strlen(buffer) + 1;
	char *msg;

	msg = (char*)malloc(strlen("notify-send ") + buflen + 3);
	strcpy(msg, "notify-send \"");
	strcat(msg, buffer);
	strcat(msg, "\"");
	system(msg);

	return;
}


int main(int argc, char **argv)
{
	bool helpMessage = false;
	bool daemonMode = false;
	bool notifyMode = false;
	bool verboseMode = false;
	int opt;
	int retVal;

	/* Argument parsing. */
	while ((opt = getopt(argc, argv, "hdnv")) != -1)
	{
		switch(opt)
		{
			case 'h': helpMessage = true; break;
			case 'd': daemonMode = true; break;
			case 'n': notifyMode = true; break;
			case 'v': verboseMode = true; break;
			default:
				fprintf(stderr, "Usage: %s [-hdnv] [message]\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	if (daemonMode) {
		retVal = fifo_reader();

	} else {
		/* Initiate buffers for concatenating non-option arguments. */
		char *oldBuf;
		char *newBuf = "\0";

		/* Create string containing non-option arguments. */
		for (int index = optind; index < argc; index++) {
			/* Size old buffer and copy new buffer contents to it. */
			oldBuf = (char*)malloc(strlen(newBuf) + 2);
			strcpy(oldBuf, newBuf);

			/* Add space if not first word. */
			if (strcmp(newBuf, "\0") != 0)
				strcat(oldBuf, " ");

			/* Resize new buffer and copy contents of old buffer and argv. */
			newBuf = (char*)malloc(strlen(oldBuf) + strlen(argv[index]) + 1);
			strcpy(newBuf, oldBuf);
			strcat(newBuf, argv[index]);
		}

		retVal = fifo_writer(newBuf);
		if (notifyMode) {
			notify_send(newBuf);
		}

		free(oldBuf);
		free(newBuf);
	}

	return retVal;
}
