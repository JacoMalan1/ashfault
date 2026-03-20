#include <ashfault/core/pipeline_manager.h>
#include <ashfault/renderer/pipeline.h>

namespace ashfault {
void PipelineManager::add_graphics_pipeline(
    const std::string &name, std::shared_ptr<GraphicsPipeline> pipeline) {
  this->m_Pipelines[name] = pipeline;
}

GraphicsPipeline *PipelineManager::get_graphics_pipeline(
    const std::string &name) {
  return this->m_Pipelines[name].get();
}
}  // namespace ashfault
