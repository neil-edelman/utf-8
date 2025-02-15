/** @license mit @std c11

 I realize that this is just starting on a project that is going to turn into…
 re2c. <https://re2c.org/manual/manual_c.html#encoding-support> has, since
 2019, already there. So that's quite easy, I guess. It's even got Graphviz
 support. */

#include "binary_is_delimit.h"
#include "delimit.h"
#include "binary_next_delimit.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>

void binary_next_delimit(struct delimit *const w) {
	const char *cursor = w->end.c, *next;
	/* Fixme: Checking for the nul-terminator is a waste. Inverting the
	 delimiters is the wrong approach. We should have two classes, we're in and
	 we're out, of which the compliment of the union is the set { 0x00 }.
	 Stored in offset. At the end, `if(start == end) start = 0`. Would that be
	 that much faster in practice? Doubtful. */
	while(binary_is_not_delimit(cursor, &next)) cursor = next;
	w->start.c = cursor;
	while(binary_is_delimit(cursor, &next)) cursor = next;
	w->end.c = cursor;

#if 0
	while(*cursor != '\0') {
		/* While it's not a word. */
		if(!binary_is_delimit(cursor, &next))
			{ cursor = next; continue; }
		/* Now it's a word. */
		w->start.c = cursor, cursor = next;
		while(binary_is_delimit(cursor, &next)) cursor = next;
		w->end.c = cursor;
		return;
	}
	w->start.c = 0;
#endif
}
