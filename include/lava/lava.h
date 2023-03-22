#ifndef LAVA_H_
#define LAVA_H_

#include "config.h"

#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#define LAVA_ARRAYLEN(arr) (sizeof(arr) / sizeof(arr[0]))

#define LAVA_CONCAT_IMPL(a,b) a##b
#define LAVA_CONCAT2(a,b) LAVA_CONCAT_IMPL(a,b)

#ifdef _MSC_VER
# define LAVA_UNREACHABLE() __assume(0)
#elif defined(__has_builtin) && __has_builtin(__builtin_unreachable)
# define LAVA_UNREACHABLE() __builtin_unreachable()
#else
# define LAVA_UNREACHABLE() (void)0
#endif

#endif /* LAVA_H_ */
