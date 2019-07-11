#ifndef __BRICA2_PORT_HPP__
#define __BRICA2_PORT_HPP__

#include "brica2/buffer.hpp"

namespace brica2 {

class port {
 public:
  template <class T, class S = std::initializer_list<ssize_t>>
  port(S&& s, const T& type_hint = T())
      : self(std::make_shared<impl>(std::forward<S>(s), type_hint)) {}

  port() = delete;
  port(const port&) = default;
  port(port&&) = default;
  port& operator=(const port&) = default;
  port& operator=(port&&) = default;

  buffer& get() { return self->content; }
  void set(const buffer& b) { self->content = b; }
  void set(buffer&& b) { self->content = std::move(b); }

  friend bool operator==(const port&, const port&);
  friend bool operator!=(const port&, const port&);

 private:
  struct impl {
    template <class T, class S = std::initializer_list<ssize_t>>
    impl(S&& s, const T& type_hint)
        : content(fill(std::forward<S>(s), type_hint)) {}
    buffer content;
  };
  std::shared_ptr<impl> self;
};

inline bool operator==(const port& lhs, const port& rhs) {
  return lhs.self == rhs.self;
}
inline bool operator!=(const port& lhs, const port& rhs) {
  return !(lhs == rhs);
}

}  // namespace brica2

#endif  // __BRICA2_PORT_HPP__
