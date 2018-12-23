#include <ets_sys.h>
#include <osapi.h>


#define CAROUSEL_INTERVAL	160

LOCAL ETSTimer _timer;
LOCAL uint8_t _ticks = 0;

LOCAL char _buffer[32] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
LOCAL uint8_t _length = 0;


void carousel_timer_func(void *args) {
	int16_t i, x;
	uint8_t char_index = _ticks / 4;
	display_clear();
	for (i = 3; i >= 0; i--) {
		x = i * 4 - _ticks % 4;
		display_char(_buffer[(char_index + i) % _length], x);
		//if (x > 0) {
		//	display_dot(x-1, 7, 1);
		//}
	}
	display_draw();
	if (char_index == _length) {
		_ticks = 0;
	}
	else {
		_ticks++;
	}
	os_timer_arm(&_timer, CAROUSEL_INTERVAL, 0);
}


void ICACHE_FLASH_ATTR
carousel_start(const char *v, uint8_t length)  {
	os_timer_disarm(&_timer);
	if (length <= 2) {
		int i;
		display_clear();
		for (i = 0; i < length; i++) {
			display_char(v[i], i * 4); 
		}
		display_draw();
		return;
	}
	os_timer_setfn(&_timer, (os_timer_func_t*) carousel_timer_func, 0);
	strncpy(&_buffer[0], v, length);
	_length = length;
	_ticks = 0;
	os_timer_arm(&_timer, CAROUSEL_INTERVAL, 0);
}


void ICACHE_FLASH_ATTR
carousel_stop()  {
	os_timer_disarm(&_timer);
	display_clear();
	display_draw();
}
