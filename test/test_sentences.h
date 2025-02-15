#include <stddef.h>

static const struct test {
	const char *sentence;
	size_t delimit_count_basic, delimit_count_regex;
} sentences[] = {
	{ "", 0, 0 },
	{ " yo", 1, 1 },
	{ " ,...,,,,.,., ~~~````` — ", 0, 0 },
	{ "They said, \"How gœs 'arr'mates' ' oh—no …\"", 8, 8 },
	{ "one two", 2, 2 },
	{ "one,two", 2, 2 },
	{ "one.two", 2, 2 },
	{ "one_two", 1, 1 }, /* '_' ∈ connector punctuation Pc. */
	{ "one—two", 2, 2 },
	{ "(1-1i)", 2, 1 }, /* Regex will identify numbers. */
	{ "0.0 a.b", 4, 3 },
	{ "fi f‌iii", 2, 2 }, /* U+200C zwnj is in the second. */
	{ "manœuver…æroplane non-joinerfif‌‌itt‌‌tt good?", 5, 5 }, /* 4 zwnj. */
	{ "100,000_000$", 2, 2 }, /* Rejection of non-Python syntax. */
	{ "zzzz̀abc de f̀ghi", 3, 3 }, /* z U+0300…f U+0300 combining grave accent. */
	/* <https://www.unicode.org/reports/tr29/>. */
	{ "나는  Chicago에  산다", 3, 3 },
	{ "1_Ыдентификатор", 1, 2 }, /* From re2c. "_" word. */
	{ "\x80\x80""a a", 2, 2 }, /* Proper handling of non-utf-8. */
	{ "\xc0\x00""a a a a a a", 0, 0 },
	{ "\xe0\x00""a a a a a a", 0, 0 },
	{ "\xf0\x00""a a a a a a", 0, 0 },
	{ "\xf8\x00""a a a a a a", 0, 0 },
	{ "\xf8""a a a a a a", 6, 6 },
	{ "我", 1, 1 },
	{ "我會極不力出調裡據程那變。投那來因？", 2, 2 },
	{ "2025 Neil Edelman, distributed under the terms of the\n"
		"[MIT License](https://opensource.org/licenses/MIT).\n", 16, 16 }
};
