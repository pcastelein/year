//NOTE all in h file and not included for the time being
#include "core.h"

static s8   s8span(u8 *, u8 *);
static b32  s8equals(s8, s8);
static size s8compare(s8, s8);
static u64  s8hash(s8);
static s8   s8trim(s8);
static s8   s8clone(s8, arena *);