#include "test_sentences.h"
#include "../src/delimit.h"
#include "../src/binary_next_delimit.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(void) {
	struct delimited delimited;
	for(const struct test *t = sentences,
		*t_end = t + sizeof sentences / sizeof *sentences;
		t < t_end; t++) {
		size_t count = 0;
		for(delimited.end.c = t->sentence; ; ) {
			binary_next_delimited(&delimited);
			if(!delimited.start.c) break;
			printf("\"%.*s\"\n", (int)(delimited.end.c - delimited.start.c), delimited.start.c);
			count++;
		}
		printf("\"%s\": %zu; supposed to be %zu.\n", t->sentence, count, t->delimited_count_basic);
		assert(count == t->delimited_count_basic);
	}

	return EXIT_SUCCESS;
}
