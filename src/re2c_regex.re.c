// re2c $INPUT -o $OUTPUT -8 (utf-8) -i --case-ranges (gnu extension very compressed code file) [--encoding-policy substitute --input-encoding utf8]
#include <assert.h>
#include <stdint.h>

#include "delimited.h"

/*!include:re2c "unicode_categories.re" */

/** To begin, `w` end must be set to the string. */
void re2c_regex_next_delimited(struct delimited *const w) {
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
		binary_digit = [01];
		hex_digit = [0-9a-fA-F];
		octal_digit = [0-7];
		dec_digit = [0-9];
		all_other_numeric = N \ [0-9]; // Number.
		decimal = [.];
		sign = [-+];
		exp = [e];
		binary = "b" sign? binary_digit+ (Pc binary_digit+)*; // Punctuation connector.
		hex = "0x" sign? hex_digit+ (Pc hex_digit+)*;
		positive = dec_digit+ (Pc dec_digit+)*;
		integer = sign? positive;
		money = integer (decimal dec_digit{2,2})? Sc; // Symbol currency. Probably not inclusive
		real = integer decimal positive (exp integer (decimal positive)?)?;
		number = binary | hex | integer | real | money;
		letter = L M*; // Letter modifier.
		word_begin = letter | all_other_numeric;
		word_mid = word_begin | Pc | [\u200b\u200c\u200d\u2060]; // Connector punctuation. We specifically don't do "\'" because they are probably separate words. Similarly, garesh \u05F3.
		word_end = word_begin;
		word     = word_begin (word_mid* word_end)?;
		word_like = (word | number)+;

		"\x00" { w->start.u = 0; return; }
		@w0 word_like @w1 { w->start.u = w0; w->end.u = w1; return; }
		* { continue; } */
	}
}
