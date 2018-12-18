/*
  64x16 Dot Matrix LED Display
  https://nurdspace.nl/64x16_Dot_Matrix_LED_Display
  https://datasheets.maximintegrated.com/en/ds/DS1302.pdf
*/

// test ok avec affichage heure
// Arduino 1.0.5 UNO R3
#if (defined(__AVR__))
#include <avr\pgmspace.h>
#else
#include <pgmspace.h>
#endif


// CONNECTIONS:
// DS1302 CLK/SCLK --> 5
// DS1302 DAT/IO --> 4
// DS1302 RST/CE --> 2
// DS1302 VCC --> 3.3v - 5v
// DS1302 GND --> GND

#include <ThreeWire.h>
#include <RtcDS1302.h>

ThreeWire myWire(A3, A4, A5); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

// Connections to board
const byte en_74138 = 2;
const byte la_74138 = 3;
const byte lb_74138 = 4;
const byte lc_74138 = 5;
const byte ld_74138 = 6;
const byte latchPin = 8;
const byte data_R1 = 10;
const byte data_R2 = 11;
const byte clockPin = 12;

byte ScanRow = 0;
unsigned long counter;

// Display buffer (which is scanned by the interrupt timer) of 8x16 bytes  (nox-custom: added an outline border)
byte buffer[128] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// ---------------------------------------------------------------------------------------------------
// FONTS sourced from various locations on the internet
// full ASCII character set (8x16) (1520 bytes)
const uint8_t font8x16_basic[] PROGMEM = {
  /*   (32) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* ! (33) */ 0x00, 0x00, 0x00, 0x18, 0x3C, 0x3C, 0x3C, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00,
  /* " (34) */ 0x00, 0x00, 0x66, 0x66, 0x66, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* # (35) */ 0x00, 0x00, 0x00, 0x00, 0x6C, 0x6C, 0xFE, 0x6C, 0x6C, 0x6C, 0xFE, 0x6C, 0x6C, 0x00, 0x00, 0x00,
  /* $ (36) */ 0x00, 0x18, 0x18, 0x7C, 0xC6, 0xC2, 0xC0, 0x7C, 0x06, 0x06, 0x86, 0xC6, 0x7C, 0x18, 0x18, 0x00,
  /* % (37) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0xC2, 0xC6, 0x0C, 0x18, 0x30, 0x60, 0xC6, 0x86, 0x00, 0x00, 0x00,
  /* & (38) */ 0x00, 0x00, 0x00, 0x38, 0x6C, 0x6C, 0x38, 0x76, 0xDC, 0xCC, 0xCC, 0xCC, 0x76, 0x00, 0x00, 0x00,
  /* ' (39) */ 0x00, 0x00, 0x30, 0x30, 0x30, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* ( (40) */ 0x00, 0x00, 0x00, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x18, 0x0C, 0x00, 0x00, 0x00,
  /* ) (41) */ 0x00, 0x00, 0x00, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x18, 0x30, 0x00, 0x00, 0x00,
  /* * (42) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* + (43) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* , (44) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x30, 0x00, 0x00,
  /* - (45) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* . (46) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00,
  /* / (47) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x00, 0x00, 0x00,
  /* 0 (48)    0x00, 0x00, 0x00, 0x3C, 0x66, 0xC3, 0xC3, 0xDB, 0xDB, 0xC3, 0xC3, 0x66, 0x3C, 0x00, 0x00, 0x00,*/
  /* 0 (48) */ 0x00, 0x00, 0x00, 0x3c, 0x66, 0xc3, 0xc7, 0xcb, 0xd3, 0xe3, 0xc3, 0x66, 0x3C, 0x00, 0x00, 0x00,
  /* 1 (49) */ 0x00, 0x00, 0x00, 0x18, 0x38, 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00, 0x00, 0x00,
  /* 2 (50) */ 0x00, 0x00, 0x00, 0x7C, 0xC6, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0xC6, 0xFE, 0x00, 0x00, 0x00,
  /* 3 (51) */ 0x00, 0x00, 0x00, 0x7C, 0xC6, 0x06, 0x06, 0x3C, 0x06, 0x06, 0x06, 0xC6, 0x7C, 0x00, 0x00, 0x00,
  /* 4 (52) */ 0x00, 0x00, 0x00, 0x0C, 0x1C, 0x3C, 0x6C, 0xCC, 0xFE, 0x0C, 0x0C, 0x0C, 0x1E, 0x00, 0x00, 0x00,
  /* 5 (53) */ 0x00, 0x00, 0x00, 0xFE, 0xC0, 0xC0, 0xC0, 0xFC, 0x06, 0x06, 0x06, 0xC6, 0x7C, 0x00, 0x00, 0x00,
  /* 6 (54) */ 0x00, 0x00, 0x00, 0x38, 0x60, 0xC0, 0xC0, 0xFC, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00,
  /* 7 (55) */ 0x00, 0x00, 0x00, 0xFE, 0xC6, 0x06, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00,
  /* 8 (56) */ 0x00, 0x00, 0x00, 0x7C, 0xC6, 0xC6, 0xC6, 0x7C, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00,
  /* 9 (57) */ 0x00, 0x00, 0x00, 0x7C, 0xC6, 0xC6, 0xC6, 0x7E, 0x06, 0x06, 0x06, 0x0C, 0x78, 0x00, 0x00, 0x00,
  /* : (58) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00,
  /* ; (59) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30, 0x00, 0x00, 0x00,
  /* < (60) */ 0x00, 0x00, 0x00, 0x00, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x00, 0x00, 0x00,
  /* = (61) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* > (62) */ 0x00, 0x00, 0x00, 0x00, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x00, 0x00, 0x00,
  /* ? (63) */ 0x00, 0x00, 0x00, 0x7C, 0xC6, 0xC6, 0x0C, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00,
  /* @ (64) */ 0x00, 0x00, 0x00, 0x00, 0x7C, 0xC6, 0xC6, 0xDE, 0xDE, 0xDE, 0xDC, 0xC0, 0x7C, 0x00, 0x00, 0x00,
  /* A (65) */ 0x00, 0x00, 0x00, 0x10, 0x38, 0x6C, 0xC6, 0xC6, 0xFE, 0xC6, 0xC6, 0xC6, 0xC6, 0x00, 0x00, 0x00,
  /* B (66) */ 0x00, 0x00, 0x00, 0xFC, 0x66, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x66, 0x66, 0xFC, 0x00, 0x00, 0x00,
  /* C (67) */ 0x00, 0x00, 0x00, 0x3C, 0x66, 0xC2, 0xC0, 0xC0, 0xC0, 0xC0, 0xC2, 0x66, 0x3C, 0x00, 0x00, 0x00,
  /* D (68) */ 0x00, 0x00, 0x00, 0xF8, 0x6C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x6C, 0xF8, 0x00, 0x00, 0x00,
  /* E (69) */ 0x00, 0x00, 0x00, 0xFE, 0x66, 0x62, 0x68, 0x78, 0x68, 0x60, 0x62, 0x66, 0xFE, 0x00, 0x00, 0x00,
  /* F (70) */ 0x00, 0x00, 0x00, 0xFE, 0x66, 0x62, 0x68, 0x78, 0x68, 0x60, 0x60, 0x60, 0xF0, 0x00, 0x00, 0x00,
  /* G (71) */ 0x00, 0x00, 0x00, 0x3C, 0x66, 0xC2, 0xC0, 0xC0, 0xDE, 0xC6, 0xC6, 0x66, 0x3A, 0x00, 0x00, 0x00,
  /* H (72) */ 0x00, 0x00, 0x00, 0xC6, 0xC6, 0xC6, 0xC6, 0xFE, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0x00, 0x00, 0x00,
  /* I (73) */ 0x00, 0x00, 0x00, 0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00,
  /* J (74) */ 0x00, 0x00, 0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0xCC, 0xCC, 0xCC, 0x78, 0x00, 0x00, 0x00,
  /* K (75) */ 0x00, 0x00, 0x00, 0xE6, 0x66, 0x66, 0x6C, 0x78, 0x78, 0x6C, 0x66, 0x66, 0xE6, 0x00, 0x00, 0x00,
  /* L (76) */ 0x00, 0x00, 0x00, 0xF0, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x62, 0x66, 0xFE, 0x00, 0x00, 0x00,
  /* M (77) */ 0x00, 0x00, 0x00, 0xC3, 0xE7, 0xFF, 0xFF, 0xDB, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0x00, 0x00, 0x00,
  /* N (78) */ 0x00, 0x00, 0x00, 0xC6, 0xE6, 0xF6, 0xFE, 0xDE, 0xCE, 0xC6, 0xC6, 0xC6, 0xC6, 0x00, 0x00, 0x00,
  /* O (79) */ 0x00, 0x00, 0x00, 0x7C, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00,
  /* P (80) */ 0x00, 0x00, 0x00, 0xFC, 0x66, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x60, 0xF0, 0x00, 0x00, 0x00,
  /* Q (81) */ 0x00, 0x00, 0x00, 0x7C, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0xD6, 0xDE, 0x7C, 0x0C, 0x0E, 0x00,
  /* R (82) */ 0x00, 0x00, 0x00, 0xFC, 0x66, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0x66, 0x66, 0xE6, 0x00, 0x00, 0x00,
  /* S (83) */ 0x00, 0x00, 0x00, 0x7C, 0xC6, 0xC6, 0x60, 0x38, 0x0C, 0x06, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00,
  /* T (84) */ 0x00, 0x00, 0x00, 0xFF, 0xDB, 0x99, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00,
  /* U (85) */ 0x00, 0x00, 0x00, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00,
  /* V (86) */ 0x00, 0x00, 0x00, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0x66, 0x3C, 0x18, 0x00, 0x00, 0x00,
  /* W (87) */ 0x00, 0x00, 0x00, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xDB, 0xDB, 0xFF, 0x66, 0x66, 0x00, 0x00, 0x00,
  /* X (88) */ 0x00, 0x00, 0x00, 0xC3, 0xC3, 0x66, 0x3C, 0x18, 0x18, 0x3C, 0x66, 0xC3, 0xC3, 0x00, 0x00, 0x00,
  /* Y (89) */ 0x00, 0x00, 0x00, 0xC3, 0xC3, 0xC3, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00,
  /* Z (90) */ 0x00, 0x00, 0x00, 0xFF, 0xC3, 0x86, 0x0C, 0x18, 0x30, 0x60, 0xC1, 0xC3, 0xFF, 0x00, 0x00, 0x00,
  /* [ (91) */ 0x00, 0x00, 0x00, 0x3C, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3C, 0x00, 0x00, 0x00,
  /* \ (92) */ 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0x70, 0x38, 0x1C, 0x0E, 0x06, 0x02, 0x00, 0x00, 0x00,
  /* ] (93) */ 0x00, 0x00, 0x00, 0x3C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x3C, 0x00, 0x00, 0x00,
  /* ^ (94) */ 0x00, 0x10, 0x38, 0x6C, 0xC6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* _ (95) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00,
  /* ` (96) */ 0x00, 0x30, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* a (97) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0xCC, 0xCC, 0x76, 0x00, 0x00, 0x00,
  /* b (98) */ 0x00, 0x00, 0x00, 0xE0, 0x60, 0x60, 0x78, 0x6C, 0x66, 0x66, 0x66, 0x66, 0x7C, 0x00, 0x00, 0x00,
  /* c (99) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0xC6, 0xC0, 0xC0, 0xC0, 0xC6, 0x7C, 0x00, 0x00, 0x00,
  /* d (100) */ 0x00, 0x00, 0x00, 0x1C, 0x0C, 0x0C, 0x3C, 0x6C, 0xCC, 0xCC, 0xCC, 0xCC, 0x76, 0x00, 0x00, 0x00,
  /* e (101) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0xC6, 0xFE, 0xC0, 0xC0, 0xC6, 0x7C, 0x00, 0x00, 0x00,
  /* f (102) */ 0x00, 0x00, 0x00, 0x38, 0x6C, 0x64, 0x60, 0xF0, 0x60, 0x60, 0x60, 0x60, 0xF0, 0x00, 0x00, 0x00,
  /* g (103) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x7C, 0x0C, 0xCC, 0x78,
  /* h (104) */ 0x00, 0x00, 0x00, 0xE0, 0x60, 0x60, 0x6C, 0x76, 0x66, 0x66, 0x66, 0x66, 0xE6, 0x00, 0x00, 0x00,
  /* i (105) */ 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00,
  /* j (106) */ 0x00, 0x00, 0x00, 0x06, 0x06, 0x00, 0x0E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x66, 0x66, 0x3C,
  /* k (107) */ 0x00, 0x00, 0x00, 0xE0, 0x60, 0x60, 0x66, 0x6C, 0x78, 0x78, 0x6C, 0x66, 0xE6, 0x00, 0x00, 0x00,
  /* l (108) */ 0x00, 0x00, 0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00,
  /* m (109) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE6, 0xFF, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0x00, 0x00, 0x00,
  /* n (110) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDC, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x00, 0x00,
  /* o (111) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00,
  /* p (112) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDC, 0x66, 0x66, 0x66, 0x66, 0x66, 0x7C, 0x60, 0x60, 0xF0,
  /* q (113) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x7C, 0x0C, 0x0C, 0x1E,
  /* r (114) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDC, 0x76, 0x66, 0x60, 0x60, 0x60, 0xF0, 0x00, 0x00, 0x00,
  /* s (115) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0xC6, 0x60, 0x38, 0x0C, 0xC6, 0x7C, 0x00, 0x00, 0x00,
  /* t (116) */ 0x00, 0x00, 0x00, 0x10, 0x30, 0x30, 0xFC, 0x30, 0x30, 0x30, 0x30, 0x36, 0x1C, 0x00, 0x00, 0x00,
  /* u (117) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x76, 0x00, 0x00, 0x00,
  /* v (118) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC3, 0xC3, 0xC3, 0xC3, 0x66, 0x3C, 0x18, 0x00, 0x00, 0x00,
  /* w (119) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC3, 0xC3, 0xC3, 0xDB, 0xDB, 0xFF, 0x66, 0x00, 0x00, 0x00,
  /* x (120) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC3, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0xC3, 0x00, 0x00, 0x00,
  /* y (121) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0x7E, 0x06, 0x0C, 0xF8,
  /* z (122) */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xCC, 0x18, 0x30, 0x60, 0xC6, 0xFE, 0x00, 0x00, 0x00,
  /* { (123) */ 0x00, 0x00, 0x00, 0x0E, 0x18, 0x18, 0x18, 0x70, 0x18, 0x18, 0x18, 0x18, 0x0E, 0x00, 0x00, 0x00,
  /* | (124) */ 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00,
  /* } (125) */ 0x00, 0x00, 0x00, 0x70, 0x18, 0x18, 0x18, 0x0E, 0x18, 0x18, 0x18, 0x18, 0x70, 0x00, 0x00, 0x00,
  /* ~ (126) */ 0x00, 0x00, 0x00, 0x76, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* BLK(127)*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};


#define countof(a) (sizeof(a) / sizeof(a[0]))

char timestring[20];

void setup() {
  Serial.begin(9600);

  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);

  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printDateTime(compiled);
  Serial.println();

  if (Rtc.GetIsWriteProtected())
  {
    Serial.println("RTC was write protected, enabling writing now");
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled)
  {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);
  }
  else if (now > compiled)
  {
    Serial.println("RTC is newer than compile time. (this is expected)");
  }
  else if (now == compiled)
  {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }

  // Set up Timer2 as the scanning interrupt timer
  cli(); // clear interrupts
  TCCR2A = 0; TCCR2B = 0; TCNT2  = 0;
  TCCR2B |= (1 << CS12) | (1 << CS10);     // Set 1024 prescaler
  // 160Hz scan rate = 10 frames/second (16 pairs of rows)
  OCR2A = 97;  // 97 = (16,000,000 / (1024*160)) - 1
  TCCR2A |= (1 << WGM21); TIMSK2 |= (1 << OCIE2A);

  pinMode(latchPin, OUTPUT);  pinMode(clockPin, OUTPUT);
  pinMode(data_R1, OUTPUT);   pinMode(data_R2, OUTPUT);

  pinMode(en_74138, OUTPUT);
  pinMode(la_74138, OUTPUT);  pinMode(lb_74138, OUTPUT);
  pinMode(lc_74138, OUTPUT);  pinMode(ld_74138, OUTPUT);

  digitalWrite(en_74138, LOW);
  digitalWrite(data_R1, HIGH); digitalWrite(data_R2, HIGH);
  counter = millis();
  sei(); //allow interrupts

  // Demo affichage texte static
  //nox-custom: draw " ON AIR "
  drawChar( 0, 0, char('h'), 0, false);
  drawChar( 8, 0, char('h'), 0, false);
  drawChar(16, 0, char(':'), 0, false);
  drawChar(24, 0, char('m'), 0, false);
  drawChar(32, 0, char('m'), 0, false);
  drawChar(40, 0, char(':'), 0, false);
  drawChar(48, 0, char('s'), 0, false);
  drawChar(56, 0, char('s'), 0, false);
  //end nox-custom
  delay(1000);
}

