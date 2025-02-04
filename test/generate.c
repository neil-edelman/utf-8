/* @license MIT @std C11 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>

#define ENDIAN X(little), X(big)
#define X(n) n
static enum { ENDIAN } endian;
#undef X
#define X(n) #n
static const char *endian_strings[] = { ENDIAN };
#undef X

/* <https://www.unicode.org/reports/tr44/#General_Category_Values>. */
#define CATEGORIES X(WTF), X(Cc), X(Cf), X(Co), X(Cs), \
	X(Ll), X(Lm), X(Lo), X(Lt), X(Lu), X(Mc), X(Me), X(Mn), \
	X(Nd), X(Nl), X(No), X(Pc), X(Pd), X(Pe), X(Pf), X(Pi), X(Po), X(Ps), \
	X(Sc), X(Sk), X(Sm), X(So), X(Zl), X(Zp), X(Zs)
#define X(n) n
enum character_category { CATEGORIES };
#undef X
#define X(n) #n
static const char *category_strings[] = { CATEGORIES };
#undef X

static const unsigned tab = 4, wrap = 76;

/** Note. Utf-8 disallows overlong encodings so it conflicts with '\0' as
 nul-termination. Should be "modified utf-8" which uses one overlong encoding
 of U+0000 as "0xC0, 0x80". Might be important in some instances? */
struct unicode {
	uint32_t unicode;
	enum character_category category;
	uint8_t utf8_size;
	char utf8[5];
	union { char utf32byte[4]; uint32_t utf32; };
};

/* Visualization trie. The Patricia tree has don't-care bits, so is not
 applicable to inverse-range-queries. (Nice try, tough.) */
static const char *unicode_key(/*const<-it is not, technically :( */ struct unicode *const*const u)
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

static void print_byte(const struct unicode_trie *const utf8,
	const char *const name) {
	bool first = true;
	unsigned column = tab;
	size_t entries = 0;
	printf("%s = {\n"
		"\t", name);
	for(struct unicode_trie_cursor cur = unicode_trie_begin(utf8);
		unicode_trie_exists(&cur); unicode_trie_next(&cur)) {
		const struct unicode *const u = *unicode_trie_entry(&cur);
		entries++;
		int r = snprintf(0, 0, "%s0x%"PRIx32"", first ? "" : ", ", u->utf32);
		if(r < 0) perror("output"), exit(EXIT_FAILURE);
		if((column += (unsigned)r) > wrap - 1 /*","*/) /* Soft-return. */
			printf(",\n\t"), column = tab + (unsigned)r - 2 /*", "*/,
			first = true;
		printf("%s0x%"PRIx32"", first ? "" : ", ", u->utf32);
		first = false;
	}
	printf("\n"
		"}; /* %zu entries. */\n", entries);
}

