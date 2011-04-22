#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>

/**
 * rte_get_pty
 */
int
rte_get_pty (int ptmx)
{
	char buf[32];

	memset (buf, 0, sizeof (buf));

	// get the name of the pseudo-terminal and get permission to use it
	ptsname_r (ptmx, buf, sizeof (buf) - 1);
	grantpt (ptmx);
	unlockpt (ptmx);

	return open (buf, O_RDWR | O_NONBLOCK);
}

/**
 * rte_send_command
 */
void
rte_send_command (int fd, char *command)
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

/**
 * rte_set_utf8
 */
int
rte_set_utf8 (int fd)
{
	struct termios tio;
	tcflag_t saved_cflag;

	if (tcgetattr (fd, &tio) == -1) {
		return 0;
	}

	saved_cflag = tio.c_iflag;
	tio.c_iflag |= IUTF8;

	/* Only set the flag if it changes */
	if (saved_cflag != tio.c_iflag && tcsetattr (fd, TCSANOW, &tio) == -1) {
		return 0;
	}

	return 1;
}

/**
 * rte_get_size
 */
int
rte_get_size (int fd, int *rows, int *cols)
{
	struct winsize size;

	memset (&size, 0, sizeof (size));
	if (ioctl (fd, TIOCGWINSZ, &size) == 0) {
		if (cols != NULL) {
			*cols = size.ws_col;
		}
		if (rows != NULL) {
			*rows = size.ws_row;
		}
		return 1;
	} else {
		return 0;
	}
}

/**
 * rte_set_size
 */
int
rte_set_size (int fd, int rows, int cols)
{
	struct winsize size;

	memset (&size, 0, sizeof (size));
	size.ws_row = rows > 0 ? rows : 24;
	size.ws_col = cols > 0 ? cols : 80;
	if (ioctl (fd, TIOCSWINSZ, &size) != 0) {
		return 0;
	}

	return 1;
}

/**
 * rte_sig_child
 */
void
rte_sig_child (int sig, siginfo_t *info, void *context)
{
	printf ("child has died\n");
#if 0
	printf ("si_signo    = %d\n",  info->si_signo);		/* Signal number */
	printf ("si_errno    = %d\n",  info->si_errno);		/* An errno value */
	printf ("si_code     = %d\n",  info->si_code);		/* Signal code */
	printf ("si_pid      = %d\n",  info->si_pid);		/* Sending process ID */
	printf ("si_uid      = %d\n",  info->si_uid);		/* Real user ID of sending process */
	printf ("si_status   = %d\n",  info->si_status);	/* Exit value or signal */
	printf ("si_utime    = %ld\n", info->si_utime);		/* User time consumed */
	printf ("si_stime    = %ld\n", info->si_stime);		/* System time consumed */
	printf ("si_value    = %p\n",  info->si_value.sival_ptr);/* Signal value */
	printf ("si_int      = %d\n",  info->si_int);		/* POSIX.1b signal */
	printf ("si_ptr      = %p\n",  info->si_ptr);		/* POSIX.1b signal */
	printf ("si_overrun  = %d\n",  info->si_overrun);	/* Timer overrun count; POSIX.1b timers */
	printf ("si_timerid  = %d\n",  info->si_timerid);	/* Timer ID; POSIX.1b timers */
	printf ("si_addr     = %p\n",  info->si_addr);		/* Memory location which caused fault */
	printf ("si_band     = %ld\n", info->si_band);		/* Band event (was int in glibc 2.3.2 and earlier) */
	printf ("si_fd       = %d\n",  info->si_fd);		/* File descriptor */
#endif
}

/**
 * rte_clean_env
 */
void
rte_clean_env (void)
{
	const char *names[] = { "BZIP2", "CDPATH", "EDITOR", "FIGNORE", "GREP_COLOR", "GREP_OPTIONS", "GZIP", "HISTCONTROL", "HISTFILE", "HISTFILESIZE", "HISTIGNORE", "HISTSIZE", "HTML_TIDY", "INPUTRC", "KBUILD_VERBOSE", "LESS", "LESSCHARSET", "LESS_TERMCAP_mb", "LESS_TERMCAP_md", "LESS_TERMCAP_me", "LESS_TERMCAP_se", "LESS_TERMCAP_so", "LESS_TERMCAP_ue", "LESS_TERMCAP_us", "LS_COLORS", "MAKEFLAGS", "PAGER", "PATH", "PROMPT_COMMAND", "PS1", "TZ", "WINDOWMANAGER", "WORK_DIR", "WWW_HOME", "XZ_OPT", NULL };
	const char **ptr = names;

	for (; *ptr; ptr++) {
		unsetenv (*ptr);
	}
}


/**
 * main
 */
int
main (int argc, char *argv[])
{
	int fd = -1;
	int fd2 = -1;
	int rows = -1;
	int cols = -1;
	int pid = -1;
	struct sigaction sig_new;

	fd = getpt();
	fcntl (fd, F_SETFL, O_NONBLOCK);
	rte_set_utf8 (fd);

	rte_get_size (STDOUT_FILENO, &rows, &cols);
	printf ("size = (%dx%d)\n", cols, rows);

	rte_set_size (fd, rows, cols);
	printf ("set size\n");

	pid = fork();
	switch (pid) {
		case -1: /* error */
			printf ("fork failed\n");
			break;
		case 0: /* child */
			fd2 = rte_get_pty (fd);
			close (fd);

			setsid();			// set up the session and the processs group
			setpgid (0, 0);

			ioctl (fd2, TIOCSCTTY, 0);	// set controlling tty

			dup2 (fd2, STDIN_FILENO);	// use the pty for our stdin,stdout,stderr
			dup2 (fd2, STDOUT_FILENO);
			dup2 (fd2, STDERR_FILENO);

			close (fd2);

			rte_clean_env();

			execl ("/home/terminal/readline/bash/bash", "RASH", "--norc", "--noprofile", NULL);
			break;
		default: /* parent */
			memset (&sig_new, 0, sizeof (sig_new));
			sig_new.sa_flags     = SA_SIGINFO;
			sig_new.sa_sigaction = rte_sig_child;
			sigaction (SIGCHLD, &sig_new, NULL);

			sleep (1);

			rte_send_command (fd, "seq 50 | paste -s");
			//rte_send_command (fd, "ls -l");
			//rte_send_command (fd, "vim /tmp/$RANDOM.txt");
			//rte_send_command (fd, "echo hello world");
			//rte_send_command (fd, "set");

			sleep (1);

			// two ways to finish
			if (1)
				kill (pid, SIGTERM);
			else
				rte_send_command (fd, "exit");

			close (fd);
			waitpid (pid, NULL, 0);		// 0 or WNOHANG
			break;
	}

	//envp2 = __vte_pty_merge_environ (envv, pty->priv->term);

	return 0;
}

