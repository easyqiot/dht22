#include <c_types.h>
#include <eagle_soc.h> 
#include <gpio.h>
#include "debug.h"
#include "io_config.h"


/* GPIO */

// DATA PIN
#define DATA_MUX			PERIPHS_IO_MUX_MTCK_U	
#define DATA_NUM			13
#define DATA_FUNC			FUNC_GPIO13

// Clock PIN
#define CLOCK_MUX			PERIPHS_IO_MUX_MTMS_U	
#define CLOCK_NUM			14
#define CLOCK_FUNC			FUNC_GPIO14

#define DISPLAY_INTENSITY	0


#define GPIO_SET(n, v)	GPIO_OUTPUT_SET(GPIO_ID_PIN(n), v)
#define DATA_SET(v)		GPIO_OUTPUT_SET(GPIO_ID_PIN(DATA_NUM), v)
#define CLOCK_SET(v)	GPIO_OUTPUT_SET(GPIO_ID_PIN(CLOCK_NUM), v)
#define LOW				0
#define HIGH			1


LOCAL uint8_t display_buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};


void ICACHE_FLASH_ATTR
display_send(char data) {
	uint8_t i;
	for (i = 0; i < 8; i++) {
		CLOCK_SET(LOW);
		DATA_SET(data & 1 ? HIGH : LOW);
		data >>= 1;
		CLOCK_SET(HIGH);
	}
}



void ICACHE_FLASH_ATTR
display_send_command(char cmd) {
	DATA_SET(LOW);
	display_send(cmd);
	DATA_SET(HIGH);
}


void ICACHE_FLASH_ATTR
display_send_data(uint8_t address, char data) {
	display_send_command(0x44);
	DATA_SET(LOW);
	display_send(0xC0 | address);
	display_send(data);
	DATA_SET(HIGH);
}



void ICACHE_FLASH_ATTR
display_draw() {
	uint8_t i;
	for(i=0; i < 8; i++) {
		display_send_data(i, display_buffer[i]);
		DATA_SET(LOW);
		CLOCK_SET(LOW);
		CLOCK_SET(HIGH);
		DATA_SET(HIGH);
	}
	display_send_command(0x88 | DISPLAY_INTENSITY);
}


void ICACHE_FLASH_ATTR
display_clear() {
    uint8_t i;
	for(i=0; i < 8; i++) {
		display_buffer[i] = 0;
	}
}


void ICACHE_FLASH_ATTR
display_dot(uint8_t x, uint8_t y, bool on) {
	x &= 0x07;
	y &= 0x07;

	if (on) {
		display_buffer[y] |= 1 << x;
	}
	else {
		display_buffer[y] &= ~(1 << x);
	}
}


void ICACHE_FLASH_ATTR
display_init() {
	PIN_FUNC_SELECT(DATA_MUX, DATA_FUNC);
	GPIO_OUTPUT_SET(GPIO_ID_PIN(DATA_NUM), 1);

	PIN_FUNC_SELECT(CLOCK_MUX, CLOCK_FUNC);
	GPIO_OUTPUT_SET(GPIO_ID_PIN(CLOCK_NUM), 1);
}



LOCAL uint8_t digitmap[10][5] = {
	{7, 5, 5, 5, 7},  // 0
	{7, 2, 2, 3, 2},  // 1
	{7, 1, 7, 4, 7},  // 2
	{7, 4, 7, 4, 7},  // 3
	{4, 4, 7, 5, 5},  // 4
	{7, 4, 7, 1, 7},  // 5
	{7, 5, 7, 1, 7},  // 6
	{4, 4, 4, 4, 7},  // 7
	{7, 5, 7, 5, 7},  // 8
	{7, 4, 7, 5, 7},  // 9
};


void ICACHE_FLASH_ATTR
display_number(uint8_t n) {
	int i;
	if (n < 0 || n > 99) {
		return;
	}
	uint8_t d0 = n % 10;
	uint8_t d1 = n / 10;
	for (i = 0; i < 5; i++) {
		display_buffer[i] = (digitmap[d0][i] << 4) | (d1? digitmap[d1][i]: 0);
	}
}


