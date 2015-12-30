#include "softfloat.h"
#include "platform.h" // softfloat isNaNF32UI, etc.
#include "internals.h" // ditto
#include <cassert>

#include "../riscv/mulhi.hxx"
#include "decode_hwacha_ut.hxx"
#include "hwacha.hxx"
