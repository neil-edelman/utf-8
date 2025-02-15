#include "test_sentences.h"
#include "../src/delimit.h"
#include "../src/binary_next_delimit.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

int main(void) {
	int ret_code = EXIT_FAILURE;
	errno = 0;

	/* "ab  c" 2. buffer[3]: 1. Fixed. */
	/* "𝍠 𝍡 𝍢 𝍣 𝍤" 5. buffer[5]: 2. Fixed. */

//#define SHOW
	//freopen("UnicodeData.txt", "r", stdin);
	/* wc 161479, [5] 390032, [32768] 390032.
	 `wc` delimits words by `isspace`, so the first line,
	 "0000;<control>;Cc;0;BN;;;;;N;NULL;;;;" `wc` would say 1. `binary_delimit`
	 uses "[\p{L}\p{M}\p{N}\p{Pc}\u200b\u200c\u200d\u2060]+", which parses that,
	 "0000  control  Cc 0 BN     N NULL    ", would say 7.
	 Count 390032. */

	struct {
		size_t read, want, end;
		enum { ZERO, ONE, TWO, THREE } assist_size;
		/* Pieces of code-point from end; size-terminated. `nul`-terminated. */
		union { char c; uint8_t u; } assist[3], utf8[/*5*//*8192*/32768];
	} buffer;
	buffer.assist_size = ZERO;
	assert(sizeof buffer.assist >= 3 && sizeof buffer.utf8 > 4);

	struct {
		struct delimit delimit;
		size_t count;
		int on_edge;
	} find = {0};

	/* Re-fill the buffer with a utf-8 source. */
	while(buffer.read = fread(buffer.utf8 + buffer.assist_size, 1,
		buffer.want = sizeof buffer.utf8 - 1 - buffer.assist_size, stdin)) {
		assert(buffer.read <= buffer.want
			&& buffer.assist_size + buffer.read <= sizeof buffer.utf8 - 1);

		/* Copy code-point leftovers from last time. */
		switch(buffer.assist_size) {
		case THREE: buffer.utf8[2].c = buffer.assist[2].c; /* Fall… */
		case TWO:   buffer.utf8[1].c = buffer.assist[1].c; /* Fall… */
		case ONE:   buffer.utf8[0].c = buffer.assist[0].c;
		case ZERO:  break;
		}
		buffer.end = buffer.assist_size + buffer.read; /* So far, but… */
		buffer.assist_size = ZERO;

		/* We might have truncated a code-point this time. */
		if(buffer.read == buffer.want) {
			uint8_t *u = &buffer.utf8[buffer.end - 1].u;
			switch(*u & 0xc0) {
			case 0x00:
			case 0x40: break; /* 1-byte. */
			case 0xc0:
				/* "/110-----", "/1110----", "/11110---" */
				if((*u & 0xf8) != 0xf8)
					buffer.assist_size = ONE,
					buffer.assist[0].u = u[0];
				/* …otherwise error, "11111---/", cut anywhere. */
				break;
			case 0x80: /* "10------" continuation byte. */
				u--;
				switch(*u & 0xf8) {
				case 0xf0: /* "/11110--- 10------" */
				case 0xe0: /* "/1110---- 10------" */
				case 0xe8:
					buffer.assist_size = TWO;
					buffer.assist[0].u = u[0];
					buffer.assist[1].u = u[1];
					break;
				case 0x80: /* "10------ 10------" continuation byte. */
				case 0x88:
				case 0x90:
				case 0x98:
				case 0xa0:
				case 0xa8:
				case 0xb0:
				case 0xb8:
					u--;
					/* /"11110---" "10------" "10------" */
					if((*u & 0xf8) == 0xf0)
						buffer.assist_size = THREE,
						buffer.assist[0].u = u[0],
						buffer.assist[1].u = u[1],
						buffer.assist[2].u = u[2];
					/* …otherwise error. */
					break;
				default: break;
				}
				/* …otherwise error. */
				break;
			}
		}
		/* `nul`-terminate the rest. */
		buffer.utf8[buffer.end -= buffer.assist_size].c = '\0';

		for(find.delimit.end.c = &buffer.utf8[0].c;
			binary_next_delimit(&find.delimit),
			find.delimit.start.c != find.delimit.end.c; ) {
			if(!find.on_edge); else {
				find.on_edge = 0;
				if(find.delimit.start.c == &buffer.utf8[0].c) {
#ifdef SHOW
					printf("…continuing \"%.*s\"\n", (int)(find.delimit.end.c - find.delimit.start.c), find.delimit.start.c);
#endif
					continue; /* `for`, not `while`. */
				}
			}
			find.count++;
#ifdef SHOW
			printf("\"%.*s\"—%zu\n", (int)(find.delimit.end.c - find.delimit.start.c), find.delimit.start.c, find.count);
#endif
		}
		if(buffer.read < buffer.want) break; /* The last. */
		find.on_edge = (find.delimit.end.c == &buffer.utf8[0].c + buffer.end);
	}
	if(ferror(stdin))
		if(errno) perror("stdin"); else fprintf(stderr, "stdin: Error.\n");
	else
		ret_code = EXIT_SUCCESS;
	printf("Count %zu.\n", find.count);
	return ret_code;
}
