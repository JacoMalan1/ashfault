#ifndef ASHFAULT_PIPELINE_MANAGER_H
#define ASHFAULT_PIPELINE_MANAGER_H

#include <ashfault/renderer/pipeline.h>
#include <ashfault/ashfault.h>
#include <string>
#include <unordered_map>
#include <memory>

namespace ashfault {
class ASHFAULT_API PipelineManager {
public:
  PipelineManager() = default;

  void add_graphics_pipeline(const std::string &name,
                             std::shared_ptr<GraphicsPipeline> pipeline);
  GraphicsPipeline *get_graphics_pipeline(const std::string &name);

private:
  std::unordered_map<std::string, std::shared_ptr<GraphicsPipeline>>
      m_Pipelines;
};
} // namespace ashfault

#endif
