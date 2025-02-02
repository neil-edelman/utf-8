#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>
#include <stdlib.h>

/* <http://c-faq.com/misc/bitsets.html>, except reversed for msb-first. */
#define TRIE_MASK(n) ((1 << CHAR_BIT - 1) >> (n) % CHAR_BIT)
#define TRIE_SLOT(n) ((n) / CHAR_BIT)
#define QUERY(a, n) ((a)[TRIE_SLOT(n)] & TRIE_MASK(n))

struct ascii_trie_branch { uint16_t left : 10, skip : 6; };
static const struct word_trie { struct ascii_trie_branch branch[5]; char leaf[6]; }
	word_trie = { { { 1, 1 }, { 0, 2 }, { 0, 1 }, { 0, 0 }, { 0, 0 } },
	{ '0', ':', 'A', '[', 'a', '{' } };

static bool is_prefix(const char *prefix, const char *word) {
	for( ; ; prefix++, word++) {
		if(*prefix == '\0') return true;
		if(*prefix != *word) return false;
	}
}

#include <stdio.h>

static bool is_word(const char *const codepoint) {
	/* Doesn't check for '\0'; assumes well-formed. */
	unsigned br0 = 0, br1 = 6, lf = 0;
	struct { unsigned cur, next; } bit = { 0, 0 };
	printf("Comparing \"%s\" %u%u%u%u %u%u%u%u.\n", codepoint,
		!!(*codepoint&128), !!(*codepoint&64), !!(*codepoint&32), !!(*codepoint&16),
		!!(*codepoint&8), !!(*codepoint&4), !!(*codepoint&2), !!(*codepoint&1));
	while(br0 < br1) {
		const struct ascii_trie_branch *const branch = word_trie.branch + br0;
		printf("At the top. Branches [%u,%u], leaf %u.\n", br0, br1, lf);
		for(bit.next = bit.cur + branch->skip; bit.cur < bit.next; bit.cur++) {
			printf("Bit %u: %u. Care-bit %u. Don't-care.\n", bit.cur, !!QUERY(codepoint, bit.cur), bit.next);
			if(QUERY(codepoint, bit.cur)) { /* Arg 1, trie 0: off most-end. */
				if(!QUERY(word_trie.leaf + lf, bit.cur)) {
					lf += br1 - br0 - 1 /*?*/;
					printf("Off the most-end. Now leaf %d.\n", lf);
					goto found;
				}
			} else { /* Arg 0, trie 1, off least-end. */
				if(QUERY(word_trie.leaf + lf, bit.cur)) {
					lf--;
					printf("Off the most-end. Now leaf %d.\n", lf);
					goto found;
				}
			}
		}
		printf("Bit %u: %u. Decision-bit.\n", bit.cur, !!QUERY(codepoint, bit.cur));
		/*for(byte.next = (bit += branch->skip) / 8;
			byte.cur <= byte.next; byte.cur++)
			if(codepoint[byte.cur] == '\0') goto finally;*/
		if(!QUERY(codepoint, bit.cur))
			br1 = ++br0 + branch->left;
		else
			br0 += branch->left + 1, lf += branch->left + 1;
		bit.cur++;
	}
found:
	printf("Leaf %d.\n", lf);
	return !(lf & 1);
}

int main(void) {
	assert(!is_word("\0"));
	assert(!is_word(" "));
	assert(!is_word("/"));
	assert(is_word("0"));
	assert(is_word("9"));
	assert(!is_word(":"));
	assert(!is_word("@"));
	assert(is_word("A"));
	assert(is_word("Z"));
	assert(!is_word("["));
	assert(!is_word("`"));
	assert(is_word("a"));
	assert(is_word("z"));
	assert(!is_word("{"));
	assert(!is_word("~"));
	return EXIT_SUCCESS;
}
