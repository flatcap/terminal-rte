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

/**
 * rte_ptsname
 */
static char *
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

/**
 * rte_set_utf8
 * @pty: a #VtePty
 * @utf8: whether or not the pty is in UTF-8 mode
 * @error: (allow-none): return location to store a #GError, or %NULL
 *
 * Tells the kernel whether the terminal is UTF-8 or not, in case it can make
 * use of the info.  Linux 2.6.5 or so defines IUTF8 to make the line
 * discipline do multibyte backspace correctly.
 *
 * Returns: %1 on success, %0 on failure
 */
int
rte_set_utf8 (int fd, int utf8)
{
	struct termios tio;
	tcflag_t saved_cflag;

	if (tcgetattr (fd, &tio) == -1) {
		return 0;
	}

	saved_cflag = tio.c_iflag;
	if (utf8) {
		tio.c_iflag |= IUTF8;
	} else {
		tio.c_iflag &= ~IUTF8;
	}

	/* Only set the flag if it changes */
	if (saved_cflag != tio.c_iflag && tcsetattr (fd, TCSANOW, &tio) == -1) {
		return 0;
	}

	return 1;
}

/**
 * rte_get_size
 * @rows: (out) (allow-none): a location to store the number of rows, or %NULL
 * @columns: (out) (allow-none): a location to store the number of columns, or %NULL
 * @error: return location to store a #GError, or %NULL
 *
 * Reads the pseudo terminal's window size.
 *
 * If getting the window size failed, @error will be set to a #GIOError.
 *
 * Returns: %1 on success, %0 on failure with @error filled in
 *
 * Since: 0.26
 */
