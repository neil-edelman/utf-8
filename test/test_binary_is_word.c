/** @license mit @std c11 */

#include "../build/word_edges.h"
#include "../src/binary_is_word.h"
#include "../src/unicode.h"
#include <inttypes.h>
#include <stdio.h>
#include <assert.h>

int main(void) {
	int retval = EXIT_FAILURE;
	struct unicode_deque info = unicode_load();
	if(!info.back) goto catch;

	for(struct unicode_deque_cursor i = unicode_deque_begin(&info);
		unicode_deque_exists(&i); unicode_deque_next(&i)) {
		const struct unicode *const u = unicode_deque_entry(&i);
		char *next;
		bool is_from_catalog
			= u->category == Ll || u->category == Lu || u->category == Lt
			|| u->category == Lo || u->category == Nd,
			is_from_word = is_word(u->utf8, &next);
		printf("U+%"PRIx32":0x%"PRIx32": %s (%s).\n",
			u->unicode, u->internal.uint, is_from_word ? "yes" : "no",
			is_from_catalog ? "yes" : "no");
		assert(is_from_word == is_from_catalog);
		assert(next == u->utf8 + u->utf8_size);
	}

	retval = EXIT_SUCCESS;
	goto finally;
catch:
	perror("test binary is_word");
finally:
	unicode_deque_(&info);
	return retval;
}
