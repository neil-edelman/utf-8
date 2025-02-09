#include "../src/u32_upper_bound.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>

int main(void) {
	const unsigned test[] = { 10, 10, 10, 20, 20, 20, 30, 30 };
	const size_t results[] = { /* For [0–35). */
		/* 0*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/*10*/ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		/*20*/ 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		/*30*/ 8, 8, 8, 8, 8
	};

	printf("On [");
	for(const unsigned *t = test, *t_end = test + sizeof test / sizeof *test;
		t < t_end; t++)
		printf("%s%u", t == test ? "" : ", ", *t);
	printf("].\n");
	for(uint32_t i = 0; i < 35; i++) {
		const size_t r = u32_upper_bound(test, 0, sizeof test / sizeof *test, i);
		printf("upper(%"PRIu32") = %zu, it's actually %zu.\n",
			i, r, results[i]);
		assert(r == results[i]);
	}

	return EXIT_SUCCESS;
}