void loop() {
  RtcDateTime now = Rtc.GetDateTime();

  //  printTime(timestring, now);
  char datestring[20];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u:%02u:%02u"),
             now.Hour(),
             now.Minute(),
             now.Second() );
  Serial.print(datestring);
  Serial.println();

  //  drawChar( 0, 0, char(' '), 0, false);
  //  drawChar( 8, 0, char(' '), 0, false);
  //  drawChar(16, 0, char(' '), 0, false);
  //  drawChar(24, 0, char(' '), 0, false);
  //  drawChar(32, 0, char(' '), 0, false);
  //  drawChar(40, 0, char(' '), 0, false);
  //  drawChar(48, 0, char(' '), 0, false);
  //  drawChar(56, 0, char(' '), 0, false);
  //  //end nox-custom
  //  delay(100);

  for (uint8_t i = 0; i < 128; i++) /* boucle << classique >> pour le parcours d'un tableau */
  {
    buffer[i] = 0;  /* chaque case du tableau reçoit son indice comme valeur */
  }

  //  drawChar( 0, 0, char(' '), 0, false);
  //  delay(10);
  drawChar( 0, 0, char(datestring[0]), 0, false);
  //  drawChar( 8, 0, char(' '), 0, false);
  //  delay(10);
  drawChar( 8, 0, char(datestring[1]), 0, false);
  //  drawChar(16, 0, char(' '), 0, false);
  //  delay(10);
  drawChar(16, 0, char(datestring[2]), 0, false);
  //  drawChar(24, 0, char(' '), 0, false);
  //  delay(10);
  drawChar(24, 0, char(datestring[3]), 0, false);
  //  drawChar(32, 0, char(' '), 0, false);
  //  delay(10);
  drawChar(32, 0, char(datestring[4]), 0, false);
  //  drawChar(40, 0, char(' '), 0, false);
  //  delay(10);
  drawChar(40, 0, char(datestring[5]), 0, false);
  //  drawChar(48, 0, char(' '), 0, false);
  //  delay(10);
  drawChar(48, 0, char(datestring[6]), 0, false);
  //  drawChar(56, 0, char(' '), 0, false);
  //  delay(10);
  drawChar(56, 0, char(datestring[7]), 0, false);

  delay(1000); // ten seconds
}

