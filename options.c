#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

/**
 * struct options
 */
struct options {
	int	 batch;
	int	 follow;
	int	 generate;
	int	 g_cols;
	int	 g_rows;
	int	 line_num;
	char	*filename;
	int	 winsize;
	int	 w_cols;
	int	 w_rows;
	int	 wrap;
};


struct options opts;

/**
 * usage
 */
void
usage (void)
{
	printf ("Usage: program [options]\n"
		"    -b, --batch\n"
		"    -f, --follow {on|off}\n"
		"    -g, --generate COLSxROWS\n"
		"    -l, --linenum LINENUM\n"
		"    -s, --save FILENAME\n"
		"    -w, --winsize COLSxROWS\n"
		"    -r, --wrap {on|off}\n"
		"    -h, --help\n");
	// --random
	// --seed
}

/**
 * get_on_off
 */
int
get_on_off (char *text)
{
	if (strcasecmp (text, "on") == 0)
		return 1;
	if (strcasecmp (text, "off") == 0)
		return 0;
	return -1;
}

/**
 * get_cols_rows
 */
int
get_cols_rows (char *text, int *cols, int *rows)
{
	int c = -1;
	int r = -1;
	int num = -1;

	num = sscanf (text, "%dx%d", &c, &r);
	if (num == 2) {
		*cols = c;
		*rows = r;
		return 1;
	}

	return 0;
}

/**
 * parse_options
 */
int
parse_options(int argc, char *argv[])
{
	static const char *sopt = "-bf:g:h?l:s:w:r:";
	static const struct option lopt[] = {
		{ "batch",	 no_argument,		NULL, 'b' },
		{ "follow",	 required_argument,	NULL, 'f' },
		{ "generate",	 required_argument,	NULL, 'g' },
		{ "help",	 no_argument,		NULL, 'h' },
		{ "linenum",	 required_argument,	NULL, 'l' },
		{ "save",	 required_argument,	NULL, 's' },
		{ "winsize",	 required_argument,	NULL, 'w' },
		{ "wrap",	 required_argument,	NULL, 'r' },
		{ NULL, 0, NULL, 0 }
	};

	int c = -1;
	int err = 0;
	int help = 0;
	char *end = NULL;

	opterr = 0; /* We'll handle the errors, thank you. */

	while ((c = getopt_long(argc, argv, sopt, lopt, NULL)) != -1) {
		switch (c) {
		case 1:	/* A non-option argument */
			printf ("Unknown argument: %s\n", optarg);
			err++;
			break;
		case 'b':
			opts.batch = 1;
			break;
		case 'f':
			opts.follow = get_on_off (optarg);
			break;
		case 'g':
			opts.generate = 1;
			get_cols_rows (optarg, &opts.g_cols, &opts.g_rows);
			break;
		case 'h':
		case '?':
			help++;
			break;
		case 'l':
			opts.line_num = strtol(optarg, &end, 0);
			if (end && (*end != 0))
				err++;
			break;
		case 's':
			opts.filename = optarg;
			break;
		case 'w':
			opts.winsize = 1;
			get_cols_rows (optarg, &opts.w_cols, &opts.w_rows);
			break;
		case 'r':
			opts.wrap = get_on_off (optarg);
			break;

		default:
			if (((optopt == 'f') || (optopt == 'g') ||
			     (optopt == 'l') || (optopt == 's') ||
			     (optopt == 'r') || (optopt == 'w')) && (!optarg)) {
				printf ("Option '%s' requires an argument.\n", argv[optind-1]);
			} else {
				printf ("Unknown option '%s'.\n", argv[optind-1]);
			}
			err++;
			break;
		}
	}

	if (err || help)
		usage();

	return (!err || !help);
}

/**
 * main
 */
int
main (int argc, char *argv[])
{
	parse_options (argc, argv);

	printf ("batch    = %d\n", opts.batch);
	printf ("follow   = %d\n", opts.follow);
	printf ("generate = %d\n", opts.generate);
	printf ("g_cols   = %d\n", opts.g_cols);
	printf ("g_rows   = %d\n", opts.g_rows);
	printf ("line_num = %d\n", opts.line_num);
	printf ("filename = %s\n", opts.filename);
	printf ("winsize  = %d\n", opts.winsize);
	printf ("w_cols   = %d\n", opts.w_cols);
	printf ("w_rows   = %d\n", opts.w_rows);
	printf ("wrap     = %d\n", opts.wrap);
	return 0;
}

