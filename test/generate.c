#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
//#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

/* X-macro from
 <https://www.unicode.org/reports/tr44/#General_Category_Values>. */
#define CATEGORIES X(WTF), X(Cc), X(Cf), X(Co), X(Cs), \
	X(Ll), X(Lm), X(Lo), X(Lt), X(Lu), X(Mc), X(Me), X(Mn), \
	X(Nd), X(Nl), X(No), X(Pc), X(Pd), X(Pe), X(Pf), X(Pi), X(Po), X(Ps), \
	X(Sc), X(Sk), X(Sm), X(So), X(Zl), X(Zp), X(Zs)

#define X(n) #n
static const char *category_strings[] = { CATEGORIES };
#undef X

/** Note. Utf-8 disallows overlong encodings. Should be "modified utf-8" which
 uses one overlong encoding of U+0000 as "0xC0, 0x80" to solve it conflicting
 with '\0' as nul-termination. We just won't care about that. */
struct unicode {
	uint32_t unicode;
#define X(n) n
	enum character_category { CATEGORIES } category;
#undef X
	char utf8[5]; /* In utf-8. */
	uint8_t bytes;
	uint8_t is_word;
};

/* Visualization trie. The Patricia tree has don't-care bits, so is not
 applicable to inverse-range-queries. (Nice try, tough.) */
static const char *unicode_key(const struct unicode *const*const u)
	{ return (*u)->utf8; }
#define TRIE_NAME unicode
#define TRIE_ENTRY struct unicode *
#define TRIE_TO_STRING
#include "../src/trie.h"

static void unicode_to_string(const struct unicode *const u, char (*const a)[12])
	{ sprintf(*a, "U+%x", u->unicode % 0x1000000); }
#define DEQUE_NAME unicode
#define DEQUE_TYPE struct unicode
#define DEQUE_FRONT
#define DEQUE_TO_STRING
#include "../src/deque.h"

static void print_unicode(const struct unicode *const u) {
	char a[12];
	unicode_to_string(u, &a);
	fprintf(stderr, "%s: (%"PRIu8")", a, u->bytes);
	assert(u->bytes < 4);
	for(unsigned i = 0; i < u->bytes; i++) {
		fprintf(stderr, " %.2x", u->utf8[i]);
	}
	fprintf(stderr, " \"%s\" %s%s.\n", u->utf8, category_strings[u->category],
		u->is_word ? " belongs to a word" : "");
}

