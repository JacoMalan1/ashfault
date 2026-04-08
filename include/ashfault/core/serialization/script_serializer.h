#ifndef ASHFAULT_SCRIPT_SERIALIZER_H
#define ASHFAULT_SCRIPT_SERIALIZER_H

#include <ashfault/core/component/script.h>
#include <flatbuffers/flatbuffer_builder.h>
#include <ashfault/core/asset_manager.hpp>
#include "script_generated.h"

namespace ashfault::serialization {
class ScriptSerializer {
public:
  static flatbuffers::Offset<ScriptComponent> serialize(
      flatbuffers::FlatBufferBuilder &builder,
      const ashfault::ScriptComponent &script);
  static ashfault::ScriptComponent deserialize(const ScriptComponent *script,
                                               AssetManager *asset_manager);
};
}  // namespace ashfault::serialization

#endif
