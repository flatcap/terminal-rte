#ifdef DETAB_MAIN
#include <stdio.h>
#include <string.h>
#endif

void
detab (char *in, char *out, int max_len)
{
	int i = 0;
	while (*in && i < max_len - 1) {
		if (*in == '\t') {
			out[i++] = ' ';
			while (i % 8 && i < max_len - 1) {
				out[i++] = ' ';
			}
			in++;
		} else {
			out[i++] = *in++;
		}
	}

	out[i] = 0;
}

#ifdef DETAB_MAIN
int
main (int argc, char *argv[])
{
	char *one = "hello\tworld";
	char *two = "\t1\t12\t123\t1234\t12345\t123456\t1234567\t12345678\t123456789";
	char buffer[256];

	memset (buffer, 0, sizeof (buffer));
	detab (one, buffer, sizeof (buffer) - 1);
	printf ("%s<<\n", one);
	printf ("%s<<\n", buffer);
	printf ("\n");

	memset (buffer, 0, sizeof (buffer));
	detab (two, buffer, sizeof (buffer) - 1);
	printf ("%s<<\n", two);
	printf ("%s<<\n", buffer);
	printf ("\n");

	return 0;
}
#endif

