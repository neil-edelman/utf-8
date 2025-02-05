#include "unicode.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

int main(void) {
	int ret_val = EXIT_FAILURE;
	errno = 0;
	struct unicode_deque storage = unicode_load();
	if(!storage.back) goto catch;
	for(struct unicode_deque_cursor cur = unicode_deque_begin(&storage);
		unicode_deque_exists(&cur); unicode_deque_next(&cur)) {
		const struct unicode *const x = unicode_deque_entry(&cur);
		printf("Unicode: 0x%"PRIu32".\n", x->unicode);
	}
	ret_val = EXIT_SUCCESS;
	goto finally;
catch:
	perror("main");
finally:
	unicode_deque_(&storage);
	return ret_val;
}
