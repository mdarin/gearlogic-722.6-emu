#ifndef POLLSTICK_H
#define POLLSTICK_H

#include "params.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "core.h"

// -------------------------
// Polling for stick control
// This is W202 electronic gear stick, should work on any pre-canbus sticks.
void pollstick(void *);
// For manual microswitch control, gear up
void gearUp();
// For manual microswitch control, gear down
void gearDown();
// Polling for manual switch keys
void pollkeys();

#endif // POLLSTICK_H
