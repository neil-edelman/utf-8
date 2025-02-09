#ifndef DEFINE

#	include <stdint.h>

/* <https://www.unicode.org/reports/tr44/#General_Category_Values>. */
#	define CATEGORIES X(WTF), X(Cc), X(Cf), X(Co), X(Cs), \
	X(Ll), X(Lm), X(Lo), X(Lt), X(Lu), X(Mc), X(Me), X(Mn), \
	X(Nd), X(Nl), X(No), X(Pc), X(Pd), X(Pe), X(Pf), X(Pi), X(Po), X(Ps), \
	X(Sc), X(Sk), X(Sm), X(So), X(Zl), X(Zp), X(Zs)
#	define X(n) n
enum character_category { CATEGORIES };
#	undef X
extern const char *category_strings[];

/** Note. Utf-8 disallows overlong encodings so it conflicts with '\0' as
 nul-termination. Should be "modified utf-8" which uses one overlong encoding
 of U+0000 as "0xC0, 0x80". Might be important in some instances? */
struct unicode {
	uint32_t unicode;
	enum character_category category;
	uint8_t utf8_size;
	char utf8[5];
	union internal { char byte[4]; uint32_t uint; } internal;
};

struct unicode_deque unicode_load(void);

#endif

#ifdef DELAY
#	undef DELAY
#else
#	ifdef DEFINE
#		undef DEFINE
#	else
#		define DEQUE_DECLARE_ONLY
#	endif
#	define DEQUE_NAME unicode
#	define DEQUE_TYPE struct unicode
#	define DEQUE_FRONT
#	define DEQUE_TO_STRING
#	define DEQUE_NON_STATIC
#	include "../src/deque.h"
#endif
