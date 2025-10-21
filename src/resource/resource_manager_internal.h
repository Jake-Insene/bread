#pragma once
#include "debug/fail.h"
#include "debug/debug.h"


#define RMDebugInfo(...) Log::debug("[ResourceManager]: " __VA_ARGS__)
#define RMFatal(...) Fatal("[ResourceManager]: " __VA_ARGS__)