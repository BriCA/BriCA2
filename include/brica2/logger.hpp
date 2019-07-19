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
  void log(std::string tag, std::string msg) {
    ostream << tag << " " << msg << std::endl;
  }
};

BRICA2_LOGGER_DECL std::unique_ptr<ostream_wrapper> wrapper;

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

inline void log(std::string tag, std::string msg) {
  if (!enabled()) return;
  std::lock_guard<std::mutex> lock{detail::mutex};
  detail::wrapper->log(tag, msg);
}

inline void info(std::string msg) { log("[INFO ]", msg); }
inline void warn(std::string msg) { log("[WARN ]", msg); }
inline void error(std::exception&& e) {
  log("[ERROR]", e.what());
  throw e;
}

}  // namespace logger
}  // namespace brica2

#endif  // __BRICA2_LOGGER_HPP__
