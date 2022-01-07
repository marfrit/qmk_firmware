#pragma once

#include "quantum.h"
#include "ps2.h"

#define XXX KC_NO

/*
                   F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24.
                   F1,  F2,  F3,  F4,  F5,  F6,  F7,  F8,  F9,  F10, F11, F12.

RST, ESC,     GRV, 1,   2,   3,   4,   5,   6,   7,   8,   9,   0,   -,     =, STB,BSP,    INS, HOM, PUP,   NLK, SLS, AST, MNS,
SLK, MPL,     TAB,  Q,   W,   E,   R,   T,   Y,   U,   I,   O,   P,   [,   ],      STB     DEL, END, PDN,   7,   8,   9,   PLS,
BRK, INT5,    CLK,   A,   S,   D,   F,   G,   H,   J,   K    L,   ;,   ',   \,     ENT,         UP,         4,   5,   6,   COM,
APP, PSC,     LSF, <,   Z,   X,   C,   V,   B,   N,   M,   ,,   .,   /,   STB      RSF,    LFT, SHT, RGT,   1,   2,   3,   ENT,
RGU, LGU,     LCL,      LAT,                     SPC,           RAT,               RCL,         DN,         STB, 0,   STB, DOT

*/
#define LAYOUT(\
                   K01, K02, K03, K04, K05, K06, K07, K08, K09, K0A, K0B, K0C, \
                   K0D, K0E, K0F, K10, K11, K12, K13, K14, K15, K16, K17, K18, \
\
K19, K1A,   K1B, K1C, K1D, K1E, K1F, K20, K21, K22, K23, K24, K25, K26, K27, K28, K29,   K2A, K2B, K2C,   K2D, K2E, K2F, K30, \
K31, K32,   K33,  K34, K35, K36, K37, K38, K39, K3A, K3B, K3C, K3D, K3E, K3F,     K40,   K41, K42, K43,   K44, K45, K46, K47, \
K48, K49,   K4A,   K4B, K4C, K4D, K4E, K4F, K50, K51, K52, K53, K54, K55, K56,    K57,        K58,        K59, K5A, K5B, K5C, \
K5D, K5E,   K5F, K60, K61, K62, K63, K64, K65, K66, K67, K68, K69, K6A,  K6B,     K6C,   K6D, K6E, K6F,   K70, K71, K72, K73, \
K74, K75,   K76,       K77,               K78,                        K79,        K7A,        K7B,        K7C, K7D, K7E, K7F  \
) { \
 { XXX, K01, K02, K03, K04, K05, K06, K07,  \
  K08, K09, K0A, K0B, K0C, K0D, K0E, K0F }, \
 { K10, K11, K12, K13, K14, K15, K16, K17,  \
  K18, K19, K1A, K1B, K1C, K1D, K1E, K1F }, \
 { K20, K21, K22, K23, K24, K25, K26, K27,  \
  K28, K29, K2A, K2B, K2C, K2D, K2E, K2F }, \
 { K30, K31, K32, K33, K34, K35, K36, K37,  \
  K38, K39, K3A, K3B, K3C, K3D, K3E, K3F }, \
 { K40, K41, K42, K43, K44, K45, K46, K47,  \
  K48, K49, K4A, K4B, K4C, K4D, K4E, K4F }, \
 { K50, K51, K52, K53, K54, K55, K56, K57,  \
  K58, K59, K5A, K5B, K5C, K5D, K5E, K5F }, \
 { K60, K61, K62, K63, K64, K65, K66, K67,  \
  K68, K69, K6A, K6B, K6C, K6D, K6E, K6F }, \
 { K70, K71, K72, K73, K74, K75, K76, K77,  \
  K78, K79, K7A, K7B, K7C, K7D, K7E, K7F } \
}

#define IBMPC_PROTOCOL_NO       0
#define IBMPC_PROTOCOL_AT       0x10
#define IBMPC_PROTOCOL_AT_Z150  0x11
#define IBMPC_PROTOCOL_XT       0x20
#define IBMPC_PROTOCOL_XT_IBM   0x21
#define IBMPC_PROTOCOL_XT_CLONE 0x22
#define IBMPC_PROTOCOL_XT_ERROR 0x23

// Error numbers
#define IBMPC_ERR_NONE        0
#define IBMPC_ERR_RECV        0x00
#define IBMPC_ERR_SEND        0x10
#define IBMPC_ERR_TIMEOUT     0x20
#define IBMPC_ERR_FULL        0x40
#define IBMPC_ERR_ILLEGAL     0x80
#define IBMPC_ERR_FF          0xF0

void ibmpc_host_init(void);
void ibmpc_host_enable(void);
void ibmpc_host_disable(void);
int16_t ibmpc_host_send(uint8_t data);
int16_t ibmpc_host_recv_response(void);
int16_t ibmpc_host_recv(void);
void ibmpc_host_isr_clear(void);
void ibmpc_host_set_led(uint8_t usb_led);

// static uint16_t read_keyboard_id(void);
int16_t read_wait(uint16_t wait_ms);

extern volatile uint16_t ibmpc_isr_debug;
extern volatile uint8_t ibmpc_protocol;
extern volatile uint8_t ibmpc_error;

/* reset for XT Type-1 keyboard: low pulse for 500ms */
#define PS2_RST_HIZ() do { \
    setPinInput(PS2_DATA_PIN);  \
    setPinInput(PS2_CLOCK_PIN);  \
} while (0)
#define PS2_RST_LO() do { \
    setPinOutput(XT_RST_PIN1);  \
    setPinOutput(XT_RST_PIN2);  \
} while (0)

#define ID_STR(id)  (id == 0xFFFE ? "_????" : \
                    (id == 0xFFFD ? "_Z150" : \
                    (id == 0x0000 ? "_AT84" : \
                     "")))

typedef enum { NONE, PC_XT, PC_AT, PC_TERMINAL, PC_MOUSE } keyboard_kind_t;
#define KEYBOARD_KIND_STR(kind) \
    (kind == PC_XT ? "XT" :   \
     kind == PC_AT ? "AT" :   \
     kind == PC_TERMINAL ? "TERMINAL" :   \
     kind == PC_MOUSE ? "MOUSE" :   \
     "NONE")

extern const uint8_t map_cs1[MATRIX_ROWS][MATRIX_COLS];
extern const uint8_t map_cs2[MATRIX_ROWS][MATRIX_COLS];
extern const uint8_t map_cs3[MATRIX_ROWS][MATRIX_COLS];
