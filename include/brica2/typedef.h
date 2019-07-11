#ifndef __BRICA2_TYPEDEF_H__
#define __BRICA2_TYPEDEF_H__

#include <cstddef>
#include <type_traits>

namespace brica2 {

using ssize_t = typename std::make_signed<size_t>::type;

}

#endif  // __BRICA2_TYPEDEF_H__
