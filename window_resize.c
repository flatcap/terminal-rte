// gcc -Wall -o jim{,.c} -lcurses

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <curses.h>
#include <term.h>

void
winsize_ioctl (void)
{
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

	printf ("ioctl:  Window resized (%dx%d)\n", w.ws_col, w.ws_row);
}

void
winsize_curses (void)
{
	static char termbuf[2048];
	int rows;
	int cols;
	char *termtype = getenv("TERM");

	if (tgetent(termbuf, termtype) < 0) {
		printf ("Could not access the termcap data base.\n");
		return;
	}

	rows = tgetnum("li");
	cols = tgetnum("co");
	printf ("curses: Window resized (%dx%d)\n", cols, rows);
}

void
sig_winch (int arg)
{
	winsize_ioctl();
	winsize_curses();
}

int
main (int argc, char *argv[])
{
	signal (SIGWINCH, sig_winch);

	while (getch() != 27) {
	}

	return 0;
}

