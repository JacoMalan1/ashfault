#ifndef ASHFAULT_MATERIAL_H
#define ASHFAULT_MATERIAL_H

namespace ashfault {
struct Material {
  int albedo_texture_index;
  int normal_texture_index;
};
}  // namespace ashfault

#endif
