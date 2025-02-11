#include "test_sentences.h"
#include "../src/delimit.h"
#include "../src/binary_next_delimit.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(void) {
	int ret_code = EXIT_FAILURE;
	char buffer[8192];
	/* "ab  c" 3: 1 :[ */

	struct delimit delimit;
	size_t count = 0, read;
	int on_edge = 0;
	errno = 0;

	freopen("UnicodeData.txt", "r", stdin);
	// 5—387915, 8192—390032. :[ 3—390032, 5—390032, 8192—390032. :]
	// fixme: It's still chopping up code-points.

	while(read = fread(buffer, 1, sizeof buffer - 1, stdin)) {
		buffer[read] = '\0';
		//printf("Read: \"%s\".\n", buffer);
		for(delimit.end.c = buffer;
			binary_next_delimit(&delimit), delimit.start.c; ) {
			if(!on_edge); else {
				on_edge = 0;
				if(delimit.start.c == buffer) {
					/* fixme: May get split mid-code-point! this can not do.
					 Maybe back-track, put the nul, and keep… a little bit of
					 wiggle room on either side? */
					printf("…continuing \"%.*s\"\n", (int)(delimit.end.c - delimit.start.c), delimit.start.c);
					continue;
				}
			}
			count++;
			printf("\"%.*s\"—%zu\n", (int)(delimit.end.c - delimit.start.c), delimit.start.c, count);
		}
		if(read < sizeof buffer - 1) break;
		on_edge = (delimit.end.c == buffer + sizeof buffer - 1);
	}
	if(ferror(stdin))
		if(errno) perror("stdin"); else fprintf(stderr, "stdin: Error.\n");
	else
		ret_code = EXIT_SUCCESS;
	printf("Count %zu.\n", count);
	return ret_code;
}
