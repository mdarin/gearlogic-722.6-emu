#ifndef POLLTRANS_H
#define POLLTRANS_H

#include "params.h"
#include "sens.h"
#include "mock_aruduino.h"
#include "pollstick.h"

// Polling time for transmission control
// R/N/P modulation pressure regulation
// idle SPC regulation
// Boost control
void polltrans(void *);

#endif // POLLTRANS_H
