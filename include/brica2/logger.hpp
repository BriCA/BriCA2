#ifndef __BRICA2_LOGGER_HPP__
#define __BRICA2_LOGGER_HPP__

#include <iostream>
#include <string>
#include <exception>
#include <memory>

#ifndef BRICA2_LOGGER_EXTERN
#define BRICA2_LOGGER_EXTERN 0
#endif  // BRICA2_LOGGER_EXTERN

#if BRICA2_LOGGER_EXTERN
#define BRICA2_LOGGER_DECL extern
#define BRICA2_LOGGER_DEFINE                               \
  std::mutex brica2::logger::detail::mutex;                \
  std::unique_ptr<brica2::logger::detail::ostream_wrapper> \
      brica2::logger::detail::wrapper;
#else
#define BRICA2_LOGGER_DECL static
#endif  // BRICA2_LOGGER_EXTERN

namespace brica2 {

struct log_tags {
  std::string info = "[INFO ]";
  std::string warn = "[WARN ]";
  std::string error = "[ERROR]";
};

namespace logger {
namespace detail {

BRICA2_LOGGER_DECL std::mutex mutex;

struct ostream_wrapper {
  std::ostream& ostream;
  explicit ostream_wrapper(std::ostream& init) : ostream(init) {}
};

BRICA2_LOGGER_DECL std::unique_ptr<ostream_wrapper> wrapper;

template <class Head> inline void log_impl(Head&& head) {
  wrapper->ostream << head << std::endl;
}

template <class Head, class... Tail>
inline void log_impl(Head&& head, Tail&&... tail) {
  wrapper->ostream << head << " ";
  log_impl(std::forward<Tail>(tail)...);
}

}  // namespace detail

inline void enable(std::ostream& ostream) {
  std::lock_guard<std::mutex> lock{detail::mutex};
  detail::wrapper.reset(new detail::ostream_wrapper(ostream));
}

inline bool enabled() { return static_cast<bool>(detail::wrapper); }

inline void disable() {
  std::lock_guard<std::mutex> lock{detail::mutex};
  detail::wrapper.release();
}

template <class... Args> inline void log(Args&&... args) {
  if (!enabled()) return;
  std::lock_guard<std::mutex> lock{detail::mutex};
  detail::log_impl(std::forward<Args>(args)...);
}

template <class... Args> inline void info(Args&&... args) {
  log("[INFO ]", std::forward<Args>(args)...);
}

template <class... Args> inline void warn(Args&&... args) {
  log("[WARN ]", std::forward<Args>(args)...);
}

inline void error(std::exception&& e) {
  log("[ERROR]", e.what());
  throw e;
}

}  // namespace logger
}  // namespace brica2

#endif  // __BRICA2_LOGGER_HPP__
