#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define cprintf(fmt, args...) do { \
	FILE *fp = fopen("/dev/console", "w"); \
	if (fp) { \
		fprintf(fp, fmt, ## args); \
		fclose(fp); \
	} \
} while (0)

int main(int argc, char **argv, char *env[])
{
	int i = 0;

	while (argv[i]) {
		cprintf("argv[%d]: %s\n", i, argv[i]);
		i++;
	}

	i = 0;
	while (env[i]) {
		cprintf("env[%d]: %s\n", i, env[i]);
		i++;
	}

	return 0;
}
