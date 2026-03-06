#include "CLSTL/unique_ptr.h"
#include <ashfault/engine.h>

namespace ashfault {
Engine::Engine() : m_Renderer(clstl::make_unique<Renderer>()) {}

void Engine::run() {
}
}
