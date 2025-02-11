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

	// what we need is a model that says exactly how all the bytes behave.

	// "00000000"/ always terminates
	// "0yyyzzzz"/ otherwise utf-8

	// "110xxxyy" "10yyxxxx"/ 2-byte utf-8
	// "110-----"/ !"10------" 1-byte error

	// "1110wwww" "10xxxxyy" "10yyzzzz"/ 3-byte utf-8
	// "1110----" "10------"/ !"10------" 2-byte error
	// "1110----"/ !"10------" 1-byte error

	// "11110uvv" "10vvwwww" "10xxxxyy" "10yyzzzz"/ 4-byte utf-8
	// "11110---" "10------" "10------"/ !"10------" 3-byte error
	// "11110---" "10------"/ !"10------" 2-byte error
	// "11110---"/ !"10------" 1-byte error

	// ("11111---" | "10------")+ otherwise is at least 1-byte error (64+8=72)

	// we don't worry about splitting errors up; they are still errors

	// so for the end -2 byte, we (might) be in the middle of a valid
	// code-point,
	//                   /110-----
	//                   /1110----
	//                   /11110---
	//          /1110---- 10------
	//          /11110--- 10------
	// /11110--- 10------ 10------
	// all others are we can set it to the end
	//                    0-------/
	//           110----- 10------/
	//  1110---- 10------ 10------/
	//  ******** 10------ 10------/ (otherwise error gets cut)
	//           ******** 10------/ (otherwise error gets cut)
	//  10------ 10------ 10------/ (error or 4-bytes)
	//                    11111---/ (error gets cut)

	// 127
	// 2047
	// 65535
	// 2097151 instead of…
	// 2164910 (https://stackoverflow.com/q/15668718/2472827)

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
