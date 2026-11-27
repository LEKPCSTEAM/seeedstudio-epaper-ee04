#pragma once

#define MAJOR_VERSION 0
#define MINOR_VERSION 1
#define PATCH_VERSION 0
// #define PRE_RELEASE_VERSION ""

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#ifdef PRE_RELEASE_VERSION
#define FIRMWARE_VERSION                                                       \
  TOSTRING(MAJOR_VERSION)                                                      \
  "." TOSTRING(MINOR_VERSION) "." TOSTRING(                                    \
      PATCH_VERSION) "-" PRE_RELEASE_VERSION
#else
#define FIRMWARE_VERSION                                                       \
  TOSTRING(MAJOR_VERSION)                                                      \
  "." TOSTRING(MINOR_VERSION) "." TOSTRING(PATCH_VERSION)
#endif
