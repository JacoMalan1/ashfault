#ifndef ASHFAULT_EDITOR_H
#define ASHFAULT_EDITOR_H

#include <ashfault/application.h>
#include <ashfault/ashfault.h>
#include <ashfault/core/camera.h>
#include <ashfault/core/layer_stack.h>
#include <ashfault/core/pipeline_manager.h>
#include <ashfault/core/scene.h>
#include <spdlog/details/log_msg.h>

#include <memory>

namespace ashfault::editor {
class ASHFAULT_API Editor : public Application {
 public:
  Editor(std::shared_ptr<Window> window);
  ~Editor();
  void run() override;
};
}  // namespace ashfault::editor

#endif