void printDateTime(const RtcDateTime& dt)
{
  char datestring[20];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second() );
  Serial.print(datestring);
}

void printTime(char * t_str, const RtcDateTime& dt)
{
  // char datestring[20];

  snprintf_P(t_str,
             countof(t_str),
             PSTR("%02u:%02u:%02u"),
             dt.Hour(),
             dt.Minute(),
             dt.Second() );
  Serial.print(t_str);
}

// Routine to print a single character in one of 8 columns
// Inputs:
// x is one of (0,16,24,32,40,48,56),
// y (0 to 16 or 24 depending upon font size),
// n is either (0 to 9) or an ascii value,
// font=0 for big font, 2 for small font,
// inverse is true for an inverted character
void drawChar(uint16_t x, uint16_t y, byte n, byte font, boolean inverse) {
  byte charbytes[16], fontrows, xover8 = x >> 3;
  int index;

  if (0 != (x % 8)) return;    // x not a multiple of 8
  if ((n > 9) && (n < 32)) return; // invalid character
  if (font == 0) fontrows = 16; else fontrows = 8;
  if ((n >= 0) && (n <= 9)) index = (n + 16) * fontrows; else index = (n - 32) * fontrows; // go to the right code for this character

  // addressing start at buffer and add y (rows) * (WIDTH is 64 so WIDTH/8) is 8 plus (x / 8) is 0 to 7
  byte *pDst = buffer + (y << 3) + xover8;
  for (byte i = 0; i < fontrows; i++) { // fill up the charbytes array with the right bits
    if (font == 0) charbytes[i] = pgm_read_byte(&(font8x16_basic[index + i]));
    // reverse bit order for fonts 0 and 1
    if (font != 0) {
      charbytes[i] = (charbytes[i] & 0xF0) >> 4 | (charbytes[i] & 0x0F) << 4;
      charbytes[i] = (charbytes[i] & 0xCC) >> 2 | (charbytes[i] & 0x33) << 2;
      charbytes[i] = (charbytes[i] & 0xAA) >> 1 | (charbytes[i] & 0x55) << 1;
    }
    if (inverse) charbytes[i] = ~charbytes[i];
  }

  //  const byte *pSrc = charbytes; // point at the first set of 8 pixels
  byte *pSrc = charbytes; // point at the first set of 8 pixels

  for (byte i = 0; i < fontrows; i++) {
    *pDst |= *pSrc;  //nox-custom: OR added to draw OVER buffer instead of replace

    // nox-custom: add a line above and below chars
    //    if ( i == 1 || i == 14) {
    //      *pDst = byte(0xFF);
    //    } //end nox-custom

    pDst += 8;         // go to next row on buffer
    pSrc++;            // go to next set of 8 pixels in character
  }
}

