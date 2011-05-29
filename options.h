#ifndef _OPTIONS_H_
#define _OPTIONS_H_

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
	char	*font_face;
	int	 font_point;
};


extern struct options opts;

int parse_options (int argc, char *argv[]);
void dump_options (struct options *opts);

#endif // _OPTIONS_H_

