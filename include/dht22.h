
#ifndef _DHT_H
#define _DHT_H

#include <gpio.h>
#include <eagle_soc.h>


// DATA Line
//#define DHT_MUX			PERIPHS_IO_MUX_GPIO5_U
//#define DHT_NUM			5
//#define DHT_FUNC		FUNC_GPIO5
#define DHT_MUX			PERIPHS_IO_MUX_GPIO4_U
#define DHT_NUM			4
#define DHT_FUNC		FUNC_GPIO4


#define DHT_DATA_SIZE	40
#define DHT_ONE_US		70
#define DHT_ZERO_US		30


typedef struct {
	int16_t temperature;
	int16_t humidity;
} dht_result_t;


typedef void (*DHTCallback)(dht_result_t*);


enum dht_status {
	DHT_IDLE,
	DHT_REQ,
	DHT_WAITING,
	DHT_LEADER,
	DHT_DATA
};


struct dht_session {
	enum dht_status status;
	uint8_t bit;
	char data[DHT_DATA_SIZE/8];
	char current_byte;
	DHTCallback callback;
};


void dht_init();
void dht_read();
void dht_register_callback(DHTCallback);


#endif
