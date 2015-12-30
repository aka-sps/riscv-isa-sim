// See LICENSE for license details.

#include "softfloat.h"
#include "platform.h" // softfloat isNaNF32UI, etc.
#include "internals.h" // ditto
#include <cassert>

#include "mmu.hxx"
#include "mulhi.hxx"
#include "tracer.hxx"
