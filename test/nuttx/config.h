#include <stdbool.h>
#include <stdio.h>

#define CONFIG_BOARD_LOOPSPERMSEC 5000
#define DEBUGASSERT assert
#define DEBUGPANIC() assert(false)
#define FAR
#define OK 0