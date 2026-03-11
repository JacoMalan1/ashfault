#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include <ashfault/core/window.h>
#include <ashfault/core/engine.h>
#include <ashfault/editor/editor.h>
#include <spdlog/spdlog.h>

int main() {
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] %^%l%$: {%s:%#} -> %v");
  spdlog::set_level(spdlog::level::trace);

  auto window = std::make_shared<ashfault::Window>(1280, 720, false);
  auto engine = std::make_shared<ashfault::Engine>();
  auto application = std::make_unique<ashfault::editor::Editor>(engine, window);
  application->run();
}
