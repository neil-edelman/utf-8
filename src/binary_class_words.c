/** @license mit @std c11

 I realize that this is just starting on a project that is going to turn into…
 re2c. <https://re2c.org/manual/manual_c.html#encoding-support> has, since
 2019, already there. So that's quite easy, I guess. It's even got Graphviz
 support. */

#include "../build/word_edges.h"
#include "binary_is_word.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>

struct word { union { const char *c; const uint8_t *u; } start, end; };

static void next_word(struct word *const w) {
	const char *cursor = w->end.c, *next;
	while(*cursor != '\0') {
		if(!is_word(cursor, &next)) { cursor = next; continue; }
		w->start.c = cursor, cursor = next;
		while(is_word(cursor, &next)) cursor = next;
		w->end.c = cursor;
		return;
	}
	w->start.c = 0;
}

#include <assert.h>

int main(void) {
	const struct test { const char *sentence; size_t word_count; }
	sentences[] = {
		{ "", 0 },
		{ " yo", 1 },
		{ " ,...,,,,.,., ~~~````` — ", 0 },
		{ "They said, \"How gœs 'arr'mates' ' oh—no …\"", 8 },
		{ "one two", 2 },
		{ "one,two", 2 },
		{ "one.two", 2 },
		{ "one_two", 1 },
		{ "one—two", 2 },
		{ "(1-1i)", 2 },
		{ "0.0 a.b", 4 },
		{ "fi f‌iii", 2 }, /* U+200C zwnj is in the second. */
		{ "manœuver…æroplane non-joinerfif‌‌itt‌‌tt good?", 5 }, /* 4 zwnj */
		{ "100,000_000$", 2 },
		{ "zzzz̀abc de f̀ghi", 3 }, /* z U+0300…f U+0300 combining grave accent. */
		{ "나는  Chicago에  산다", 3 }, /* Actually 5? */
		{ "_Ыдентификатор", 1 }, /* From re2c. "_" word. */
		{ "\x80\x80""a a", 2 }, /* Proper handling of non-utf-8. */
		{ "\xc0\x00""a a a a a a", 0 },
		{ "\xe0\x00""a a a a a a", 0 },
		{ "\xf0\x00""a a a a a a", 0 },
		{ "\xf8\x00""a a a a a a", 0 },
		{ "\xf8""a a a a a a", 6 },
		{ "我", 1 },
		{ "我會極不力出調裡據程那變。投那來因？", 2 }
	};
	struct word word;
	for(const struct test *t = sentences,
		*t_end = t + sizeof sentences / sizeof *sentences;
		t < t_end; t++) {
		size_t count = 0;
		for(word.end.c = t->sentence; ; ) {
			next_word(&word);
			if(!word.start.c) break;
			printf("\"%.*s\"\n", (int)(word.end.c - word.start.c), word.start.c);
			count++;
		}
		printf("\"%s\": %zu; supposed to be %zu.\n", t->sentence, count, t->word_count);
		assert(count == t->word_count);
	}

	return EXIT_SUCCESS;
}
