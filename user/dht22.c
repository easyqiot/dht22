#include <eagle_soc.h>
#include <c_types.h>
#include <gpio.h>

#include "dht22.h"
#include "debug.h"


LOCAL uint32_t lasttime;
LOCAL struct dht_session dht;


LOCAL void dht_cleanup() {
	GPIO_OUTPUT_SET(GPIO_ID_PIN(DHT_NUM), 1);
	ETS_GPIO_INTR_DISABLE();
	dht.status = DHT_IDLE;
}


LOCAL void dht_add_bit(uint8_t b) {
	uint8_t pos = dht.bit % 8;

	dht.current_byte |= b << (7-pos);
	if (pos == 7) {
		dht.data[dht.bit / 8] = dht.current_byte;
		dht.current_byte = 0;
	}
}


LOCAL bool dht_validate_data() {
	//char sum = dht.data[0];
	char sum = 0;
	sum += dht.data[0];
	sum += dht.data[1];
	sum += dht.data[2];
	sum += dht.data[3];
	return sum == dht.data[4];
}


void dht_intr_handler()
{
	uint32_t timedelta;
    bool hi = GPIO_INPUT_GET(GPIO_ID_PIN(DHT_NUM)) ;
	if (hi) {
		lasttime = system_get_time();
		if (dht.status == DHT_WAITING) {
			dht.status = DHT_LEADER;
		}
		return;
	}
	
    timedelta = system_get_time() - lasttime;

	switch (dht.status) {
		case DHT_LEADER:
			dht.status = DHT_DATA;
			break;

		case DHT_DATA:
			dht_add_bit(timedelta/70);
			dht.bit++;
			if (dht.bit == DHT_DATA_SIZE) {
				if (dht_validate_data()) {
					dht_result_t r;
					r.humidity = dht.data[0];
					r.humidity <<= 8;
					r.humidity |= dht.data[1];
					
					r.temperature = dht.data[2];
					r.temperature <<= 8;
					r.temperature |= dht.data[3];

					if (dht.callback) {
						dht.callback(&r);
					}
				}
				else {
					ERROR("Data validation failed\r\n");
				}

				dht_cleanup();
				return;
			}
			break;
	}
 }


void interrupt_dispatch()
{
    s32 gpio_status;
    gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);
    if( (gpio_status >> DHT_NUM)& BIT0 ){
        dht_intr_handler();
    }  
}


void dht_read() {
	dht.bit = 0;
	dht.current_byte = 0;
	memset(&dht.data[0], 0, DHT_DATA_SIZE/8);
	dht.status = DHT_REQ;
	GPIO_OUTPUT_SET(GPIO_ID_PIN(DHT_NUM), 0);
	os_delay_us(1200);
	GPIO_OUTPUT_SET(GPIO_ID_PIN(DHT_NUM), 1);
	os_delay_us(30);

	GPIO_DIS_OUTPUT(GPIO_ID_PIN(DHT_NUM));
	ETS_GPIO_INTR_ENABLE();
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(DHT_NUM));
	dht.status = DHT_WAITING;
}


void dht_init() {
		
	// DATA Line
	PIN_FUNC_SELECT(DHT_MUX, DHT_FUNC);
	PIN_PULLUP_DIS(DHT_MUX);

	dht_cleanup();
	ETS_GPIO_INTR_ATTACH(interrupt_dispatch, NULL);
	gpio_pin_intr_state_set(GPIO_ID_PIN(DHT_NUM), GPIO_PIN_INTR_ANYEDGE);
	//wifi_enable_gpio_wakeup(DHT_NUM, GPIO_PIN_INTR_LOLEVEL);
}


void dht_register_callback(DHTCallback c) {
	dht.callback = c;
}

