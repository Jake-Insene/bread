#pragma once
#include "os/semaphore.h"
#include "platform/platform_header.h"


static_assert(sizeof(Semaphore) >= sizeof(HANDLE));


