#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>

struct mutf8 {
	enum character_category { WTF,
		Cc, Cf, Co, Cs, Ll, Lm, Lo, Lt, Lu, Mc, Me, Mn, Nd, Nl, No,
		Pc, Pd, Pe, Pf, Pi, Po, Ps, Sc, Sk, Sm, So, Zl, Zp, Zs }
		category;
	bool word;
	char string[5];
};
static const char *mutf8_key(const struct mutf8 *const info)
	{ return (const char *)info->string; }
#define TRIE_NAME mutf8
#define TRIE_ENTRY struct mutf8
#define TRIE_TO_STRING
#include "../src/trie.h"

static void mutf8_to_string(const struct mutf8 *const info, char (*const a)[12])
	{ strcpy(*a, mutf8_key(info)); }
#define DEQUE_NAME mutf8
#define DEQUE_TYPE struct mutf8
#define DEQUE_TO_STRING
#include "../src/deque.h"

int main(void) {
	const char *error = 0;
	const char *const uni_fn = "UnicodeData.txt";
	FILE *uni_fp = 0;
	char read[256];
	struct mutf8_deque deque = mutf8_deque();
	struct mutf8_trie trie = mutf8_trie();
	struct mutf8 *mutf8;
	errno = 0;
	if(!(uni_fp = fopen(uni_fn, "r"))) { error = uni_fn; goto catch; }
	while(fgets(read, sizeof read, uni_fp)) {
		struct {
			unsigned unicode;
			unsigned char category[4];
		} input;
		if(sscanf(read, "%x;%*[^;];%3[^;]", &input.unicode, input.category) != 2) { error = uni_fn; goto catch; };
		if(!(mutf8 = mutf8_deque_new(&deque))) goto catch;
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
			goto catch;
		}
		mutf8->category = cc;
		mutf8->word = (cc == Ll || cc == Lu || cc == Lt || cc == Lo || cc == Nd);
		if(input.unicode < 0x80) {
			mutf8->string[0] = (char)input.unicode;
			mutf8->string[1] = '\0';
		} else if(input.unicode < 0x0800) {
			mutf8->string[0] = 0xc0 | (char) (input.unicode >>  6u);
			mutf8->string[1] = 0x80 | (char)( input.unicode         & 0x3f);
			mutf8->string[2] = '\0';
		} else if(input.unicode < 0x010000) {
			mutf8->string[0] = 0xe0 | (char)(input.unicode  >> 12u);
			mutf8->string[1] = 0x80 | (char)((input.unicode >>  6u) & 0x3f);
			mutf8->string[2] = 0x80 | (char)( input.unicode         & 0x3f);
			mutf8->string[3] = '\0';
		} else if(input.unicode < 0x110000) {
			mutf8->string[0] = 0xf0 | (char) (input.unicode >> 18u);
			mutf8->string[1] = 0x80 | (char)((input.unicode >> 12u) & 0x3f);
			mutf8->string[2] = 0x80 | (char)((input.unicode >>  6u) & 0x3f);
			mutf8->string[3] = 0x80 | (char)( input.unicode         & 0x3f);
			mutf8->string[4] = '\0';
		} else {
			fprintf(stderr, "Only supports 0x110_000 code points in unicode 16.0.\n");
			goto catch;
		}
		struct mutf8 *info_ptr;
		if(!mutf8_trie_add(&trie, (char *)mutf8->string, &info_ptr)) goto catch;
		*info_ptr = *mutf8;
	}
	if(ferror(uni_fp)) goto catch; /* May not produce a meaningful error. */
	mutf8_trie_graph_all(&trie, "graph.gv", 0);
	goto finally;
catch:
	perror(error);
finally:
	mutf8_trie_(&trie);
	mutf8_deque_(&deque);
	if(uni_fp) fclose(uni_fp);
	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
