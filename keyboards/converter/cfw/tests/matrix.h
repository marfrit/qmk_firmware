// Stub matrix.h for standalone CFW testing (no QMK dependency)
#pragma once
#include <stdint.h>
typedef uint16_t matrix_row_t;
#ifndef MATRIX_ROWS
#define MATRIX_ROWS 8
#endif
#ifndef MATRIX_COLS
#define MATRIX_COLS 16
#endif
