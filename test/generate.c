#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>

/* X-macro from
 <https://www.unicode.org/reports/tr44/#General_Category_Values>. */
#define CATEGORIES X(WTF), X(Cc), X(Cf), X(Co), X(Cs), \
	X(Ll), X(Lm), X(Lo), X(Lt), X(Lu), X(Mc), X(Me), X(Mn), \
	X(Nd), X(Nl), X(No), X(Pc), X(Pd), X(Pe), X(Pf), X(Pi), X(Po), X(Ps), \
	X(Sc), X(Sk), X(Sm), X(So), X(Zl), X(Zp), X(Zs)

/*#define X(n) #n
static const char *category_strings[] = { CATEGORIES };
#undef X*/

/** Note. Utf-8 disallows overlong encodings. Should be "modified utf-8" which
 uses one overlong encoding of U+0000 as "0xC0, 0x80" to solve it conflicting
 with '\0' as nul-termination. We just won't care about that. */
struct unicode {
#define X(n) n
	enum character_category { CATEGORIES } category;
#undef X
	char utf8codepoint[5];
	uint8_t bytes;
	uint8_t is_word;
};

/* Visualization trie. The Patricia tree has don't-care bits, so is not
 applicable to inverse-range-queries. (Nice try, tough.) */
static const char *unicode_key(const struct unicode *const*const u)
	{ return (*u)->utf8codepoint; }
#define TRIE_NAME unicode
#define TRIE_ENTRY struct unicode *
#define TRIE_TO_STRING
#include "../src/trie.h"

static void unicode_to_string(const struct unicode *const*const u, char (*const a)[12]) {
	const char *const uni = unicode_key(u);
	strcpy(*a, unicode_key(u));
}
#define DEQUE_NAME unicode
#define DEQUE_TYPE struct unicode
#define DEQUE_FRONT
#define DEQUE_TO_STRING
#include "../src/deque.h"

int main(void) {
	const char *errmsg = 0;
	const char *const unicode_fn = "UnicodeData.txt";
	FILE *unicode_fp = 0;
	char read[256];
	struct unicode_deque storage = unicode_deque();
	struct unicode_trie bytetrie[4] = { {0}, {0}, {0}, {0} };
	struct unicode *u = 0;
	errno = 0;
	bool first = true;

	/* Load the unicode data into `deque`.
	 <https://www.unicode.org/Public/UNIDATA/UnicodeData.txt> */
	if(!(unicode_fp = fopen(unicode_fn, "r"))) { errmsg = unicode_fn; goto catch; }
	fprintf(stderr, "Opened \"%s\" for reading.\n", unicode_fn);
	while(fgets(read, sizeof read, unicode_fp)) {
		/* `sscanf` is a hack; whatever. */
		struct { unsigned unicode; char category[4]; } input;
		if(sscanf(read, "%x;%*[^;];%3[^;]", &input.unicode,
			input.category) != 2) { errmsg = unicode_fn; goto catch; };
		if(input.unicode >= 0x80) continue; /* Debug ascii. Much smaller set. */
		if(!(u = unicode_deque_new_back(&storage))) goto catch;
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
		if(input.unicode < 0x80) {
			u->utf8codepoint[0] = (char)input.unicode;
			u->utf8codepoint[1] = '\0';
		} else if(input.unicode < 0x0800) {
			u->utf8codepoint[0] = 0xc0 | (char) (input.unicode >>  6u);
			u->utf8codepoint[1] = 0x80 | (char)( input.unicode         & 0x3f);
			u->utf8codepoint[2] = '\0';
		} else if(input.unicode < 0x010000) {
			u->utf8codepoint[0] = 0xe0 | (char)(input.unicode  >> 12u);
			u->utf8codepoint[1] = 0x80 | (char)((input.unicode >>  6u) & 0x3f);
			u->utf8codepoint[2] = 0x80 | (char)( input.unicode         & 0x3f);
			u->utf8codepoint[3] = '\0';
		} else if(input.unicode < 0x110000) {
			u->utf8codepoint[0] = 0xf0 | (char) (input.unicode >> 18u);
			u->utf8codepoint[1] = 0x80 | (char)((input.unicode >> 12u) & 0x3f);
			u->utf8codepoint[2] = 0x80 | (char)((input.unicode >>  6u) & 0x3f);
			u->utf8codepoint[3] = 0x80 | (char)( input.unicode         & 0x3f);
			u->utf8codepoint[4] = '\0';
		} else {
			fprintf(stderr, "Only supports 0x110_000 code points in unicode 16.0.0.\n");
			errno = ERANGE;
			errmsg = unicode_fn;
			goto catch;
		}
		if(first) {
			//unicode_deque_graph_fn(&storage, "first.gv");
			// I can't figure that out.
			first = false;
		}
	}
	fclose(unicode_fp), unicode_fp = 0;
	//unicode_deque_graph_fn(&storage, "storage.gv");

	/* Output C. */
	bool is_in_word = false, is_first = true;
	struct { size_t nots, words; } count = { 0, 0 };
	for(struct unicode_deque_cursor cur = unicode_deque_begin(&storage);
		unicode_deque_exists(&cur); unicode_deque_next(&cur)) {
		struct unicode **put_data_here;
		u = unicode_deque_entry(&cur);
		/* Put both entries in the trie if it's a rising-or-falling-edge. */
		if(!(is_in_word ^ u->is_word)) continue;
		is_in_word = u->is_word;
		switch(unicode_trie_add(&bytetrie[u->bytes], u->utf8codepoint, &put_data_here)) {
		case TRIE_PRESENT:
			fprintf(stderr, "Has duplicate code-points.\n");
			errno = EDOM;
		case TRIE_ERROR: errmsg = "output"; goto catch;
		case TRIE_ABSENT:
			*put_data_here = u;
			if(u->is_word) count.words++; else count.nots++;
			printf("%s\"%s\"", is_first ? "" : ", ", u->utf8codepoint);
			is_first = false;
			break;
		}
	}
	printf(".\n");
	unicode_trie_graph_all(&bytetrie[0], "trie0.gv", 0);

#if 0

#define ONLY_2_BYTES
#ifdef ONLY_1_BYTE
		if(input.unicode >= 0x80) continue;
#elif defined ONLY_2_BYTES
		if(input.unicode < 0x80 || input.unicode >= 0x0800) continue;
#elif defined ONLY_3_BYTES
		if(input.unicode < 0x8000 || input.unicode >= 0x010000) continue;
#elif defined ONLY_4_BYTES
		if(input.unicode < 0x010000 || input.unicode >= 0x110000) continue;
#endif



	}
	if(ferror(uni_fp)) goto catch; /* May not produce a meaningful error. */
	mutf8_trie_graph_all(&trie, "graph.gv", 0);

	/*char *test = "a";
	mutf8 = mutf8_trie_get(&trie, test);
	fprintf(stderr, "\"%s\" is %s and %s in the set of word-code-points.\n",
		test, category_strings[mutf8->category],
		mutf8->word ? "belongs" : "does not belong");*/
	fprintf(stderr, "There are %zu words and %zu not-words. Total %zu.\n", count.words, count.nots, count.words + count.nots);
#endif

	goto finally;
catch:
	perror(errmsg);
finally:
	unicode_trie_(&bytetrie);
	unicode_deque_(&storage);
	if(unicode_fp) fclose(unicode_fp), unicode_fp = 0;
	return errmsg ? EXIT_FAILURE : EXIT_SUCCESS;
}
