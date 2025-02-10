/** @license mit @std c11 */

#include "../src/binary_is_delimited.h"
#include "../src/unicode.h"
#include "test_sentences.h"
#include <inttypes.h>
#include <stdio.h>
#include <assert.h>

int main(void) {
	int retval = EXIT_FAILURE;
	struct unicode_deque info = unicode_load();
	if(!info.back) goto catch;

	/* All unicode characters. */
	for(struct unicode_deque_cursor i = unicode_deque_begin(&info);
		unicode_deque_exists(&i); unicode_deque_next(&i)) {
		const struct unicode *const u = unicode_deque_entry(&i);
		const char *next;
		/* fixme: Shouldn't be pasting stuff. */
		bool is_from_catalog = u->category == Ll || u->category == Lu
			|| u->category == Lt || u->category == Lm || u->category == Lo
			|| u->category == Mc || u->category == Me || u->category == Mn
			|| u->category == Nd || u->category == Nl || u->category == No
			|| u->category == Pc
			|| u->unicode == 0x200b /* zwsp */
			|| u->unicode == 0x200c /* zwnj */
			|| u->unicode == 0x200d /* zwj */
			|| u->unicode == 0x2060, /* wj */
			is_from_word = binary_is_delimited(u->utf8, &next);
		printf("U+%"PRIx32":0x%"PRIx32": %s (%s).\n",
			u->unicode, u->internal.uint, is_from_word ? "yes" : "no",
			is_from_catalog ? "yes" : "no");
		assert(is_from_word == is_from_catalog);
		assert(next == u->utf8 + u->utf8_size);
	}

	retval = EXIT_SUCCESS;
	goto finally;
catch:
	perror("test binary is delimited");
finally:
	unicode_deque_(&info);
	return retval;
}
