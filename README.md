# log

Very simple file logger implementation.

## Why
I wanted to implement ergonomic logging in modern C++ without C-style macros. :) Intended for personal use in toy projects.

### Exapmle

```cpp
#include "tbrekalo/log.hpp"

auto main(int, char**) -> int {
  namespace tb = tbrekalo;

  auto run = [](auto& logger) -> void {
    tb::log::trace<"{}">(logger)(42);
    tb::log::debug<"{}">(logger)(42);
    tb::log::info<"{}">(logger)(42);
    tb::log::warn<"{}">(logger)(42);
    tb::log::error<"{}">(logger)(42);
    tb::log::fatal<"{}">(logger)(42);
  };

  tb::log::FileLogger tty(stdout, tb::log::Level::TRACE, tb::log::FmtLevelTty);
  tb::log::FileLogger file("/tmp/log.txt", tb::log::Level::TRACE, tb::log::FmtLevelASCII);

  run(tty);
  run(file);

  return 0;
}
```

### TODO:
- Callback based logger
- File roller
