#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include <CLSTL/shared_ptr.h>
#include <CLSTL/unique_ptr.h>
#include <ashfault/core/window.h>
#include <ashfault/core/engine.h>
#include <ashfault/editor/editor.h>
#include <spdlog/spdlog.h>

int main() {
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] %^%l%$: {%s:%#} -> %v");
  spdlog::set_level(spdlog::level::trace);

  auto window = clstl::make_shared<ashfault::Window>(1280, 720, false);
  auto engine = clstl::make_shared<ashfault::Engine>();
  auto application = clstl::make_unique<ashfault::Editor>(engine, window);
  application->run();
}