int
rte_get_size (int fd, int *rows, int *columns)
{
	struct winsize size;

	memset (&size, 0, sizeof (size));
	if (ioctl (fd, TIOCGWINSZ, &size) == 0) {
		if (columns != NULL) {
			*columns = size.ws_col;
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
 * @rows: the desired number of rows
 * @columns: the desired number of columns
 * @error: (allow-none); return location to store a #GError, or %NULL
 *
 * Attempts to resize the pseudo terminal's window size.  If successful, the
 * OS kernel will send #SIGWINCH to the child process group.
 *
 * If setting the window size failed, @error will be set to a #GIOError.
 *
 * Returns: %1 on success, %0 on failure with @error filled in
 */
int
rte_set_size (int fd, int rows, int columns)
{
	struct winsize size;

	memset (&size, 0, sizeof (size));
	size.ws_row = rows > 0 ? rows : 24;
	size.ws_col = columns > 0 ? columns : 80;
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
	printf ("SIGCHLD - child dead?\n");
}


#ifdef wibble
/**
 * vte_pty_child_setup
 * @pty: a #VtePty
 *
 * FIXMEchpe
 *
 * Since: 0.26
 */
void
rte_child_setup (int fd)
{
	int fd = -1;
	const char *tty = NULL;

	if (priv->foreign) {
		fd = priv->pty_fd;
	} else {
		/* Save the name of the pty -- we'll need it later to acquire it as our controlling terminal.  */
		switch (data->mode) {
			case TTY_OPEN_BY_NAME:
				tty = data->tty.name;
				break;
			case TTY_OPEN_BY_FD:
				fd = data->tty.fd;
				tty = ttyname (fd);
				break;
		}

		printf ("Setting up child pty: name = %s, fd = %d\n", tty ? tty : " (none)", fd);

		/* Try to reopen the pty to acquire it as our controlling terminal. */
		/* FIXMEchpe: why not just use the passed fd in TTY_OPEN_BY_FD mode? */
		if (tty != NULL) {
			int i = open (tty, O_RDWR);
			if (i != -1) {
				if (fd != -1){
					close (fd);
				}
				fd = i;
			}
		}
	}

	if (fd == -1)
		_exit (127);

	/* Start a new session and become process-group leader. */
	setsid();
	setpgid (0, 0);

	/* TIOCSCTTY is defined?  Let's try that, too. */
	ioctl (fd, TIOCSCTTY, fd);

	/* now setup child I/O through the tty */
	if (fd != STDIN_FILENO) {
		if (dup2 (fd, STDIN_FILENO) != STDIN_FILENO){
			_exit (127);
		}
	}
	if (fd != STDOUT_FILENO) {
		if (dup2 (fd, STDOUT_FILENO) != STDOUT_FILENO){
			_exit (127);
		}
	}
	if (fd != STDERR_FILENO) {
		if (dup2 (fd, STDERR_FILENO) != STDERR_FILENO){
			_exit (127);
		}
	}

	/* Close the original slave descriptor, unless it's one of the stdio descriptors. */
	if (fd != STDIN_FILENO && fd != STDOUT_FILENO && fd != STDERR_FILENO) {
		close (fd);
	}

	/* Reset our signals -- our parent may have done any number of weird things to them. */
	_vte_pty_reset_signal_handlers();

	/* Now set the TERM environment variable */
	if (priv->term != NULL) {
		g_setenv ("TERM", priv->term, 1);
	}
}

#endif

/**
 * main
 */
int
main (int argc, char *argv[])
{
	int fd = -1;
	char *buf = NULL;
	int rows = -1;
	int columns = -1;
	int pid = -1;
	int ppid = -1;
	int fd2 = -1;
	char read_buf[1024];
	int count;
	int i;
	struct sigaction sig_new;
	struct sigaction sig_old;

	fd = getpt();
	if (!fd) {
		perror ("getpt");
		return 1;
	}

	printf ("fd = %d\n", fd);

	if ((buf = rte_ptsname (fd)) == NULL) {
		close (fd);
		return 1;
	}

	if (rte_set_utf8 (fd, 1) == 0) {
		free (buf);
		close (fd);
		return 1;
	}

	printf ("name = %s\n", buf);

	if (rte_set_size (fd, rows, columns)) {
		printf ("set size\n");
	}

	if (rte_get_size (fd, &rows, &columns)) {
		printf ("size = (%dx%d)\n", columns, rows);
	}

	printf ("tty = %s\n", ttyname (fd));

	ppid = getpid();

	pid = fork();
	switch (pid) {
		case -1:
			printf ("fork failed\n");
		/* child */
		case 0:
			/* Start a new session and become process-group leader. */
			fd2 = open (buf, O_RDWR | O_NONBLOCK);
			printf ("child fd = %s, %d\n", buf, fd);
			free (buf);
#if 0
			signal (SIGPIPE, SIG_DFL);
			signal (SIGTERM, SIG_DFL);
#endif
#if 1
			setsid();
			setpgid (0, ppid);
#endif

			/* TIOCSCTTY is defined?  Let's try that, too. */
			ioctl (fd2, TIOCSCTTY, 0);

			//vte_pty_child_setup (pty);
			/* now setup child I/O through the tty */
			if (fd2 != STDIN_FILENO) {
				if (dup2 (fd2, STDIN_FILENO) != STDIN_FILENO){
					return 1;
				}
			}
			if (fd2 != STDOUT_FILENO) {
				if (dup2 (fd2, STDOUT_FILENO) != STDOUT_FILENO){
					return 1;
				}
			}
			if (fd2 != STDERR_FILENO) {
				if (dup2 (fd2, STDERR_FILENO) != STDERR_FILENO){
					return 1;
				}
			}
			printf ("child close %d\n", fd);
			close (fd);
			printf ("child close %d\n", fd2);
			close (fd2);
			printf ("child ok\n");

			execl ("/home/terminal/readline/bash-4.2/rash", "RASH", NULL);
			perror ("execl");
			break;
		/* parent */
		default:
			memset (&sig_new, 0, sizeof (sig_new));
			memset (&sig_old, 0, sizeof (sig_old));

			sig_new.sa_flags     = SA_SIGINFO;
			sig_new.sa_sigaction = rte_sig_child;

			sigaction (SIGCHLD, &sig_new, &sig_old);

			sleep (2);
			printf ("parent ok\n");
			//fd2 = open (buf, O_RDWR | O_NONBLOCK);
			//printf ("parent fd = %s, %d\n", buf, fd);
			//sleep (5);
			for (i = 0; i < 3; i++) {
				count = write (fd, "ls\n", 3);
				printf ("write = %d\n", count);
				sleep (1);
				count = read (fd, read_buf, sizeof (read_buf));
				if (count > 0) {
					write (STDOUT_FILENO, read_buf, count);
				}
			}
			//printf ("starting vim\n");
			//count = write (fd, "vim $RANDOM.txt\n", 16);
#if 0
			printf ("running set\n");
			count = write (fd, "set\n", 4);
			sleep (1);
			while ((count = read (fd, read_buf, sizeof (read_buf))) != -1) {
				write (STDOUT_FILENO, read_buf, count);
			}
#endif

			sleep (1);
			close (fd);
#if 0
			printf ("exit\n");
			count = write (fd, "exit\n", 5);
			sleep (1);
			while ((count = read (fd, read_buf, sizeof (read_buf))) != -1) {
				write (STDOUT_FILENO, read_buf, count);
			}
#else
			printf ("kill %d\n", pid);
			kill (pid, SIGTERM);
#endif
			printf ("wait for child to die\n");
			//sleep (300);	// interrupted by signal
			//waitpid (pid, NULL, WNOHANG);
			waitpid (pid, NULL, 0);
			printf ("child dead\n");
			break;
	}

	//envp2 = __vte_pty_merge_environ (envv, pty->priv->term);

	free (buf);	// clean this up before execl?

	return 0;
}

