// gcc -Wall -o window_resize{,.c} -lcurses

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <curses.h>
#include <term.h>

/**
 * winsize_ioctl
 */
void
winsize_ioctl (void)
{
	struct winsize w;
	ioctl (STDOUT_FILENO, TIOCGWINSZ, &w);

	printf ("ioctl:  Window resized (%dx%d)\n", w.ws_col, w.ws_row);
}

/**
 * winsize_curses
 */
void
winsize_curses (void)
{
	static char termbuf[2048];
	int rows;
	int cols;
	char *termtype = getenv ("TERM");

	if (tgetent (termbuf, termtype) < 0) {
		printf ("Could not access the termcap data base.\n");
		return;
	}

	rows = tgetnum ("li");
	cols = tgetnum ("co");
	printf ("curses: Window resized (%dx%d)\n", cols, rows);
}

/**
 * sig_winch
 */
void
sig_winch (int sig, siginfo_t *info, void *context)
{
	winsize_ioctl();
	winsize_curses();
}

/**
 * main
 */
int
main (int argc, char *argv[])
{
	struct sigaction sig;

	memset (&sig, 0, sizeof (sig));
	sig.sa_flags     = SA_SIGINFO;
	sig.sa_sigaction = sig_winch;
	sigaction (SIGWINCH, &sig, NULL);

	while (1) {
		sleep (1);
	}

	return 0;
}

