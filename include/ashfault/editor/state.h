#ifndef ASHFAULT_EDITOR_STATE_H
#define ASHFAULT_EDITOR_STATE_H

#include <ashfault/ashfault.h>
namespace ashfault {
class ASHFAULT_API State {
public:
  enum RuntimeState { Edit = 0, Play };
};
}  // namespace ashfault

#endif
