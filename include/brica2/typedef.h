#ifndef __BRICA2_TYPEDEF_H__
#define __BRICA2_TYPEDEF_H__

#include "brica2/macros.h"

#include <cstddef>
#include <type_traits>

NAMESPACE_BEGIN(BRICA2_NAMESPACE)

using ssize_t = typename std::make_signed<size_t>::type;

NAMESPACE_END(BRICA2_NAMESPACE)

#endif  // __BRICA2_TYPEDEF_H__
