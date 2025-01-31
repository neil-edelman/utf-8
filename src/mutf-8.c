/** @license 2025 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 [Modified-utf-8](https://en.wikipedia.org/wiki/UTF-8#Modified_UTF-8).

 @std C89 */

#include "mutf-8.h"
#include <ctype.h>

/** Counts the code-points of a null-terminated string. If it is a well-formed
 mutf-8 string, this is the code-point count. @order `Θ(|s|)` */
size_t mutf8_code_point_count(const char *s) {
	size_t count = 0;
	while(*s != '\0') count += ((*s++ & 192) != 128);
	return count;
}
