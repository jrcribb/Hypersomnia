#pragma once
#include "augs/log.h"
#include "augs/build_settings.h"

void cleanup_proc();

#if ENABLE_ENSURE
#define ensure(x) if(!(x))\
{\
    LOG( "ensure(%x) failed\nfile: %x\nline: %x", #x, __FILE__, __LINE__ );\
	cleanup_proc(); \
}
#define ensure_eq(expected, actual) if(!(expected == actual))\
{\
    LOG( "ensure_eq(%x, %x) failed:\nexpected: %x\nactual: %x\nfile: %x\nline: %x", #expected, #actual, expected, actual, __FILE__, __LINE__ );\
	cleanup_proc(); \
}
#else
#define ensure(x) (x)
#define ensure_eq(x, y) (x, y)
#endif