int main(void) {
	const char *errmsg = 0;
	const char *const unicode_fn = "UnicodeData.txt";
	FILE *unicode_fp = 0;
	char read[256];
	struct unicode_deque storage = unicode_deque();
	struct unicode *u = 0;
	errno = 0;

	/* Load the unicode data into `deque`.
	 <https://www.unicode.org/Public/UNIDATA/UnicodeData.txt> */
	if(!(unicode_fp = fopen(unicode_fn, "r"))) { errmsg = unicode_fn; goto catch; }
	fprintf(stderr, "Opened \"%s\" for reading.\n", unicode_fn);
	while(fgets(read, sizeof read, unicode_fp)) {
		/* `sscanf` is a hack; whatever. */
		struct { unsigned unicode; char category[4]; } input;
		if(sscanf(read, "%x;%*[^;];%3[^;]", &input.unicode,
			input.category) != 2) { errmsg = unicode_fn; goto catch; };
		if(!(u = unicode_deque_new_back(&storage))) goto catch;
		u->unicode = input.unicode;
		if(input.unicode < 0x80) {
			u->bytes = 1;
			u->utf8[0] = (char)input.unicode;
			u->utf8[1] = '\0';
		} else if(input.unicode < 0x0800) {
			u->bytes = 2;
			u->utf8[0] = 0xc0 | (char) (input.unicode >>  6u);
			u->utf8[1] = 0x80 | (char)( input.unicode         & 0x3f);
			u->utf8[2] = '\0';
		} else if(input.unicode < 0x010000) {
			u->bytes = 3;
			u->utf8[0] = 0xe0 | (char)(input.unicode  >> 12u);
			u->utf8[1] = 0x80 | (char)((input.unicode >>  6u) & 0x3f);
			u->utf8[2] = 0x80 | (char)( input.unicode         & 0x3f);
			u->utf8[3] = '\0';
		} else if(input.unicode < 0x110000) {
			u->bytes = 4;
			u->utf8[0] = 0xf0 | (char) (input.unicode >> 18u);
			u->utf8[1] = 0x80 | (char)((input.unicode >> 12u) & 0x3f);
			u->utf8[2] = 0x80 | (char)((input.unicode >>  6u) & 0x3f);
			u->utf8[3] = 0x80 | (char)( input.unicode         & 0x3f);
			u->utf8[4] = '\0';
		} else {
			fprintf(stderr, "Only supports 0x110_000 code points in unicode 16.0.0 #44.\n");
			errno = ERANGE;
			errmsg = unicode_fn;
			goto catch;
		}
		enum character_category cc = WTF;
		switch(input.category[0]) {
		case 'C':
			switch(input.category[1]) {
			case 'c': cc = Cc; break;
			case 'f': cc = Cf; break;
			case 'o': cc = Co; break;
			case 's': cc = Cs; break;
			}
			break;
		case 'L':
			switch(input.category[1]) {
			case 'l': cc = Ll; break;
			case 'm': cc = Lm; break;
			case 'o': cc = Lo; break;
			case 't': cc = Lt; break;
			case 'u': cc = Lu; break;
			}
			break;
		case 'M':
			switch(input.category[1]) {
			case 'c': cc = Mc; break;
			case 'e': cc = Me; break;
			case 'n': cc = Mn; break;
			}
			break;
		case 'N':
			switch(input.category[1]) {
			case 'd': cc = Nd; break;
			case 'l': cc = Nl; break;
			case 'o': cc = No; break;
			}
			break;
		case 'P':
			switch(input.category[1]) {
			case 'c': cc = Pc; break;
			case 'd': cc = Pd; break;
			case 'e': cc = Pe; break;
			case 'f': cc = Pf; break;
			case 'i': cc = Pi; break;
			case 'o': cc = Po; break;
			case 's': cc = Ps; break;
			}
			break;
		case 'S':
			switch(input.category[1]) {
			case 'c': cc = Sc; break;
			case 'k': cc = Sk; break;
			case 'm': cc = Sm; break;
			case 'o': cc = So; break;
			}
			break;
		case 'Z':
			switch(input.category[1]) {
			case 'l': cc = Zl; break;
			case 'p': cc = Zp; break;
			case 's': cc = Zs; break;
			}
			break;
		default: break;
		}
		if(cc == WTF) {
			fprintf(stderr, "Character %u unknown category %s.\n", input.unicode, input.category);
			errno = ERANGE;
			errmsg = unicode_fn;
			goto catch;
		}
		u->category = cc;
		u->is_word = (cc == Ll || cc == Lu || cc == Lt || cc == Lo || cc == Nd);
	}
	fclose(unicode_fp), unicode_fp = 0;
	unicode_deque_graph_fn(&storage, "storage.gv");

	/* Output C. */
	struct unicode_trie bytetrie[4] = { {0}, {0}, {0}, {0} };
	bool is_in_word = false, is_first = true;
	struct { size_t nots, words; } count = { 0, 0 };
	unsigned bytes_prev = 0;
	for(struct unicode_deque_cursor cur = unicode_deque_begin(&storage);
		unicode_deque_exists(&cur); unicode_deque_next(&cur)) {
		struct unicode **put_data_here;
		u = unicode_deque_entry(&cur);
		/* We rely on sorted date so that we can not come back to the same
		 byte-count. Otherwise each trie would have it's own. */
		if(u->bytes != bytes_prev) { is_in_word = false; bytes_prev = u->bytes; }
		/* Put it in trie if it's a rising-or-falling-edge. */
		if(!(is_in_word ^ u->is_word)) continue;
		is_in_word = u->is_word;
		switch(unicode_trie_add(&bytetrie[u->bytes - 1], u->utf8, &put_data_here)) {
		case TRIE_PRESENT:
			fprintf(stderr, "Has duplicate code-points.\n");
			errno = EDOM;
		case TRIE_ERROR: errmsg = "output"; goto catch;
		case TRIE_ABSENT:
			*put_data_here = u;
			if(u->is_word) count.words++; else count.nots++;
			fprintf(stderr, "%s\"%s\"", is_first ? "" : ", ", u->utf8);
			is_first = false;
			break;
		}
	}
	printf(".\n");

	/* Detect the endianness. */
	const union { uint32_t a; uint8_t b[4]; } test = { .a = 1 };
	fprintf(stderr, "(%"PRIu32") == (%"PRIu8",%"PRIu8",%"PRIu8",%"PRIu8")\n",
		test.a, test.b[0], test.b[1], test.b[2], test.b[3]);
	assert((test.b[0] == 1) ^ (test.b[3] == 1) && !test.b[1] && !test.b[2]);
	bool right_to_left = (test.b[0] ? true : false);
	fprintf(stderr, "It's %s.\n",
		right_to_left ? "right-to-left" : "left-to-right");

	/* Output the programme. */
	for(unsigned i = 0; i < 4; i++) {
		char trie_name[32];
		bool first = true;
		sprintf(trie_name, "trie-%u-byte.gv", i + 1);
		unicode_trie_graph_all(&bytetrie[i], trie_name, 0);
		printf("static char uniedge[%u] = {\n"
			"\t", i);
		for(struct unicode_trie_cursor cur = unicode_trie_begin(&bytetrie[i]);
			unicode_trie_exists(&cur); unicode_trie_next(&cur)) {
			u = *unicode_trie_entry(&cur);
			uint32_t utf8code = 0; /* fixme: endianness. */
			bool first_inner = true;
			for(unsigned byte = 0; byte < u->bytes; byte++) {
				printf("%s(0x%"PRIx8")", first_inner ? "" : ":", (uint8_t)u->utf8[byte]);
				first_inner = false;
			}
			for(unsigned byte = 0; byte < u->bytes; byte++) {
				if(right_to_left)
					utf8code |= (uint32_t)(uint8_t)u->utf8[byte] << (byte * 8u);
				else
					utf8code = (utf8code << 8u) | (uint8_t)u->utf8[byte];
			}
			printf("->"/*"%s"*/"0x%"PRIx32"\"%s\"\n", /*first ? "" : ", ",*/ utf8code, u->utf8);
			first = false;
		}
		printf("\n"
			"};\n");
	}

	fprintf(stderr, "There are %zu words and %zu not-words. Total %zu.\n", count.words, count.nots, count.words + count.nots);

	goto finally;
catch:
	perror(errmsg);
finally:
	for(size_t i = 0; i < 4; i++) unicode_trie_(&bytetrie[i]);
	unicode_deque_(&storage);
	if(unicode_fp) fclose(unicode_fp), unicode_fp = 0;
	return errmsg ? EXIT_FAILURE : EXIT_SUCCESS;
}
