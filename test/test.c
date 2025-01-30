#include "../src/mutf-8.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>


int main(void) {
	const char *const samples[] = {
		"",
		"—",
		"我",
		"我會極不力出調裡據程那變。投那來因？",
		"They said, \"How gœs 'arr'mates' ' oh—no …\""
	};
	for(const char *const*i = samples,
		*const*const i_end = i + sizeof samples / sizeof *samples;
		i < i_end; i++) {
		printf("<%s>: %zu\n", *i, mutf8_code_point_count(*i));
	}
	printf("%s: %zu\n", samples[3], mutf8_code_point_count(samples[3]));
	printf("%zu\n", sizeof samples / sizeof *samples);
	return EXIT_SUCCESS;
}
