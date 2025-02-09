#include "../src/unicode.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

/** fixme: This is not really a test. */
int main(void) {
	int ret_val = EXIT_FAILURE;
	errno = 0;
	struct unicode_deque info = unicode_load();
	if(!info.back) goto catch;
	unicode_deque_graph_fn(&info, "unicode.gv");
	for(struct unicode_deque_cursor cur = unicode_deque_begin(&info);
		unicode_deque_exists(&cur); unicode_deque_next(&cur)) {
		const struct unicode *const x = unicode_deque_entry(&cur);
		printf("Unicode: 0x%"PRIu32".\n", x->unicode);
	}
	ret_val = EXIT_SUCCESS;
	goto finally;
catch:
	perror("Unicode test");
finally:
	unicode_deque_(&info);
	return ret_val;
}
