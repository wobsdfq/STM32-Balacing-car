#include "stm32f10x.h" 
#include "Timer.h"

#define PERIODIC(T) \
static uint32_t nxt = 0; \
if(GetTick() < nxt) return; \
nxt += (T);

#define PERIODIC_START(NAME, T) \
static uint32_t NAME##_nxt = 0; \
if(GetTick() >= NAME##_nxt) {\
	NMAE##_nxt +=(T);
	
#define PERIODIC_END }