void moveLeft(byte pixels, byte rowstart, byte rowstop) { // routine to move certain rows on the screen "pixels" pixels to the left
  byte row, column;
  short unsigned int address;

  for (column = 0; column < 8; column++) {
    for (row = rowstart; row < rowstop; row++) {
      address = (row << 3) + column; /// right here!

      if (column == 7) {
        buffer[address] = buffer[address] << pixels; // shuffle pixels left on last column and fill with a blank
      }
      else {                // shuffle pixels left and add leftmost pixels from next column
        byte incomingchar = buffer[address + 1];
        buffer[address] = buffer[address] << pixels;

        for (byte x = 0; x < pixels; x++) {
          buffer[address] += ((incomingchar & (128 >> x)) >> (7 - x)) << (pixels - x - 1);
        }
      }
    }
  }
}

void shiftOut(byte row) { // fast routine to shove out 8 columns into two rows via board's shift registers
  for (byte column = 0; column < 8; column++) {
    byte index = column + (row << 3);
    for (byte i = 0; i < 8; i++) {
      PORTB &= ~(3 << (data_R1 - 8));                            // data_R2 is LOW; data_R1 is LOW;
      PORTB &= ~(1 << (clockPin - 8));                           // digitalWrite(clockPin,LOW);
      PORTB |= !((buffer[index] >> (7 - i)) & 0x01) << (data_R1 - 8); // top set of rows
      PORTB |= !((buffer[index + 128] >> (7 - i)) & 0x01) << (data_R2 - 8); // bottom set of rows
      PORTB |= 1 << (clockPin - 8);                              // digitalWrite(clockPin,HIGH);
    }
  }
}

ISR(TIMER2_COMPA_vect) {
  cli();
  digitalWrite(en_74138, HIGH);     // Turn off display
  shiftOut(ScanRow);                // Shift out 8 columns
  digitalWrite(latchPin, LOW);
  digitalWrite(latchPin, HIGH);
  PORTD = (ScanRow << 3) | (PORTD & 0X87); // Highlight row: pins 3 4 5 6 (la_74138 lb_74138 lc_74138 ld_74138)
  digitalWrite(en_74138, LOW);     // Turn on display
  ScanRow++;                       // Do the next pair of rows next time this routine is called
  if (ScanRow == 16) ScanRow = 0;
  sei();
}
