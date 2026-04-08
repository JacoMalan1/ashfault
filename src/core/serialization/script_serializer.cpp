#include <ashfault/core/component/script.h>
#include <ashfault/core/script.h>
#include <ashfault/core/serialization/script_serializer.h>
#include "script_generated.h"

namespace ashfault::serialization {
flatbuffers::Offset<ScriptComponent> ScriptSerializer::serialize(
    flatbuffers::FlatBufferBuilder &builder,
    const ashfault::ScriptComponent &script) {
  auto asset = CreateAsset(builder, builder.CreateString(script.script.id()),
                           builder.CreateString(script.script.path()));
  return CreateScriptComponent(builder, asset);
}

ashfault::ScriptComponent ScriptSerializer::deserialize(
    const ScriptComponent *script, AssetManager *asset_manager) {
  auto asset = asset_manager->load<Script>(script->script()->id()->str(),
                                           script->script()->path()->str());
  return ashfault::ScriptComponent{.script = asset};
}
}  // namespace ashfault::serialization