LOCAL uint8_t charmap[91][5] = {
	{0, 0, 0, 0, 0},  //  32: Space
	{2, 0, 2, 2, 2},  //  33: !
	{0, 0, 0, 5, 5},  //  34: "
	{0, 7, 5, 7, 0},  //  35: #
	{3, 6, 2, 3, 6},  //  36: $
	{4, 1, 2, 4, 1},  //  37: % 
	{7, 5, 2, 5, 2},  //  38: & 
	{0, 0, 0, 2, 2},  //  39: '
	{2, 1, 1, 1, 2},  //  40: ( 
	{2, 4, 4, 4, 2},  //  41: )
	{0, 5, 2, 5, 0},  //  42: * 
	{0, 2, 7, 2, 0},  //  43: + 
	{1, 2, 0, 0, 0},  //  44: , 
	{0, 0, 7, 0, 0},  //  45: - 
	{2, 0, 0, 0, 0},  //  46: . 
	{1, 2, 2, 2, 4},  //  47: / 
	{7, 5, 5, 5, 7},  //  48: 0 
	{7, 2, 2, 3, 2},  //  49: 1 
	{7, 1, 7, 4, 7},  //  50: 2 
	{7, 4, 7, 4, 7},  //  51: 3 
	{4, 4, 7, 5, 5},  //  52: 4 
	{7, 4, 7, 1, 7},  //  53: 5 
	{7, 5, 7, 1, 7},  //  54: 6 
	{4, 4, 4, 4, 7},  //  55: 7 
	{7, 5, 7, 5, 7},  //  56: 8 
	{7, 4, 7, 5, 7},  //  57: 9 
	{0, 2, 0, 2, 0},  //  58: : 
	{1, 2, 0, 2, 0},  //  59: ; 
	{4, 2, 1, 2, 4},  //  60: < 
	{0, 7, 0, 7, 0},  //  61: = 
	{1, 2, 4, 2, 1},  //  62: > 
	{2, 0, 2, 4, 3},  //  63: ? 
	{6, 6, 5, 5, 7},  //  64: @ 
	{5, 5, 7, 5, 7},  //  65: A
	{7, 5, 7, 1, 1},  //  66: b
	{7, 1, 1, 1, 7},  //  67: C
	{3, 5, 5, 5, 3},  //  68: D
	{7, 1, 7, 1, 7},  //  69: E
	{1, 1, 7, 1, 7},  //  70: F
	{7, 4, 7, 5, 7},  //  71: g
	{5, 5, 7, 5, 5},  //  72: H
	{7, 2, 2, 2, 7},  //  73: I
	{3, 4, 4, 4, 6},  //  74: J
	{5, 3, 1, 3, 5},  //  75: K
	{7, 1, 1, 1, 1},  //  76: L
	{5, 5, 5, 7, 5},  //  77: M
	{5, 5, 5, 5, 3},  //  78: N
	{7, 5, 5, 5, 7},  //  79: O
	{1, 1, 7, 5, 7},  //  80: P
	{4, 4, 7, 5, 7},  //  81: q
	{5, 3, 7, 5, 7},  //  82: R
	{3, 4, 2, 1, 6},  //  83: S
	{2, 2, 2, 2, 7},  //  84: T
	{7, 5, 5, 5, 5},  //  85: U
	{2, 5, 5, 5, 5},  //  86: V
	{5, 7, 5, 5, 5},  //  87: W
	{5, 5, 2, 5, 5},  //  88: X
	{7, 4, 7, 5, 5},  //  89: Y
	{7, 1, 2, 4, 7},  //  90: Z
	{3, 1, 1, 1, 3},  //  91: [ 
	{4, 2, 2, 2, 1},  //  92  backslash  
	{6, 4, 4, 4, 6},  //  93: ] 
	{0, 0, 0, 5, 2},  //  94: ^ 
	{7, 0, 0, 0, 0},  //  95: _ 
	{0, 0, 0, 4, 2},  //  96: `   
	{5, 5, 7, 5, 7},  //  97: a 
	{7, 5, 7, 1, 1},  //  98: b 
	{7, 1, 1, 1, 7},  //  99: c 
	{3, 5, 5, 5, 3},  // 100: d 
	{7, 1, 7, 1, 7},  // 101: e 
	{1, 1, 7, 1, 7},  // 102: f 
	{7, 4, 7, 5, 7},  // 103: g 
	{1, 1, 7, 5, 5},  // 104: h 
	{2, 2, 2, 0, 2},  // 105: i 
	{3, 4, 4, 4, 6},  // 106: j 
	{5, 3, 1, 3, 5},  // 107: k 
	{7, 1, 1, 1, 1},  // 108: l 
	{5, 5, 5, 7, 5},  // 109: m 
	{5, 5, 5, 5, 3},  // 110: n 
	{7, 5, 5, 5, 7},  // 111: o 
	{1, 1, 7, 5, 7},  // 112: p 
	{4, 4, 7, 5, 7},  // 113: q 
	{5, 3, 7, 5, 7},  // 114: r 
	{3, 4, 2, 1, 6},  // 115: s 
	{2, 2, 2, 2, 7},  // 116: t 
	{7, 5, 5, 5, 5},  // 117: u 
	{2, 5, 5, 5, 5},  // 118: v 
	{5, 7, 5, 5, 5},  // 119: w 
	{5, 5, 2, 5, 5},  // 120: x 
	{7, 4, 7, 5, 5},  // 121: y 
	{7, 1, 2, 4, 7},  // 122: z 
};


void ICACHE_FLASH_ATTR
display_char(uint8_t c0, int pos) {
	int i;
	if ((c0 < 32) || (c0 > 122) || (pos > 7) || (pos < -2)) {
		return;
	}
	uint8_t d0 = c0 - 32; 
	for (i = 0; i < 5; i++) {
		display_buffer[i+1] &= (0xFF << (pos+3)) | (0xFF >> (8-pos));
		display_buffer[i+1] |= (pos > 0) ? charmap[d0][i] << pos: \
				charmap[d0][i] >> abs(pos);
	}
}


