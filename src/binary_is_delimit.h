#include "../build/delimit_edges.h"
#include "u32_upper_bound.h"

#include <stdbool.h>

/** Checks whether the first code-point in non-empty `string_in_utf8` is in the
 class of code-points that can be in words. This is very context-dependent in
 some situations, and some languages more than others. For example, a point
 could be a decimal (it is in a word) or a period (it is not.) Sets
 `output_next` to the next potential next code-point, if it isn't the end of
 the string. Bytes that don't appear in utf-8 are false; it sets `output` to
 the first non-continuation byte. <https://www.unicode.org/reports/tr36/>,
 (seek to a code-point should not be dependant on previous code-point's
 errors.) */
static bool binary_is_delimit(const char *const string_in_utf8,
		const char **const output_next) {
	const union {
		const char *input;
		const uint8_t *unchar;
		char *unconst;
	} string = { .input = string_in_utf8 };
	union { uint32_t u32; uint8_t u8[4]; } internal = { .u32 = 0 };
	size_t edge;

	uint8_t byte = string.unchar[0];
	if(byte < 0x80) { /* 1 byte? */
		internal.u8[0] = byte;
		edge = u32_upper_bound(utf8_delimit_edges, 0, utf8_delimit_end[0],
			internal.u32);
		*output_next = string.unconst + 1;
	} else if((byte & 0xe0) == 0xc0) { /* 2 bytes continued? */
		internal.u8[1] = byte;
		byte = string.unchar[1];
		if((byte & 0xc0) != 0x80)
			return *output_next = string.unconst + 1, false;
		internal.u8[0] = byte;
		edge = u32_upper_bound(utf8_delimit_edges, utf8_delimit_end[0], utf8_delimit_end[1], internal.u32);
		*output_next = string.unconst + 2;
	} else if((byte & 0xf0) == 0xe0) { /* 3 bytes continued? */
		internal.u8[2] = byte;
		byte = string.unchar[1];
		if((byte & 0xc0) != 0x80)
			return *output_next = string.unconst + 1, false;
		internal.u8[1] = byte;
		byte = string.unchar[2];
		if((byte & 0xc0) != 0x80)
			return *output_next = string.unconst + 2, false;
		internal.u8[0] = byte;
		edge = u32_upper_bound(utf8_delimit_edges, utf8_delimit_end[1], utf8_delimit_end[2], internal.u32);
		*output_next = string.unconst + 3;
	} else if((byte & 0xf8) == 0xf0) { /* 4 bytes continued? */
		internal.u8[3] = byte;
		byte = string.unchar[1];
		if((byte & 0xc0) != 0x80)
			return *output_next = string.unconst + 1, false;
		internal.u8[2] = byte;
		byte = string.unchar[2];
		if((byte & 0xc0) != 0x80)
			return *output_next = string.unconst + 2, false;
		internal.u8[1] = byte;
		byte = string.unchar[3];
		if((byte & 0xc0) != 0x80)
			return *output_next = string.unconst + 3, false;
		internal.u8[0] = byte;
		edge = u32_upper_bound(utf8_delimit_edges, utf8_delimit_end[2], utf8_delimit_end[3], internal.u32);
		*output_next = string.unconst + 4;
	} else {
		/* Not normalized utf-8 (16.0.0 #44) character. Maybe… */
		return *output_next = string.unconst + 1, false;
	}
	return edge & 1;
}
