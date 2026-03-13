#ifndef ASHFAULT_H
#define ASHFAULT_H

#ifdef ASHFAULT_PLATFORM_WINDOWS
  #ifdef ASHFAULT_BUILD_DLL
    #define ASHFAULT_API __declspec(dllexport)
  #else
    #define ASFHAULT_API __declspec(dllimport)
  #endif
#else
  #define ASHFAULT_API
#endif

namespace ashfault {}

#endif
