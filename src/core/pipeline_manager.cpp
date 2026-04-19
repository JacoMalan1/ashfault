#include <ashfault/core/pipeline_manager.h>
#include <ashfault/renderer/pipeline.h>

namespace ashfault {
void PipelineManager::add_graphics_pipeline(
    const std::string &name, std::shared_ptr<GraphicsPipeline> pipeline) {
  this->m_GraphicsPipelines[name] = pipeline;
}

GraphicsPipeline *PipelineManager::get_graphics_pipeline(
    const std::string &name) {
  return this->m_GraphicsPipelines[name].get();
}

void PipelineManager::destroy() {
  for (auto [name, pipeline] : m_GraphicsPipelines) {
    pipeline->destroy();
  }
  for (auto [name, pipeline] : m_ComputePipelines) {
    pipeline->destroy();
  }
}

void PipelineManager::add_compute_pipeline(
    const std::string &name, std::shared_ptr<ComputePipeline> pipeline) {
  m_ComputePipelines[name] = pipeline;
}

ComputePipeline *PipelineManager::get_compute_pipeline(
    const std::string &name) {
  return m_ComputePipelines[name].get();
}
}  // namespace ashfault
