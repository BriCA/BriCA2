#ifndef __BRICA2_ASSERT_HPP__
#define __BRICA2_ASSERT_HPP__

#include <exception>
#include <stdexcept>

namespace brica2 {

struct fail_fast : public std::logic_error {
  explicit fail_fast(char const* const message) : std::logic_error(message) {}
};

#define BRICA2_STRINGIFY_DETAIL(x) #x
#define BRICA2_STRINGIFY(x) BRICA2_STRINGIFY_DETAIL(x)

#if defined(__clang__) || defined(__GNUC__)
#define BRICA2_LIKELY(x) __builtin_expect(!!(x), 1)
#define BRICA2_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define BRICA2_LIKELY(x) (!!(x))
#define BRICA2_UNLIKELY(x) (!!(x))
#endif

namespace detail {

[[noreturn]] inline void terminate() noexcept { std::terminate(); }

template <class Exception>
[[noreturn]] void throw_exception(Exception&& exception) {
  throw std::forward<Exception>(exception);
}

}  // namespace detail
}  // namespace brica2

#define BRICA2_CONTRACT_CHECK(type, cond)                                   \
  (BRICA2_LIKELY(cond) ? static_cast<void>(0)                               \
                       : brica2::detail::throw_exception(brica2::fail_fast( \
                             "BRICA2: " type "failure at " __FILE__         \
                             ": " BRICA2_STRINGIFY(__LINE__))))

#define Expects(cond) BRICA2_CONTRACT_CHECK("Precondition", cond)
#define Ensures(cond) BRICA2_CONTRACT_CHECK("Postcondition", cond)

#endif  // __BRICA2_ASSERT_HPP__
