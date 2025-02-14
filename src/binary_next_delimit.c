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
	while(*cursor != '\0') {
		/* While it's not a word.
		 Fixme: Checking above and below for different things… probably is a
		 source of inefficiency? Only in non-words. So this is inverted, so
		 0x00 is included, so we must check on that. What if we flipped 0x01 in
		 the table instead of inverting…? On null, the potential function would
		 pass this, and then also pass the second is word, `start == end`.
		 Would that really make that much of a difference? */
		if(!binary_is_delimit(cursor, &next))
			{ cursor = next; continue; }
		/* Now it's a word. */
		w->start.c = cursor, cursor = next;
		while(binary_is_delimit(cursor, &next)) cursor = next;
		w->end.c = cursor;
		return;
	}
	w->start.c = 0;
}
