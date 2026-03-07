#ifndef ASHFAULT_COMPONENT_MODEL_H
#define ASHFAULT_COMPONENT_MODEL_H

#include <CLSTL/shared_ptr.h>
#include <ashfault/renderer/buffer.hpp>

namespace ashfault {
template <class T, class I> struct MeshComponent {
  clstl::shared_ptr<ashfault::VulkanBuffer<T>> vertex_buffer;
  clstl::shared_ptr<ashfault::VulkanBuffer<I>> index_buffer;
};
} // namespace ashfault

#endif
