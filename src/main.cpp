#include <ashfault/application.h>
#include <ashfault/core/window.h>

int main() {
  auto window = std::make_shared<ashfault::Window>(1280, 720, false);
  auto application = std::make_unique<ashfault::Application>(window);
  application->run();
}
