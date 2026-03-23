#ifndef EDITOR_CONTEXT_H
#define EDITOR_CONTEXT_H

#include <ashfault/core/scene.h>
#include "ashfault/core/camera.h"

namespace ashfault {
struct ASHFAULT_API RuntimeContext {
  Scene *active_scene;
};

struct ASHFAULT_API EditorContext : public RuntimeContext {
  std::optional<Entity> selected_entity;
  PerspectiveCamera *perspective_camera;
};
}

#endif
