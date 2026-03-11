#include <ashfault/application.h>
#include <ashfault/core/window.h>
#include <ashfault/core/engine.h>

int main() {
  auto window = std::make_shared<ashfault::Window>(1280, 720, false);
  auto engine = std::make_shared<ashfault::Engine>();
  auto application = std::make_unique<ashfault::Application>(engine, window);
  application->run();
}
