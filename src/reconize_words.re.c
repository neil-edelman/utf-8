// re2c $INPUT -o $OUTPUT -8 --case-ranges [--encoding-policy substitute --input-encoding utf8] -i
#include <assert.h>
#include <stdint.h>

struct word { union { const char *c; const uint8_t *u; } start, end; };

/*!include:re2c "unicode_categories.re" */

/** To begin, `w` end must be set to the string. */
static void next_word(struct subword *const w) {
	const uint8_t *YYCURSOR = w->end.u, *YYMARKER, *yyt1 = 0, *w0, *w1;
	for( ; ; ) {
		/*!re2c
		re2c:define:YYCTYPE = 'uint8_t';
		re2c:yyfill:enable = 0;
		re2c:tags = 1;

		// Simplified "Unicode Identifier and Pattern Syntax"
		// (see https://unicode.org/reports/tr31) (modified)
		// Letter and any marks that modify it.
		// <https://www.regular-expressions.info/unicode.html>
		letter = L M*;
		//word_end = letter | N;
		//word_mid = word_end | Pc | ['\u200b\u200c\u200d\u05F3];
		//word     = word_end (word_mid* word_end)?;
		//binary_digit = [01];
		//hex_digit = [0-9a-fA-F];
		//octal_digit = [0-7];
		//separator = [_]; // Python syntax.
		//decimal = [.];
		//binary = "b" binary_digit+ (separator binary_digit+)*;
		//hex = "0x" hex_digit+ (separator hex_digit+)*;
		//integer = N+ (separator N+)*;
		//money = integer (decimal N N)? Sc;
		//word_like = word | binary | hex | integer | money;
		word_like = letter+;

		"\x00" { w->start.u = 0; return; }
		@w0 word_like @w1 { w->start.u = w0; w->end.u = w1; return; }
		* { continue; } */
	}
}

#include <stdio.h>
#include <stdbool.h>

#define TEST(str, num) \
s = str; \
for(sub.end.c = s, count = 0; ; ) { \
	next_subword(&sub); \
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
	TEST(" yo", 1)
	TEST("one two", 2)
	TEST("one,two", 2)
	TEST("one.two", 2)
	TEST("one_two", 1)
	TEST("one—two", 2)
	TEST("0.0 a.b", 3)
	TEST("fi f‌iii", 2) /* U+200C zwnj is in the second */
	TEST("manœuver…æroplane non-joinerfif‌‌itt‌‌tt what?", 5) /* 4 zwnj */
	TEST("100,000_000$", 3)
	TEST("zzzz̀abc de f̀ghi", 3) /* z, U+0300…f, U+0300 combining grave accent */
	TEST("나는  Chicago에  산다", 3) /* actually 5 */
	TEST("Ыдентификатор", 1) /* From re2c. */

	return 0;
}
