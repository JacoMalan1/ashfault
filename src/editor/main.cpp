#include <ashfault/core/window.h>
#include <ashfault/editor/editor.h>
#include <spdlog/spdlog.h>

int main() {
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] %^%l%$: {%s:%#} -> %v");
  spdlog::set_level((spdlog::level::level_enum)SPDLOG_ACTIVE_LEVEL);

  auto window = std::make_shared<ashfault::Window>(1280, 720);
  auto application = std::make_unique<ashfault::editor::Editor>(window);
  application->run();
}
