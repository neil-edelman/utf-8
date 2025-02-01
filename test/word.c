#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>

struct mutf8 {
	enum { Cc, Cf, Co, Cs, Ll, Lm, Lo, Lt, Lu, Mc, Me, Mn, Nd, Nl, No, Pc, Pd,
		Pe, Pf, Pi, Po, Ps, Sc, Sk, Sm, So, Zl, Zp, Zs } character_category;
	bool word;
	unsigned char string[5];
};
static const char *mutf8_key(const struct mutf8 *const info)
	{ return (const char *)info->string; }
#define TRIE_NAME mutf8
#define TRIE_ENTRY struct mutf8
#define TRIE_TO_STRING
#include "../src/trie.h"

static const char *mutf8_to_string(const struct mutf8 *const info, char (*const a)[12]);
#define DEQUE_NAME mutf8
#define DEQUE_TYPE struct mutf8
#define DEQUE_TO_STRING
#include "../src/deque.h"

int main(void) {
	const char *error = 0;
	const char *const uni_fn = "UnicodeData.txt";
	FILE *uni_fp = 0;
	char read[256];
	struct mutf8_deque chars = mutf8_deque();
	struct mutf8_trie trie = mutf8_trie();
	errno = 0;
	if(!(uni_fp = fopen(uni_fn, "r"))) { error = uni_fn; goto catch; }
	while(fgets(read, sizeof read, uni_fp)) {
		unsigned uni;
		unsigned char cat[4], *ch;
		if(sscanf(read, "%x;%*[^;];%3[^;]", &uni, cat) != 2)
			{ error = uni_fn; goto catch; };
		int included = 0;
		switch(cat[0]) {
		case 'L':
			switch(cat[1]) {
			case 'i': case 'u': case 't': case 'o': included = 1; break;
			default: break;
			}
		case 'N':
			if(cat[1] == 'd') included = 1; break;
		default: break;
		}
		//printf("%x, %s, %d\n", uni, cat, included);
		if(uni < 0x80) {
			if(!(ch = char_deque_append(&chars, 2))) goto catch;
			ch[0] = (unsigned char)uni;
			ch[1] = '\0';
			printf("char(%u): \"%s\"\n", uni, ch);
		} else if(uni < 0x0800) {
			if(!(ch = char_deque_append(&chars, 3))) goto catch;
			ch[0] = 0xc0 | (unsigned char)(uni >> 6u);
			ch[1] = 0x80 | (unsigned char)(uni & 0x3f);
			ch[2] = '\0';
			printf("char(%u): %u %u \"%s\"\n", uni, ch[0], ch[1], ch);
		} else if(uni < 0x010000) {
			//assert(0);
		} else if(uni < 0x110000) {
			//assert(0);
		} else {
			goto catch;
		}
		if(!utf8_trie_add(&trie, (char *)ch)) goto catch;
	}
	if(ferror(uni_fp)) goto catch; /* May not produce a meaningful error. */
	utf8_trie_graph_all(&trie, "graph.gv", 0);
	goto finally;
catch:
	perror(error);
finally:
	utf8_trie_(&trie);
	char_deque_(&chars);
	if(uni_fp) fclose(uni_fp);
	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
