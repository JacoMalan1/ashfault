#ifndef ASHFAULT_PIPELINE_MANAGER_H
#define ASHFAULT_PIPELINE_MANAGER_H

#include <ashfault/ashfault.h>
#include <ashfault/renderer/pipeline.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace ashfault {
class ASHFAULT_API PipelineManager {
public:
  PipelineManager() = default;
  void destroy();

  void add_graphics_pipeline(const std::string &name,
                             std::shared_ptr<GraphicsPipeline> pipeline);
  GraphicsPipeline *get_graphics_pipeline(const std::string &name);

  void add_compute_pipeline(const std::string &name,
                            std::shared_ptr<ComputePipeline> pipeline);
  ComputePipeline *get_compute_pipeline(const std::string &name);

private:
  std::unordered_map<std::string, std::shared_ptr<GraphicsPipeline>>
      m_GraphicsPipelines;
  std::unordered_map<std::string, std::shared_ptr<ComputePipeline>>
      m_ComputePipelines;
};
}  // namespace ashfault

#endif
