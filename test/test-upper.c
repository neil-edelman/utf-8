#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

//fprintf(stderr, " (%zu <-%zu(%0x"PRIx32")-> %zu)\n", low, mid, table[mid], high);
//fprintf(stderr, " (ub(0x%"PRIx32")=#%zu 0x%"PRIx32")\n", key, low, table[low]);

/* <!-- truth */
/** @return The majorant of `key` on the set `table` between (`low`, `high`].
 @order O(log `high` - `low`) */
static size_t upper_bound(
	const uint32_t *const table,
	size_t low, size_t high,
	const uint32_t key) {
	while(low < high) {
		size_t mid = low + (high - low) / 2;
		if(table[mid] <= key) low = mid + 1;
		else high = mid;
	}
	return low;
}
/* --> */

int main(void) {
	unsigned test[] = { 10, 10, 10, 20, 20, 20, 30, 30 };

	printf("On [");
	for(unsigned *t = test, *t_end = test + sizeof test / sizeof *test;
		t < t_end; t++)
		printf("%s%u", t == test ? "" : ", ", *t);
	printf("].\n");
	for(uint32_t i = 0; i < 35; i++) {
		printf("upper(%"PRIu32") = %zu\n",
			i, upper_bound(test, 0, sizeof test / sizeof *test, i));
	}

	return EXIT_SUCCESS;
}
