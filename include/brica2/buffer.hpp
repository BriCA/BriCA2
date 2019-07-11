#ifndef __BRICA2_BUFFER_HPP__
#define __BRICA2_BUFFER_HPP__

#include "brica2/typedef.h"
#include "brica2/format.hpp"
#include "brica2/span.hpp"

#include <algorithm>
#include <functional>
#include <memory>
#include <numeric>
#include <vector>

#include <cstdlib>
#include <cstring>

namespace brica2 {
namespace detail {

enum class byte : unsigned char {};

template <class T, class InputIt>
auto default_strides(InputIt first, InputIt last) -> decltype(auto) {
  using value_type = typename std::iterator_traits<InputIt>::value_type;
  using multiplies = std::multiplies<value_type>;
  std::size_t diff = std::distance(first, last);
  std::vector<value_type> r(diff, sizeof(T));
  std::copy(first + 1, last, r.begin());
  std::partial_sum(r.rbegin(), r.rend(), r.rbegin(), multiplies());
  return r;
}

template <class InputIt>
auto product(InputIt first, InputIt last) -> decltype(auto) {
  using value_type = typename std::iterator_traits<InputIt>::value_type;
  using multiplies = std::multiplies<value_type>;
  return std::accumulate(first, last, 1, multiplies());
}

}  // namespace detail

class incompatible_exception : public std::exception {
 public:
  const char* what() const noexcept override {
    return "buffer infos are not compatible";
  }
};

struct buffer_info {
  ssize_t itemsize;
  std::string format;
  ssize_t ndim;
  std::vector<ssize_t> shape;
  std::vector<ssize_t> strides;
  void* ptr;

  buffer_info() = delete;
};

inline bool compatible(const buffer_info& lhs, const buffer_info& rhs) {
  return lhs.itemsize == rhs.itemsize && lhs.format == rhs.format &&
         lhs.ndim == rhs.ndim && lhs.shape == rhs.shape &&
         lhs.strides == rhs.strides;
}

inline bool operator==(const buffer_info& lhs, const buffer_info& rhs) {
  return compatible(lhs, rhs) && lhs.ptr == rhs.ptr;
}

inline bool operator!=(const buffer_info& lhs, const buffer_info& rhs) {
  return !(lhs == rhs);
}

class buffer {
 public:
  template <class T, class S>
  explicit buffer(S&& s, const T& type_hint = T())
      : buffer(s.begin(), s.end(), type_hint) {}

  template <class T, class InputIt>
  buffer(InputIt first, InputIt last, const T& type_hint = T())
      : info(std::shared_ptr<buffer_info>(new buffer_info{
            sizeof(T),
            FormatDescriptor<T>::format(),
            std::distance(first, last),
            {first, last},
            detail::default_strides<T>(first, last),
            std::malloc(sizeof(T) * detail::product(first, last))})) {}

 private:
  void reset(buffer_info* ptr) { info.reset(ptr); }

 public:
  friend buffer empty_like(const buffer&);

  virtual ~buffer() {}

  buffer_info& request() { return *info; }
  const buffer_info& request() const { return *info; }

  void* data() const { return info->ptr; }
  template <class T> T* data(const T& type_hint = T()) {
    return static_cast<T*>(info->ptr);
  }

  std::size_t size() const {
    return detail::product(info->shape.begin(), info->shape.end());
  }

  std::size_t size_bytes() const { return size() * info->itemsize; }

  template <class T> span<T> as_span() const {
    return span<T>(static_cast<T*>(info->ptr), size());
  }

  friend bool operator==(const buffer&, const buffer&);
  friend bool operator!=(const buffer&, const buffer&);

 private:
  std::shared_ptr<buffer_info> info;
};

inline bool operator==(const buffer& lhs, const buffer& rhs) {
  return lhs.info == rhs.info;
}

inline bool operator!=(const buffer& lhs, const buffer& rhs) {
  return !(lhs == rhs);
}

inline bool compatible(const buffer& lhs, const buffer& rhs) {
  return compatible(lhs.request(), rhs.request());
}

template <class T, class S = std::initializer_list<ssize_t>>
auto empty(S&& s, const T& type_hint = T()) -> decltype(auto) {
  return buffer(std::forward<S>(s), type_hint);
}

template <class T, class S = std::initializer_list<ssize_t>>
auto fill(S&& s, const T& value) -> decltype(auto) {
  auto ret = buffer(std::forward<S>(s), value);
  auto size = ret.size();
  auto ptr = static_cast<T*>(ret.request().ptr);
  std::fill(ptr, ptr + size, value);
  return ret;
}

template <class T, class S = std::initializer_list<ssize_t>>
auto with(std::initializer_list<T>&& ilist, S&& s) -> decltype(auto) {
  auto ret = buffer(std::forward<S>(s), T());
  auto size = ret.size();
  auto ptr = static_cast<T*>(ret.request().ptr);
  std::vector<T> v(ilist.begin(), ilist.end());
  std::memcpy(ptr, v.data(), size * sizeof(T));
  return ret;
}

inline buffer empty_like(const buffer& other) {
  auto ret(other);
  auto size = ret.size_bytes();
  auto& info = ret.request();
  auto ptr = new buffer_info{info.itemsize,
                             info.format,
                             info.ndim,
                             info.shape,
                             info.strides,
                             std::malloc(size)};
  ret.reset(ptr);
  return ret;
}

inline buffer zeros_like(const buffer& other) {
  auto ret = empty_like(other);
  auto size = ret.size_bytes();
  auto ptr = static_cast<byte*>(ret.request().ptr);
  std::fill(ptr, ptr + size, byte(0));
  return ret;
}

}  // namespace brica2

#endif  // __BRICA2_BUFFER_HPP__
