#include "tbrekalo/log.hpp"

auto main(int, char**) -> int {
  namespace tb = tbrekalo;
  tb::log::trace(tb::log::TTY_SINK)("{}", 42);
  tb::log::debug(tb::log::TTY_SINK)("{}", 1337);
  tb::log::info(tb::log::TTY_SINK)("{}", "HelloWorld");

  return EXIT_SUCCESS;
}
