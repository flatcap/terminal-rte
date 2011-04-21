#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <features.h>
#include <stdlib.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

char *
rte_ptsname (int master)
{
	int len = 1024;
	char *buf = NULL;

	buf = calloc (1, len);
	if (ptsname_r (master, buf, len - 1) || grantpt (master) || unlockpt (master)) {
		free (buf);
		buf = NULL;
	}

	return buf;
}

int
main (int argc, char *argv[])
{
	int fd = -1;
	char *buf = NULL;
	int pid = -1;
	int fd2 = -1;
	char read_buf[1024];
	int count;
	char *cmd = NULL;

	fd = getpt();
	fcntl (fd, F_SETFL, O_NONBLOCK);

	pid = fork();
	switch (pid) {
		case -1: /* error */
			printf ("fork failed\n");
			break;
		case 0: /* child */
			/* Start a new session and become process-group leader. */
			if ((buf = rte_ptsname (fd)) == NULL) {
				close (fd);
				break;
			}

			fd2 = open (buf, O_RDWR | O_NONBLOCK);
			free (buf);

			setsid();
			setpgid (0, 0);

			/* TIOCSCTTY is defined?  Let's try that, too. */
			ioctl (fd2, TIOCSCTTY, 0);

			dup2 (fd2, STDIN_FILENO);
			dup2 (fd2, STDOUT_FILENO);
			dup2 (fd2, STDERR_FILENO);

			close (fd);
			close (fd2);

			execl ("/bin/bash", "bash", "--norc", "--noprofile", NULL);
			break;
		default: /* parent */
			cmd = "set\n";
			write (fd, cmd, strlen (cmd));
			sleep (1);
			while ((count = read (fd, read_buf, sizeof (read_buf))) != -1) {
				write (STDOUT_FILENO, read_buf, count);
			}
#if 1
			cmd = "exit\n";
			write (fd, cmd, strlen (cmd));
			sleep (1);
			while ((count = read (fd, read_buf, sizeof (read_buf))) != -1) {
				write (STDOUT_FILENO, read_buf, count);
			}
#else
			printf ("kill %d\n", pid);
			kill (pid, SIGTERM);
#endif
			close (fd);
			waitpid (pid, NULL, 0);	// 0 or WNOHANG
			break;
	}

	return 0;
}

