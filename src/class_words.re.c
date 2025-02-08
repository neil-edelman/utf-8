// re2c $INPUT -o $OUTPUT -8 --case-ranges [--encoding-policy substitute --input-encoding utf8] -i
#include <assert.h>
#include <stdint.h>

struct word { union { const char *c; const uint8_t *u; } start, end; };

/*!include:re2c "unicode_categories.re" */

/** To begin, `w` end must be set to the string. */
static void next_word(struct word *const w) {
	const uint8_t *YYCURSOR = w->end.u, *YYMARKER, *yyt1 = 0, *w0, *w1;
	for( ; ; ) {
		/*!re2c
		re2c:define:YYCTYPE = 'uint8_t';
		re2c:yyfill:enable = 0;
		re2c:tags = 1;

		// Letters, marks, numbers, connector punctuation, joiners.
		letter = L | M | N | Pc | [\u200b\u200c\u200d\u2060];

		"\x00" { w->start.u = 0; return; }
		@w0 letter+ @w1 { w->start.u = w0; w->end.u = w1; return; }
		* { continue; } */
	}
}

#include <stdio.h>
#include <stdbool.h>

#define TEST(str, num) \
s = str; \
for(sub.end.c = s, count = 0; ; ) { \
	next_word(&sub); \
	if(!sub.start.c) break; \
	printf("\"%.*s\"\n", (int)(sub.end.c - sub.start.c), sub.start.c); \
	count++; \
} \
printf("\"%s\": %zu; supposed to be %zu.\n", s, count, (size_t)num); \
assert(count == num);

int main(void) {
	const char *s;
	struct word sub;
	size_t count;

	TEST("", 0)
	TEST(" ,...,,,,.,., ~~~`````", 0)
	TEST(" yo", 1)
	TEST("one two", 2)
	TEST("one,two", 2)
	TEST("one.two", 2)
	TEST("one_two", 1)
	TEST("one—two", 2)
	TEST("(1-1i)", 2)
	TEST("0.0 a.b", 4)
	TEST("fi f‌iii", 2) /* U+200C zwnj is in the second. */
	TEST("manœuver…æroplane non-joinerfif‌‌itt‌‌tt good?", 5) /* 4 zwnj. */
	TEST("100,000_000$", 2) /* Number "," number. */
	TEST("zzzz̀abc de f̀ghi", 3) /* z U+0300…f U+0300 combining grave accent. */
	/* <https://www.unicode.org/reports/tr29/>. Actually 5? */
	TEST("나는  Chicago에  산다", 3)
	TEST("_Ыдентификатор", 1) /* From re2c. "_" word. */
	TEST("\x80\x80""a a", 2) /* Proper handling of non-utf-8. */
	TEST("\xc0\x00""a a a a a a", 0)
	TEST("\xe0\x00""a a a a a a", 0)
	TEST("\xf0\x00""a a a a a a", 0)
	TEST("\xf8\x00""a a a a a a", 0)
	TEST("\xf8""a a a a a a", 6)

	return 0;
}
