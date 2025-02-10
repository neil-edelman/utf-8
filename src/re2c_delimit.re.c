// re2c $INPUT -o $OUTPUT -8 --case-ranges [--encoding-policy substitute --input-encoding utf8] -i
#include "word.h"
#include <assert.h>
#include <stdint.h>

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
