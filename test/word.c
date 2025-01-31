#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>

struct private_char_contiguous {
	struct private_char_contiguous *last;
	unsigned char data[];
};

struct char_deque {
	size_t size, capacity;
	struct private_char_contiguous *first;
};

static struct char_deque char_deque(void)
	{ struct char_deque deque = { 0, 0, 0 }; return deque; }

static void char_deque_(struct char_deque *d) {
	if(!d) return;
	while(d->first) {
		struct private_char_contiguous *const c = d->first;
		d->first = d->first->last;
		free(c);
	}
	*d = char_deque();
}

static unsigned char *char_deque_append(struct char_deque *const d, const size_t n) {
	struct private_char_contiguous *c;
	size_t size;
	if(!n) return 0;
	if(d->capacity - d->size >= n) {
		unsigned char *found = d->first->data + d->size;
		d->size += n;
		return found;
	}
	for(size = d->capacity ? d->capacity * 2 : 32; size <= n; size *= 2);
	if(!(c = malloc(sizeof *c + sizeof *c->data * size)))
		{ if(!errno) errno = ERANGE; return 0; }
	d->capacity = size;
	d->size = n;
	c->last = d->first, d->first = c;
	printf("c is a new buffer of %zu chars, %zu taken up.", size, n);
	printf("char_deque %p->", (void *)d);
	for(struct private_char_contiguous *it = d->first; it; it = it->last) {
		printf("%p->", (void *)it);
	}
	printf("\n");
	return &c->data[0];
}

/*struct utf8_info {
	enum { Cc, Cf, Co, Cs, Ll, Lm, Lo, Lt, Lu, Mc, Me, Mn, Nd, Nl, No, Pc, Pd,
		Pe, Pf, Pi, Po, Ps, Sc, Sk, Sm, So, Zl, Zp, Zs } character_category;
	bool word;
	unsigned char string[5];
};
static const char *utf8_key(const struct utf8_info *const info) {
	return (const char *)info->string;
}
*/
#define TRIE_NAME utf8
//#define TRIE_ENTRY struct utf8_info
#define TRIE_TO_STRING
#include "../src/trie.h"

int main(void) {
	const char *error = 0;
	const char *const uni_fn = "UnicodeData.txt";
	FILE *uni_fp = 0;
	char read[256];
	struct char_deque chars = char_deque();
	struct utf8_trie trie = utf8_trie();
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
