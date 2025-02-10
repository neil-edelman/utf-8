#include "test_sentences.h"
#include "../src/re2c_next_delimit.h"
#include <stdio.h>
#include <assert.h>

int main(void) {
	struct delimit delimit;
	for(const struct test *t = sentences,
		*t_end = t + sizeof sentences / sizeof *sentences;
		t < t_end; t++) {
		size_t count = 0;
		for(delimit.end.c = t->sentence; ; ) {
			re2c_next_delimit(&delimit);
			if(!delimit.start.c) break;
			printf("\"%.*s\"\n", (int)(delimit.end.c - delimit.start.c), delimit.start.c);
			count++;
		}
		printf("\"%s\": %zu; supposed to be %zu.\n", t->sentence, count, t->delimit_count_basic);
		assert(count == t->delimit_count_basic);
	}

	return 0;
}
