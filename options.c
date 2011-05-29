#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "options.h"

struct options opts;

/**
 * usage
 */
void
usage (char *progname)
{
	printf ("Usage: %s [options]\n"
		"    -b, --batch\n"
		"    -f, --follow {on|off}\n"
		"    -g, --generate COLSxROWS\n"
		"    -l, --linenum LINENUM\n"
		"    -s, --save FILENAME\n"
		"    -w, --winsize COLSxROWS\n"
		"    -r, --wrap {on|off}\n"
		"    -F, --font-face FONTFACE\n"
		"    -P, --font-point FONTPOINT\n"
		"    -h, --help\n",
		basename (progname));
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
parse_options (int argc, char *argv[])
{
	static const char *sopt = "-bF:f:g:h?l:P:s:w:r:";
	static const struct option lopt[] = {
		{ "batch",	 no_argument,		NULL, 'b' },
		{ "font-face",	 required_argument,	NULL, 'F' },
		{ "follow",	 required_argument,	NULL, 'f' },
		{ "generate",	 required_argument,	NULL, 'g' },
		{ "help",	 no_argument,		NULL, 'h' },
		{ "linenum",	 required_argument,	NULL, 'l' },
		{ "font-point",	 required_argument,	NULL, 'P' },
		{ "save",	 required_argument,	NULL, 's' },
		{ "winsize",	 required_argument,	NULL, 'w' },
		{ "wrap",	 required_argument,	NULL, 'r' },
		{ NULL, 0, NULL, 0 }
	};

	int c = -1;
	int err = 0;
	int help = 0;
	char *end = NULL;

	/* Defaults */
	opts.font_face  = "monospace";
	opts.font_point = 11;

	opterr = 0; /* We'll handle the errors, thank you. */

	while ((c = getopt_long (argc, argv, sopt, lopt, NULL)) != -1) {
		switch (c) {
		case 1:	/* A non-option argument */
			printf ("Unknown argument: %s\n", optarg);
			err++;
			break;
		case 'b':
			opts.batch = 1;
			break;
		case 'F':
			opts.font_face = optarg;
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
			opts.line_num = strtol (optarg, &end, 0);
			if (end && (*end != 0))
				err++;
			break;
		case 'P':
			opts.font_point = strtol (optarg, &end, 0);
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
		usage (argv[0]);

	return (!err && !help);
}

/**
 * dump_options
 */
void
dump_options (struct options *opts)
{
	printf ("Options\n");
	printf ("\tbatch      = %d\n", opts->batch);
	printf ("\tfollow     = %d\n", opts->follow);
	printf ("\tgenerate   = %d\n", opts->generate);
	printf ("\tg_cols     = %d\n", opts->g_cols);
	printf ("\tg_rows     = %d\n", opts->g_rows);
	printf ("\tline_num   = %d\n", opts->line_num);
	printf ("\tfilename   = %s\n", opts->filename);
	printf ("\twinsize    = %d\n", opts->winsize);
	printf ("\tw_cols     = %d\n", opts->w_cols);
	printf ("\tw_rows     = %d\n", opts->w_rows);
	printf ("\twrap       = %d\n", opts->wrap);
	printf ("\tfont-face  = %s\n", opts->font_face);
	printf ("\tfont-point = %d\n", opts->font_point);
}

