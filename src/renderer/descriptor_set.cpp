#include <ashfault/renderer/descriptor_set.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace ashfault {
VulkanDescriptorSetBuilder::VulkanDescriptorSetBuilder(VkDevice device)
    : m_Device(device) {}

VulkanDescriptorSetBuilder &VulkanDescriptorSetBuilder::add_binding(
    VkDescriptorType type, VkShaderStageFlags stage_flags,
    std::uint32_t descriptor_count, std::uint32_t binding) {
  VkDescriptorSetLayoutBinding el{};
  el.binding = binding;
  el.descriptorCount = descriptor_count;
  el.stageFlags = stage_flags;
  el.descriptorType = type;
  this->m_Bindings.push_back(el);
  return *this;
}

std::pair<std::vector<std::shared_ptr<VulkanDescriptorSet>>,
          std::shared_ptr<VulkanDescriptorPool>>
VulkanDescriptorSetBuilder::build() {
  std::vector<VkDescriptorSetLayout> layouts;
  layouts.reserve(this->m_Bindings.size());

  for (const auto &binding : this->m_Bindings) {
    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = 1;
    layout_info.pBindings = &binding;

    VkDescriptorSetLayout layout;
    if (vkCreateDescriptorSetLayout(this->m_Device, &layout_info, nullptr,
                                    &layout) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create descriptor set layout");
    }
    layouts.push_back(layout);
  }

  std::vector<std::uint32_t> counts;
  counts.reserve(this->m_Bindings.size());

  std::vector<VkDescriptorPoolSize> pool_sizes;
  pool_sizes.reserve(this->m_Bindings.size());
  for (const auto &binding : this->m_Bindings) {
    VkDescriptorPoolSize el{};
    el.descriptorCount = binding.descriptorCount;
    el.type = binding.descriptorType;
    pool_sizes.push_back(el);
    counts.push_back(binding.descriptorCount);
  }

  VkDescriptorPoolCreateInfo pool_info{};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.pPoolSizes = pool_sizes.data();
  pool_info.poolSizeCount = pool_sizes.size();
  pool_info.maxSets = 1;

  VkDescriptorPool pool;
  if (vkCreateDescriptorPool(this->m_Device, &pool_info, nullptr, &pool) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor pool");
  }

  VkDescriptorSetVariableDescriptorCountAllocateInfo count_info{};
  count_info.sType =
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
  count_info.descriptorSetCount = counts.size();
  count_info.pDescriptorCounts = counts.data();

  VkDescriptorSetAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = pool;
  alloc_info.pSetLayouts = layouts.data();
  alloc_info.descriptorSetCount = layouts.size();

  std::vector<VkDescriptorSet> descriptor_sets;
  descriptor_sets.resize(layouts.size());
  if (vkAllocateDescriptorSets(this->m_Device, &alloc_info,
                               descriptor_sets.data()) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate descriptor sets");
  }

  std::vector<std::shared_ptr<VulkanDescriptorSet>> dsets{};
  for (std::size_t i = 0; i < descriptor_sets.size(); i++) {
    dsets.push_back(std::make_shared<VulkanDescriptorSet>(
        this->m_Device, descriptor_sets[i], layouts[i]));
  }

  auto ret_pool =
      std::make_shared<VulkanDescriptorPool>(this->m_Device, pool);
  return std::make_pair(dsets, ret_pool);
}

VulkanDescriptorPool::VulkanDescriptorPool(VkDevice device,
                                           VkDescriptorPool pool)
    : m_Pool(pool), m_Device(device) {}
VulkanDescriptorPool::~VulkanDescriptorPool() {
  vkDestroyDescriptorPool(this->m_Device, this->m_Pool, nullptr);
}

VulkanDescriptorSet::VulkanDescriptorSet(VkDevice device,
                                         VkDescriptorSet descriptor_set,
                                         VkDescriptorSetLayout layout)
    : m_Device(device), m_DescriptorSet(descriptor_set), m_Layout(layout) {}

VulkanDescriptorSet::~VulkanDescriptorSet() {
  vkDeviceWaitIdle(this->m_Device);
  vkDestroyDescriptorSetLayout(this->m_Device, this->m_Layout, nullptr);
}

VkDescriptorSetLayout &VulkanDescriptorSet::layout() { return this->m_Layout; }

const VkDescriptorSetLayout &VulkanDescriptorSet::layout() const {
  return this->m_Layout;
}

const VkDescriptorSet &VulkanDescriptorSet::handle() const {
  return this->m_DescriptorSet;
}

VkDescriptorSet &VulkanDescriptorSet::handle() { return this->m_DescriptorSet; }
} // namespace ashfault
