#if 0 /* Xcode is really buggy. */
int main(void) { }

#else
/** @license MIT @std C11

 Relies on `unicode.c` to parse unicode data specifically for utf-8; puts it
 into character category tables. The tables are paired (except possibly the
 last which might be infinite) [min, max) of inclusion in the property. Outputs
 `uint32_t` tables. */

static const unsigned tab = 4, wrap = 76;

#include "../src/unicode.h"
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
static void unicode_to_string(struct unicode *const u, char (*const a)[12]) {
	sprintf(*a, "U+%x", u->unicode);
}
#define TRIE_NAME unicode
#define TRIE_ENTRY struct unicode *
#define TRIE_TO_STRING
#include "../src/trie.h"

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

	/* Pick a property. */
	for(struct unicode_deque_cursor cur = unicode_deque_begin(&info);
		unicode_deque_exists(&cur); unicode_deque_next(&cur)) {
		/*const?*/ struct unicode **put_data_here;
		/*const?*/ struct unicode *const u = unicode_deque_entry(&cur);
		/* <https://unicode.org/reports/tr18/#RL1.4>
		 <https://www.regular-expressions.info/unicode.html>
		 U+feff zwnbsp is deprecated bom; U+05f3 Hebrew Punctuation Geresh?
		 letter = L | M | N | Pc | [\u200b\u200c\u200d\u2060]; */
		bool is_word = u->category == Ll || u->category == Lu
			|| u->category == Lt || u->category == Lm || u->category == Lo
			|| u->category == Mc || u->category == Me || u->category == Mn
			|| u->category == Nd || u->category == Nl || u->category == No
			|| u->category == Pc
			|| u->unicode == 0x200b /* zwsp */
			|| u->unicode == 0x200c /* zwnj */
			|| u->unicode == 0x200d /* zwj */
			|| u->unicode == 0x2060; /* wj */

		/* Put it in trie if it's a rising-or-falling-edge. If using the
		 nul-terminator, can _not_ have U+0000 (Cc) in the range. I think. */
		if(u->unicode == 0x00
			|| !(bytes[u->utf8_size].property ^ is_word)) continue;
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
		sprintf(trie_name, "graph/edges-%u-byte.gv", i + 1);
		unicode_trie_graph_all(&bytes[i].trie, trie_name, 0);
	}

	/* Output the programme. */
	printf("#include <stdint.h>\n\n"
		"static const uint32_t utf8_delimit_edges[] = {\n"
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
	printf("static const size_t utf8_delimit_end[] = { ");
	for(unsigned i = 0; i < 4; i++)
		printf("%s%"PRIu32"", first ? "" : ", ", running += bytes[i].size),
		first = false;
	printf(" };\n");

	goto finally;
catch:
	perror(errmsg);
finally:
	unicode_deque_(&info);
	for(size_t i = 0; i < 4; i++) unicode_trie_(&bytes[i].trie);
	/* Some vfprintf errors and Swift scaffolding causes this to crash? */
	fprintf(stderr, "This is generate_delimit_edges.\n");
	exit(errmsg ? EXIT_FAILURE : EXIT_SUCCESS);
	/*return errmsg ? EXIT_FAILURE : EXIT_SUCCESS;*/
}
#endif