int main(void) {
	const char *errmsg = 0;
	errno = 0;

	/* Detect the endianness. */
	const union { uint32_t a; uint8_t b[4]; } test = { .a = 1 };
	assert((test.b[0] == 1) ^ (test.b[3] == 1) && !test.b[1] && !test.b[2]);
	endian = test.b[0] ? little : big;
	fprintf(stderr, "(%"PRIu32") == (%"PRIu8",%"PRIu8",%"PRIu8",%"PRIu8"). "
		"Detected %s-endian.\n", test.a,
		test.b[0], test.b[1], test.b[2], test.b[3], endian_strings[endian]);

	/* Load the unicode data.
	 <https://www.unicode.org/Public/UNIDATA/UnicodeData.txt> */
	const char *const unicode_fn = "UnicodeData.txt";
	FILE *unicode_fp = 0;
	if(!(unicode_fp = fopen(unicode_fn, "r")))
		{ errmsg = unicode_fn; goto catch; }
	fprintf(stderr, "Opened \"%s\" for reading.\n", unicode_fn);
	struct unicode_deque storage = unicode_deque();
	char read[256];
	while(fgets(read, sizeof read, unicode_fp)) {
		/* `sscanf` is a hack; whatever. */
		struct { unsigned unicode; char category[4]; } input;
		if(sscanf(read, "%x;%*[^;];%3[^;]", &input.unicode,
			input.category) != 2) { errmsg = unicode_fn; goto catch; };
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

		/* Load the entire table into memory because we are lazy. */
		struct unicode *u;
		if(!(u = unicode_deque_new_back(&storage))) goto catch;
		u->unicode = input.unicode;
		u->category = cc;
		if(input.unicode < 0x80) {
			u->utf8_size = 1;
			u->utf8[0] = (char)input.unicode;
			u->utf8[1] = '\0', u->utf8[2] = '\0', u->utf8[3] = '\0',
				u->utf8[4] = '\0';
		} else if(input.unicode < 0x0800) {
			u->utf8_size = 2;
			u->utf8[0] = 0xc0 | (char) (input.unicode >>  6u);
			u->utf8[1] = 0x80 | (char)( input.unicode         & 0x3f);
			u->utf8[2] = '\0', u->utf8[3] = '\0', u->utf8[4] = '\0';
		} else if(input.unicode < 0x010000) {
			u->utf8_size = 3;
			u->utf8[0] = 0xe0 | (char)(input.unicode  >> 12u);
			u->utf8[1] = 0x80 | (char)((input.unicode >>  6u) & 0x3f);
			u->utf8[2] = 0x80 | (char)( input.unicode         & 0x3f);
			u->utf8[3] = '\0', u->utf8[4] = '\0';
		} else if(input.unicode < 0x110000) {
			u->utf8_size = 4;
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
		if(endian == big)
			u->utf32byte[0] = u->utf8[3], u->utf32byte[1] = u->utf8[2],
			u->utf32byte[2] = u->utf8[1], u->utf32byte[3] = u->utf8[0];
		else
			u->utf32byte[0] = u->utf8[0], u->utf32byte[1] = u->utf8[1],
			u->utf32byte[2] = u->utf8[2], u->utf32byte[3] = u->utf8[3];
	}
	fclose(unicode_fp), unicode_fp = 0;
	unicode_deque_graph_fn(&storage, "storage.gv");

	/* Pick a property. */
	struct {
		struct unicode_trie trie;
		bool property;
	} bytes[4] = {0};
	for(struct unicode_deque_cursor cur = unicode_deque_begin(&storage);
		unicode_deque_exists(&cur); unicode_deque_next(&cur)) {
		/*const?*/ struct unicode **put_data_here;
		/*const?*/ struct unicode *const u = unicode_deque_entry(&cur);
		bool is_word = u->category == Ll || u->category == Lu
			|| u->category == Lt || u->category == Lo || u->category == Nd;
		/* Put it in trie if it's a rising-or-falling-edge. */
		if(!(bytes[u->utf8_size].property ^ is_word)) continue;
		bytes[u->utf8_size].property = is_word;
		assert(u->utf8_size >= 1 && u->utf8_size <= 4);
		switch(unicode_trie_add(&bytes[u->utf8_size - 1].trie, u->utf8, &put_data_here)) {
		case TRIE_PRESENT:
			fprintf(stderr, "Has duplicate code-points.\n");
			errno = EDOM;
		case TRIE_ERROR: errmsg = "output"; goto catch;
		case TRIE_ABSENT: *put_data_here = u; break;
		}
	}

	/* Output tries. */
	for(unsigned i = 0; i < 4; i++) {
		char trie_name[32];
		sprintf(trie_name, "trie-%u-byte.gv", i + 1);
		unicode_trie_graph_all(&bytes[i].trie, trie_name, 0);
	}

	/* Output the programme. */
	print_byte(&bytes[0].trie, "static const uint32_t utf32_word_edges");
	print_byte(&bytes[1].trie, "static const uint32_t …");
	print_byte(&bytes[2].trie, "static const uint32_t …");
	print_byte(&bytes[3].trie, "static const uint32_t …");

	goto finally;
catch:
	perror(errmsg);
finally:
	for(size_t i = 0; i < 4; i++) unicode_trie_(&bytes[i].trie);
	unicode_deque_(&storage);
	if(unicode_fp) fclose(unicode_fp), unicode_fp = 0;
	return errmsg ? EXIT_FAILURE : EXIT_SUCCESS;
}
