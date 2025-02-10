#ifndef DELIMIT
#	define DELIMIT
#	include <stdint.h>
struct delimit { union { const char *c; const uint8_t *u; } start, end; };
#endif
