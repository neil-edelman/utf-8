/** @license 2025 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 [Modified-utf-8](https://en.wikipedia.org/wiki/UTF-8#Modified_UTF-8).

 @std C89 */

#include "mutf-8.h"
#include <ctype.h>

/*#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>*/

/** Counts the code-points of a null-terminated string. If it is a well-formed
 mutf-8 string, this is the code-point count. @order `Θ(|s|)` */
size_t mutf8_code_point_count(const char *s) {
	size_t count = 0;
	while(*s != '\0') count += ((*s++ & 192) != 128);
	return count;
}

/** Input the first letter of the string `s`, it will tell… */
int mutf8_is_word(const char *s, const char **next) {
	const char byte0 = *s;
	if(!byte0) return 0;
	/* One byte, `isalnum` is exactly what we're looking for. */
	if(!(byte0 & 0xf0)) return *next = s + 1, isalnum(byte0);
	/* Used <https://www.compart.com/en/unicode/category>, which uses Unicode
	 Standard Annex #44. I clicked on all the categories that might have a
	 non-word character. Then I clicked on the each of the blocks and made
	 arbitrary decisions for each code-point in the block about whether it's a
	 word or a separator. I asked myself—if the character was inserted into a
	 word at the middle, would it add to the word, or separate it? I don't
	 write many of the languages represented by unicode, so I kind of guessed.

	 - Blocks.
	 Control: [0x0–0x1f, 0x7f], [0x80..0x9f].
	 Format: [all]
	 - (Latin-1 Supplement: only 0x80–0xbf.)
	 - Latin-1 Symbols and Punctuation: 0xa0–0xbf.
	 - (Arabic: only 0x0600–0x0605, 0x061c, 0x06dd?)
	 - (Syriac: only 0x0700–0x070f.)
	 - (Arabic Extended-A: only 0x08e2?)
	 - (Mongolian: only 0x1801–0x180e?)
	 - General Punctuation: 0x2000–0x206F.
	 - Supplemental Punctuation: 0x2e00–0x2e7f.
	 - (Arabic Presentation Forms-B: only 0xfeff.)
	 - (Specials: only 0xfff9–0xfffb.)
	 - (Kaithi: only 0x110bb–0x110cd.)
	 - Egyptian Hieroglyph Format Controls: 0x13430–0x1343f.
	 - Shorthand Format Controls: 0x1bca0–0x1bcaf.
	 - (Musical Symbols: only 0x1d173–0x1d17a?)
	 - Tags (has been taken over by emojis): 0xe0000–0xe007f?

	 Connector Punctuation: [0x5f], [0x203f, 0x2040, 0x2054], [0xfe33, 0xfe34, 0xfe4d, 0xfe4e, 0xfe4f, 0xff3f]
	 - CJK Compatibility Forms: 0xfe30–0xff4f.

	 Dash Punctuation: [0x002d], [0x058a], [0x05be], [0x1400], [0x1806], [0x2010–0x2015], [0x2e17, 0x2e1a, 0x2e3a, 0x2e3b, 0x2e40], [0x301c, 0x3030], [0x30a0], [0xfe31, 0xfe32, 0xfe58, 0xfe63, 0xff0d, 0x10ead]
	 - (Armenian: only 0x0559–0x055f, 0x0589, 0x58a?)
	 - (Hebrew: only 0x05be, 0x05c0, 0x05c3, 0x05c6, 0x05f3, 0x05f4?)
	 - (Unified Canadian Aboriginal Syllabics: only 0x1400.)
	 - (Mongolian: already.)
	 - General Punctuation: already.
	 - Supplemental Punctuation: already.
	 - (CJK Symbols and Punctuation: only 0x3000–0x301f, 0x3030–0x3037.)
	 - (CJK Symbols and Punctuation: U+3000..U+303F, except U+3007, U+3030, and U+303D.)
	 - (Katakana: only 0x30a0.)
	 - CJK Compatibility Forms: already.
	 - Small Form Variants: 0xfe50–0xfe6f.
	 - (Halfwidth and Fullwidth Forms: only 0xff01–0xff0f, 0xff1a–0xff20, 0xff3b–0xff40, 0xff5b–0xff64, 0xffa0, 0xffe2–0xffe4, 0xffe8–0xffee
	 - (Yezidi: only 0x10ead.)

	 …(I'm here.)
	 Close Punctuation: [0x29, 0x5d, 0x7d], 0x0f3b, 0x0f3d, 0x169c, 0x2046, 0x207e, 0x208e, 0x2309, 0x230b, 0x232a, 0x2769, 0x276b, 0x276d, 0x276f, 0x2771, 0x2773, 0x2775, 0x27c6, 0x27e7, 0x27e9, 0x27eb, 0x27ed, 0x27ef, …
	 Initial Punctuation: 0xab, [2018, 201b, 201c, 201f, 2039], [20e2, 2e04, 2e09, 2e0c, 2e1c, 2e20]
	 Final Punctuation: 0x00bb, [0x2019, 0x201d, 0x203a], [0x2e03, 0x2e05, 2e0a, 2e0d, 2e1d, 2e21]

	 - Supplemental Arrows-B, U+2900–U+297F.
	 - Miscellaneous Mathematical Symbols-B, U+2980–U+29FF.

	 - And U+2E17, U+2E1A, U+2E3A, U+2E3B, U+2E40, U+301C, U+3030, U+30A0,
	   U+FE32, U+FE31, U+FE58, U+FE63, U+FF0D, U+058A, U+1400, U+05BE, U+1806,
	   U+10EAD
	 - Ideographic Symbols and Punctuation: U+16FE0..U+16FFF.
	 */
}
