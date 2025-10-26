#ifndef TBREKALO_LOG_HPP_
#define TBREKALO_LOG_HPP_

#include <chrono>
#include <filesystem>
#include <print>
#include <source_location>
#include <string_view>
#include <thread>

namespace tbrekalo::log::detail {

template <size_t N>
struct FixedString {
  char data[N + 1];
  consteval FixedString() noexcept : data{'\0'} {}
  consteval FixedString(char const (&src)[N + 1]) noexcept {
    __builtin_memcpy(data, src, N + 1);
  }

  static constexpr auto size() -> size_t {
    return N;
  }
};

template <size_t N>
FixedString(char const (&data)[N]) -> FixedString<N - 1>;

template <size_t Lhs, size_t Rhs>
consteval auto operator+(FixedString<Lhs> lhs, FixedString<Rhs> rhs) -> FixedString<Lhs + Rhs> {
  FixedString<Lhs + Rhs> dst;
  __builtin_memcpy(dst.data, lhs.data, Lhs);
  __builtin_memcpy(dst.data + Lhs, rhs.data, Rhs + 1);
  return dst;
}

} // namespace tbrekalo::log::detail

namespace tbrekalo::log {

enum class Level { TRACE, DEBUG, INFO, WARN, ERROR, FATAL };

inline constexpr auto FmtLevelTty(Level level) noexcept -> std::string_view {
  static constexpr std::array REPRS{
      "\033[37mTRACE\033[0m\t",  // White
      "\033[36mDEBUG\033[0m\t",  // Cyan
      "\033[32mINFO\033[0m\t",   // Green
      "\033[33mWARN\033[0m\t",   // Yellow
      "\033[31mERROR\033[0m\t",  // Red
      "\033[1;31mFATAL\033[0m\t" // Bold Red
  };

  return REPRS[std::to_underlying(level)];
}

inline constexpr auto FmtLevelASCII(Level level) noexcept -> std::string_view {
  static constexpr std::array REPRS{"TRACE\t", "DEBUG\t", "INFO\t", "WARN\t", "ERROR\t", "FATAL\t"};
  return REPRS[std::to_underlying(level)];
}

class FileLogger {
  static constexpr auto NO_OPT_DELETER = +[](std::FILE*) {};
  static constexpr auto FCLOSE_DELETER = +[](std::FILE* file) { std::fclose(file); };

  std::unique_ptr<std::FILE, void (*)(std::FILE*)> file_;

  Level level_;
  auto (*fmt_level_)(Level level) noexcept -> std::string_view;

public:
  using LevelFmtFn = auto (*)(Level) noexcept -> std::string_view;

  FileLogger(std::FILE* file, Level level, LevelFmtFn fmt_level)
      : file_(file, NO_OPT_DELETER), level_(level), fmt_level_(fmt_level) {}
  FileLogger(std::filesystem::path const& file, Level level, LevelFmtFn fmt_level) noexcept
      : file_(std::fopen(file.c_str(), "a"), FCLOSE_DELETER), level_(level), fmt_level_(fmt_level) {}

  auto level() const noexcept -> Level {
    return level_;
  }

  auto level(Level level) noexcept -> void {
    level_ = level;
  }

  template <detail::FixedString Fmt, class... Args>
  [[gnu::always_inline]] auto log(auto time_point, std::thread::id thread_id, Level level, std::string_view file_name,
                                  int line, Args&&... args) const -> void {
    static constexpr auto fmt = detail::FixedString("{:%FT%TZ} {} {} {}:{} ") + Fmt + detail::FixedString("\n");
    if (level >= level_) [[likely]] {
      std::print(file_.get(), fmt.data, time_point, thread_id, fmt_level_(level), file_name, line,
                 std::forward<Args>(args)...);
    }
  }
};

template <detail::FixedString Fmt, Level Level = Level::INFO>
inline constexpr auto log =
    [] [[gnu::always_inline]] (auto& sink, std::source_location loc = std::source_location::current()) {
      return [&sink, loc]<class... Args> [[gnu::always_inline]] (Args&&... args) {
        sink.template log<Fmt, Args...>(std::chrono::system_clock::now(), std::this_thread::get_id(), Level,
                                        loc.file_name(), loc.line(), std::forward<Args>(args)...);
      };
    };

/* clang-format off */
template <detail::FixedString Fmt> inline constexpr auto trace = log<Fmt, Level::TRACE>;
template <detail::FixedString Fmt> inline constexpr auto debug = log<Fmt, Level::DEBUG>;
template <detail::FixedString Fmt> inline constexpr auto info  = log<Fmt, Level::INFO >;
template <detail::FixedString Fmt> inline constexpr auto warn  = log<Fmt, Level::WARN >;
template <detail::FixedString Fmt> inline constexpr auto error = log<Fmt, Level::ERROR>;
template <detail::FixedString Fmt> inline constexpr auto fatal = log<Fmt, Level::FATAL>;
/* clang-format on */

} // namespace tbrekalo::log

#endif /* TBREKALO_LOG_HPP_ */
