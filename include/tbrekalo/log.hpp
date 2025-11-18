#ifndef TBREKALO_LOG_HPP_
#define TBREKALO_LOG_HPP_

#include <unistd.h>

#include <array>
#include <chrono>
#include <cstdlib>
#include <print>
#include <source_location>
#include <thread>
#include <type_traits>
#include <utility>

namespace tbrekalo::log {

enum class Level : uint8_t { TRACE, DEBUG, INFO, WARN, ERROR, FATAL };

struct Record {
  std::chrono::system_clock::time_point ts;
  pid_t pid;
  std::thread::id thread;
  Level level;
  std::source_location loc;
  std::string msg;
};

template <Level level>
inline constexpr auto log =
    []<class SinkT> [[gnu::always_inline]] (SinkT&& sink, std::source_location loc = std::source_location::current())
  requires(std::is_lvalue_reference_v<SinkT> && std::is_invocable_r_v<void, SinkT, Record>)
{
  return [&sink = std::forward<decltype(sink)>(sink),
          loc]<class... Args> [[gnu::always_inline]] (std::format_string<Args...> fmt, Args&&... args) {
    sink(Record{
        .ts = std::chrono::system_clock::now(),
        .pid = getpid(),
        .thread = std::this_thread::get_id(),
        .level = level,
        .loc = loc,
        .msg = std::format(fmt, std::forward<Args>(args)...),
    });
  };
};

/* clang-format off */
inline constexpr auto trace = log<Level::TRACE>;
inline constexpr auto debug = log<Level::DEBUG>;
inline constexpr auto info  = log<Level::INFO>;
inline constexpr auto warn  = log<Level::WARN>;
inline constexpr auto error = log<Level::ERROR>;
inline constexpr auto fatal = log<Level::FATAL>;
/* clang-format on */

inline static Level ENV_LEVEL = [](auto val) static -> Level {
  return std::to_underlying(Level::TRACE) <= val && val <= std::to_underlying(Level::WARN) ? Level(val) : Level::INFO;
}([](auto str) { return str != nullptr ? std::atoi(str) : std::to_underlying(Level::INFO); }(::getenv("TB_LOG_LEVEL")));

inline constexpr auto TTY_SINK = [](Record record) static {
  static constexpr std::array REPRS{
      "\033[37mTRACE\033[0m\t",  // White
      "\033[36mDEBUG\033[0m\t",  // Cyan
      "\033[32mINFO\033[0m\t",   // Green
      "\033[33mWARN\033[0m\t",   // Yellow
      "\033[31mERROR\033[0m\t",  // Red
      "\033[1;31mFATAL\033[0m\t" // Bold Red
  };

#ifndef TB_LOG_LEVEL
#define TB_LOG_LEVEL ENV_LEVEL
#endif

  if (record.level >= TB_LOG_LEVEL) [[likely]] {
    std::print("{:%FT%T%z}\t{}\t{}\t{}\t{}:{}\t'{}'\n", record.ts, getpid(), record.thread,
               REPRS[std::to_underlying(record.level)], record.loc.file_name(), record.loc.line(), record.msg);
  }
};

} // namespace tbrekalo::log

#endif /* TBREKALO_LOG_HPP_ */
