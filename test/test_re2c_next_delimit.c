#include "test_sentences.h"
#include <stdio.h>

int main(void) {
	const char *s;
	struct word word;
	size_t count;

	#define TEST(str, num) \
	s = str; \
	for(sub.end.c = s, count = 0; ; ) { \
		next_word(&sub); \
		if(!sub.start.c) break; \
		printf("\"%.*s\"\n", (int)(sub.end.c - sub.start.c), sub.start.c); \
		count++; \
	} \
	printf("\"%s\": %zu; supposed to be %zu.\n", s, count, (size_t)num); \
	assert(count == num);

	return 0;
}
