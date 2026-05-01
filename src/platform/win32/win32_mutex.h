#pragma once
#include "os/mutex.h"
#include "platform/platform_header.h"


static_assert(sizeof(Mutex) >= sizeof(SRWLOCK));