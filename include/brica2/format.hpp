#ifndef __BRICA2_FORMAT_HPP__
#define __BRICA2_FORMAT_HPP__

#include "brica2/type_traits.hpp"

#include <string>

namespace brica2 {
namespace detail {

template <class T, class... Ts> struct format_t {
  char value[2];

  constexpr format_t(char const (&cs)[sizeof...(Ts) + 1])
      : value{cs[find_type<T, Ts...>()], '\0'} {}
};

}  // namespace detail

template <class T> struct FormatDescriptor {
  static auto format() -> decltype(auto) {
    // clang-format off
    using format_t = detail::format_t<
      T, char, signed char, unsigned char, bool, short, unsigned short, int,
      unsigned int, long, unsigned long, long long, unsigned long long, ssize_t,
      std::size_t, float, double
    >;
    // clang-format on
    return std::string(format_t("cbB?hHiIlLqQnNfd").value);
  }
};

}  // namespace brica2

#endif  // __BRICA2_FORMAT_HPP__
