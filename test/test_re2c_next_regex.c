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
	TEST("(1-1i)", 1) /* "(" Number number letter ")". */
	TEST("0.0 a.b", 3) /* Number " " word "." word. */
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
