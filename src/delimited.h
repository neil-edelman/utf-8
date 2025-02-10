#ifndef DELIMITED
#	define DELIMITED
#	include <stdint.h>
struct delimited { union { const char *c; const uint8_t *u; } start, end; };
#endif
