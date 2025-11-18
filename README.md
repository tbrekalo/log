# log

Minimalistic C++ logger implementation.

### Exapmle

```cpp
#include "tbrekalo/log.hpp"

auto main(int, char**) -> int {
  namespace tb = tbrekalo;
  auto sink = [file = std::unique_ptr<std::FILE, decltype([](std::FILE* f) -> void { std::fclose(f); })>(
                   std::fopen("/tmp/tb-log.txt", "a"))](tb::log::Record record) -> void {
    tb::log::TTY_SINK(record);
    std::print(file.get(), "{:%FT%T%z}\t{}\t{}\t{}:{}\t'{}'\n", record.ts, record.pid, std::to_underlying(record.level),
               record.loc.file_name(), record.loc.line(), record.msg);
    std::fflush(file.get());
  };

  tb::log::trace(sink)("{}", 42);
  tb::log::debug(sink)("{}", 1337);
  tb::log::info(sink)("{}", "HelloWorld");

  return EXIT_SUCCESS;
}

```
