// re2c $INPUT -o $OUTPUT -8 --case-ranges [--encoding-policy substitute --input-encoding utf8] -i
// re2c src/re2c_next_delimit.re.c -o build/re2c_next_delimit.c -8 --case-ranges --input-encoding utf8 -i
#include "../src/re2c_next_delimit.h"
#include <assert.h>
#include <stdint.h>

/*!include:re2c "unicode_categories.re" */

/** To begin, `delimit` end must be set to the string. */
void re2c_next_delimit(struct delimit *const delimit) {
	const uint8_t *YYCURSOR = delimit->end.u, *YYMARKER, *yyt1 = 0, *w0, *w1;
	for( ; ; ) {
		/*!re2c
		re2c:define:YYCTYPE = 'uint8_t';
		re2c:yyfill:enable = 0;
		re2c:tags = 1;

		// Letters, marks, numbers, connector punctuation, joiners.
		letter = L | M | N | Pc | [\u200b\u200c\u200d\u2060];

		"\x00" { delimit->start.u = 0; return; }
		@w0 letter+ @w1 { delimit->start.u = w0; delimit->end.u = w1; return; }
		* { continue; } */
	}
}
