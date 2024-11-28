// wrapper for socket library routines

#include <stdio.h>
#include <string.h>
#include "utils.h"

char *
chop_newline(char *buf, int len)
{
    int n = strlen(buf);

    if (n < len && buf[n - 1] == '\n') {
	buf[n - 1] = '\0';
    }
    return buf;
}

void
print_buf(char *buf, int nbyte)
{
    int i, j;

    printf("\n");
    for (j = 0; j < nbyte / 4; j++) {
	for (i = 0; i < 4; i++) {
	    printf("%d ", buf[j * 4 + i]);
	}
	printf("   ");
    }
    printf("\n");
}
