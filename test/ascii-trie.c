#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>
#include <stdlib.h>

#include <stdio.h>

static size_t upper_1byte_bound(const char *const nondec, size_t size, const char target) {
	size_t lo = 0, hi = size, count = hi - lo;
	while(count) {
		size_t step = count / 2;
		if(/*!*/(target /*<*/ >= nondec[lo + step])) lo = step + 1, count -= step + 1;
		else count = step;
	}
	return lo;
}

static bool is_1byte_word(const char *const codepoint) {
	static const char words[] = { '0', ':', 'A', '[', 'a', '{' };
	return upper_1byte_bound(words, sizeof words / sizeof *words, *codepoint) & 1;
}

static bool is_word2(const char *const codepoint) {
	static const char words[][2] = {
		"ª", "«", "µ", "¶", "º", "»", "À", "×", "Ø", "÷", "ø", "ʰ",
		"Ͱ", "ʹ", "Ͷ", "ͺ", "ͻ", ";", "Ϳ", "΄", "Ά", "·", "Έ", "϶",
		"Ϸ", "҂", "Ҋ", "ՙ", "ՠ", "։", "א", "׳", "ؠ",
		"ـ", "ف", "ً", "٠", "٪", "ٮ",
		"ٰ", "ٱ", "۔", "ە", "ۖ", "ۮ", "۽", "ۿ", "܀", "ܐ", "ܑ", "ܒ", "ܰ", "ݍ",
		"ަ", "ޱ", "߫"
	};
	return false;
}

int main(void) {
	assert(!is_1byte_word("\0"));
	assert(!is_1byte_word(" "));
	assert(!is_1byte_word("/"));
	assert(is_1byte_word("0"));
	assert(is_1byte_word("9"));
	assert(!is_1byte_word(":"));
	assert(!is_1byte_word("@"));
	assert(is_1byte_word("A"));
	assert(is_1byte_word("Z"));
	assert(!is_1byte_word("["));
	assert(!is_1byte_word("`"));
	assert(is_1byte_word("a"));
	assert(is_1byte_word("z"));
	assert(!is_1byte_word("{"));
	assert(!is_1byte_word("~"));
	return EXIT_SUCCESS;
}



#if 0

/*const char targets[] = { '\0', '/', '0', '9', ':', '@', 'A' };
for(const char *t = targets, *const t_end = t + sizeof targets / sizeof *targets; t < t_end; t++) {
	size_t i, j;
	i = lower_bound(word_tree, sizeof word_tree / sizeof *word_tree, *t);
	j = upper_bound(word_tree, sizeof word_tree / sizeof *word_tree, *t);
	printf("codepoint: \"%c\", lower %zu:\"%c\", upper %zu:\"%c\".\n", *t, i, word_tree[i], j, word_tree[j]);
}*/

/* A Patricia-trie is not appropriate for range-queries because skip-bits. */

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

#endif
#if 0

static size_t lower_bound(const char *const nondec, size_t size, const char target) {
	size_t lo = 0, hi = size, count = hi - lo;
	while(count) {
		size_t step = count / 2;
		if(nondec[lo + step] < target) {
			lo += step + 1;
			count -= step + 1;
		} else {
			count = step;
		}
	}
	return lo;
}

#endif
