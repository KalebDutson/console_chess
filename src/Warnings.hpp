#ifndef Warning_H
#define Warning_H

#include "Debug.hpp"

// Compiler message for debug mode
#if defined(DEBUG_MODE)
#if(DEBUG_MODE == true)
#pragma message("Debug mode is enabled")
#endif
#endif

#endif