#pragma once

#include <stdint.h>
#include <stddef.h>

namespace PLAMIOmini {
namespace Platform {

// This file defines internal helper/utility functions to support the 
// implementation of functions declared under `namespace Platform` 
// in the public header "PLAMIOmini.h".

void sleepMsec(uint32_t msec);
void sleepUsec(uint32_t usec);

} // namespace
} // namespace
