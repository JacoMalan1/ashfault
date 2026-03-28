#ifndef ASHFAULT_COMPONENT_SCRIPT_H
#define ASHFAULT_COMPONENT_SCRIPT_H

#include <ashfault/ashfault.h>
#include <ashfault/core/script.h>
#include <ashfault/core/asset_manager.hpp>

namespace ashfault {
struct ASHFAULT_API ScriptComponent {
  Asset<Script> script;
};
}  // namespace ashfault

#endif
