#ifndef ASHFAULT_EDITOR_H
#define ASHFAULT_EDITOR_H

#include <CLSTL/shared_ptr.h>
#include <ashfault/application.h>
#include <ashfault/core/engine.h>

namespace ashfault {
class Editor : public Application {
public:
  Editor(clstl::shared_ptr<Engine> engine, clstl::shared_ptr<Window> window);
  void run() override;

private:
};
}

#endif
