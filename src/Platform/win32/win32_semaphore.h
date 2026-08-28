#pragma once
#include "os/semaphore.h"
#include "Platform/platform_header.h"


static_assert(sizeof(Semaphore) >= sizeof(HANDLE));


