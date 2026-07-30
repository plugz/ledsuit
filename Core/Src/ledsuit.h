#ifndef __LEDSUIT_H__
#define __LEDSUIT_H__

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

void ledsuit_init_beforeloop();
void ledsuit_tick();

#ifdef __cplusplus
} // extern "C"
#endif

#endif
