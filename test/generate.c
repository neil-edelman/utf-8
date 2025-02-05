/** @license MIT @std C11 */

#include "unicode.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>

/* Visualization trie. The Patricia tree has don't-care bits, so is not
 applicable to inverse-range-queries. (Nice try, tough.) */
static const char *unicode_key(/*const<-it is not, technically :( */ struct unicode *const*const u)
	{ return (*u)->utf8; }
#define TRIE_NAME unicode
#define TRIE_ENTRY struct unicode *
#define TRIE_TO_STRING
#include "../src/trie.h"

static const unsigned tab = 4, wrap = 76;

static void print_byte(const struct unicode_trie *const unicode) {
	bool first = true;
	unsigned column = tab;
	for(struct unicode_trie_cursor cur = unicode_trie_begin(unicode);
		unicode_trie_exists(&cur); unicode_trie_next(&cur)) {
		const struct unicode *const u = *unicode_trie_entry(&cur);
		int r = snprintf(0, 0, "%s0x%"PRIx32"", first ? "\t" : ", ",
			u->internal.uint);
		if(r < 0) perror("output"), exit(EXIT_FAILURE);
		if((column += (unsigned)r) > wrap - 1 /*","*/) /* Soft-return. */
			printf(",\n"), column = tab + (unsigned)r - 2 /*", "*/,
			first = true;
		printf("%s0x%"PRIx32"", first ? "\t" : ", ", u->internal.uint);
		first = false;
	}
}

int main(void) {
	const char *errmsg = 0;
	struct unicode_deque info = unicode_deque();
	struct {
		struct unicode_trie trie;
		bool property;
		uint32_t size;
	} bytes[4 /* The number of bytes. */] = {0};
	errno = 0;

	info = unicode_load();
	if(!info.back) goto catch;

	/* Detect the endianness. */
	/*const union { uint32_t a; uint8_t b[4]; } test = { .a = 1 };
	bool little_endian = test.b[0];*/

	/* Pick a property. */
	for(struct unicode_deque_cursor cur = unicode_deque_begin(&info);
		unicode_deque_exists(&cur); unicode_deque_next(&cur)) {
		/*const?*/ struct unicode **put_data_here;
		/*const?*/ struct unicode *const u = unicode_deque_entry(&cur);
		bool is_word = u->category == Ll || u->category == Lu
			|| u->category == Lt || u->category == Lo || u->category == Nd;
		/* fixme: To be in Unicode Level 1,
		 <https://unicode.org/reports/tr18/#RL1.4>
		 "Alphabetic values from the Unicode character" (Lm? we definitely
		 want Lm? Swift has Lm, experimentally.)
		 "U+200C ZERO WIDTH NON-JOINER and U+200D ZERO WIDTH JOINER"
		 */
		/* Put it in trie if it's a rising-or-falling-edge. */
		if(!(bytes[u->utf8_size].property ^ is_word)) continue;
		bytes[u->utf8_size].property = is_word;
		assert(u->utf8_size >= 1 && u->utf8_size <= 4);
		switch(unicode_trie_add(&bytes[u->utf8_size - 1].trie, u->utf8, &put_data_here)) {
		case TRIE_PRESENT:
			fprintf(stderr, "Has duplicate code-points.\n");
			errno = EDOM;
		case TRIE_ERROR: errmsg = "output"; goto catch;
		case TRIE_ABSENT:
			*put_data_here = u;
			bytes[u->utf8_size - 1].size++;
			break;
		}
	}

	/* Output tries. */
	for(unsigned i = 0; i < 4; i++) {
		char trie_name[32];
		sprintf(trie_name, "trie-%u-byte.gv", i + 1);
		unicode_trie_graph_all(&bytes[i].trie, trie_name, 0);
	}

	/* Output the programme. */
	printf("static const uint32_t utf32_word_edges[] = {\n"
		"\t/* %"PRIu32" code-points. */\n", bytes[0].size);
	print_byte(&bytes[0].trie);
	printf(",\n"
		"\t/* %"PRIu32" code-points. */\n", bytes[1].size);
	print_byte(&bytes[1].trie);
	printf(",\n"
		"\t/* %"PRIu32" code-points. */\n", bytes[2].size);
	print_byte(&bytes[2].trie);
	printf(",\n"
		"\t/* %"PRIu32" code-points. */\n", bytes[3].size);
	print_byte(&bytes[3].trie);
	printf("\n"
		"};\n");

	uint32_t running = 0;
	bool first = true;
	printf("static const uint32_t utf32_word_byte_end[] = { ");
	for(unsigned i = 0; i < 4; i++)
		printf("%s%"PRIu32"", first ? "" : ", ", running += bytes[i].size),
		first = false;
	printf(" };\n");

	goto finally;
catch:
	perror(errmsg);
finally:
	fprintf(stderr, "shutting down tries\n");
	for(size_t i = 0; i < 4; i++) unicode_trie_(&bytes[i].trie);
	fprintf(stderr, "shutting down info\n");
	unicode_deque_(&info);
	fprintf(stderr, "return \"%s\".\n", errmsg ? errmsg : "fine");
	return errmsg ? EXIT_FAILURE : EXIT_SUCCESS;
}
