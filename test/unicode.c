#define DELAY
#include "unicode.h"

#include <stdio.h>
#include <stdbool.h>

#define X(n) #n
const char *category_strings[] = { CATEGORIES };
#undef X

static void unicode_to_string(const struct unicode *const u, char (*const a)[12])
	{ sprintf(*a, "U+%x", u->unicode % 0x1000000); }

#define DEFINE
#include "unicode.h"

struct unicode_deque unicode_load(void) {
	const char *errmsg = 0;
	FILE *unicode_fp = 0;
	struct unicode_deque storage = unicode_deque();

	/* Detect the endianness. */
	const union { uint32_t i; uint8_t c[4]; } test = { .i = 1 };
	bool little_endian = test.c[0];

	/* Load the unicode data.
	 <https://www.unicode.org/Public/UNIDATA/UnicodeData.txt> */
	const char *const unicode_fn = "UnicodeData.txt";
	if(!(unicode_fp = fopen(unicode_fn, "r")))
		{ errmsg = unicode_fn; goto catch; }
	fprintf(stderr, "Opened \"%s\" for reading.\n", unicode_fn);
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
			fprintf(stderr, "Character %u unknown category %s.\n",
				input.unicode, input.category);
			errno = ERANGE;
			errmsg = unicode_fn;
			goto catch;
		}

		/* Load the entire table into memory because we are lazy. */
		struct unicode *u;
		if(!(u = unicode_deque_new_back(&storage))) goto catch;
		u->unicode = input.unicode;
		u->category = cc;
		u->utf8[0] = '\0', u->utf8[1] = '\0', u->utf8[2] = '\0',
			u->utf8[3] = '\0', u->utf8[4] = '\0';
		union internal v = { .uint = 0 };
		if(input.unicode < 0x80) {
			u->utf8_size = 1;
			v.byte[3] = u->utf8[0] = (char)input.unicode;
		} else if(input.unicode < 0x0800) {
			u->utf8_size = 2;
			v.byte[2] = u->utf8[0] = 0xc0 | (char) (input.unicode >>  6u);
			v.byte[3] = u->utf8[1] = 0x80 | (char)( input.unicode         & 0x3f);
		} else if(input.unicode < 0x010000) {
			u->utf8_size = 3;
			v.byte[1] = u->utf8[0] = 0xe0 | (char)(input.unicode  >> 12u);
			v.byte[2] = u->utf8[1] = 0x80 | (char)((input.unicode >>  6u) & 0x3f);
			v.byte[3] = u->utf8[2] = 0x80 | (char)( input.unicode         & 0x3f);
		} else if(input.unicode < 0x110000) {
			u->utf8_size = 4;
			v.byte[0] = u->utf8[0] = 0xf0 | (char) (input.unicode >> 18u);
			v.byte[1] = u->utf8[1] = 0x80 | (char)((input.unicode >> 12u) & 0x3f);
			v.byte[2] = u->utf8[2] = 0x80 | (char)((input.unicode >>  6u) & 0x3f);
			v.byte[3] = u->utf8[3] = 0x80 | (char)( input.unicode         & 0x3f);
		} else {
			fprintf(stderr, "Only supports 0x110_000 code points in unicode 16.0.0 #44.\n");
			errno = ERANGE;
			errmsg = unicode_fn;
			goto catch;
		}
		if(little_endian)
			u->internal.byte[0] = v.byte[3], u->internal.byte[1] = v.byte[2],
			u->internal.byte[2] = v.byte[1], u->internal.byte[3] = v.byte[0];
		else
			u->internal.uint = v.uint;
	}
	goto finally;
catch:
	perror(errmsg);
	unicode_deque_(&storage);
finally:
	if(unicode_fp) fclose(unicode_fp), unicode_fp = 0;
	return storage;
}
