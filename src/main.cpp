#include <CLSTL/shared_ptr.h>
#include <CLSTL/unique_ptr.h>
#include <ashfault/application.h>
#include <ashfault/core/window.h>
#include <ashfault/core/engine.h>

int main() {
  auto window = clstl::make_shared<ashfault::Window>(1280, 720, false);
  auto engine = clstl::make_shared<ashfault::Engine>();
  auto application = clstl::make_unique<ashfault::Application>(engine, window);
  application->run();
}
