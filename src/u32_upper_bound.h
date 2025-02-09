/* @std c99 */

#include <stddef.h>
#include <stdint.h>

/** @return The majorant of `key` on the set `table` between (`low`, `high`].
 @order \O(log `high` - `low`) */
static size_t u32_upper_bound(
	const uint32_t *const table,
	size_t low, size_t high,
	const uint32_t key) {
	while(low < high) {
		size_t mid = low + (high - low) / 2;
		if(table[mid] <= key) low = mid + 1;
		else high = mid;
	}
	return low;
}
