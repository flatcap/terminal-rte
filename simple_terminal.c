// gcc -Wall -o simple_terminal{,.c} -lcurses

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>

int
get_pty (int ptmx)
{
	char buf[32];

	memset (buf, 0, sizeof (buf));

	// get the name of the pseudo-terminal and get permission to use it
	ptsname_r (ptmx, buf, sizeof (buf) - 1);
	grantpt (ptmx);
	unlockpt (ptmx);

	return open (buf, O_RDWR | O_NONBLOCK);
}

void
send_command (int fd, char *command)
{
	int count;
	static char read_buf[1024];

	// send command to shell
	write (fd, command, strlen (command));
	write (fd, "\n", 1);

	// wait for a response
	usleep (500000);
	while ((count = read (fd, read_buf, sizeof (read_buf))) != -1) {
		write (STDOUT_FILENO, read_buf, count);
	}
}

int
main (int argc, char *argv[])
{
	int fd = -1;
	int fd2 = -1;
	int pid = -1;

	fd = getpt();
	fcntl (fd, F_SETFL, O_NONBLOCK);

	pid = fork();
	switch (pid) {
		case -1: /* error */
			printf ("fork failed\n");
			break;
		case 0: /* child */
			fd2 = get_pty (fd);
			close (fd);

			setsid();			// set up the session and the processs group
			setpgid (0, 0);

			ioctl (fd2, TIOCSCTTY, 0);	// set controlling tty

			dup2 (fd2, STDIN_FILENO);	// use the pty for our stdin,stdout,stderr
			dup2 (fd2, STDOUT_FILENO);
			dup2 (fd2, STDERR_FILENO);

			close (fd2);

			execl ("/bin/bash", "bash", "--norc", "--noprofile", NULL);
			break;
		default: /* parent */
			send_command (fd, "ls -l");

			// two ways to finish
			if (1)
				kill (pid, SIGTERM);
			else
				send_command (fd, "exit");

			close (fd);
			waitpid (pid, NULL, 0);		// 0 or WNOHANG
			break;
	}

	return 0;
}

