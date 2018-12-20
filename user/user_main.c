
// Internal 
#include "partition.h"
#include "wifi.h"
#include "config.h"
#include "dht22.h"

// SDK
#include <ets_sys.h>
#include <osapi.h>
#include <gpio.h>
#include <mem.h>
#include <user_interface.h>
#include <driver/uart.h>
#include <upgrade.h>
#include <string.h>
#include <osapi.h>

// LIB: EasyQ
#include "easyq.h" 
#include "debug.h"


#define VERSION				"0.1.0"
#define HUMIDIFIER_QUEUE	"humidifier"
#define HUMIDITY_MIN		400
#define HUMIDITY_MAX		450


LOCAL EasyQSession eq;
LOCAL ETSTimer dht_status_timer;
LOCAL uint32_t ticks = 0;


void dht_result(dht_result_t * result) {
	ticks++;
	if (ticks % 10 == 0) {
		if (result->humidity < HUMIDITY_MIN) {
			easyq_push(&eq, HUMIDIFIER_QUEUE, "on");
			return;
		}
		else if (result->humidity > HUMIDITY_MAX) {
			easyq_push(&eq, HUMIDIFIER_QUEUE, "off");
			return;
		}
	}
	char str[32];
	os_sprintf(str, "(%d) H: %d, T: %d", ticks, result->humidity, 
			result->temperature);
	easyq_push(&eq, DHT_STATUS_QUEUE, str);
}


void
fota_report_status(const char *q) {
	char str[50];
	float vdd = system_get_vdd33() / 1024.0;

	uint8_t image = system_upgrade_userbin_check();
	os_sprintf(str, "Image: %s Version: "VERSION" VDD: %d.%03d", 
			(UPGRADE_FW_BIN1 == image)? "FOTA": "APP",
			(int)vdd, 
			(int)(vdd*1000)%1000
		);
	easyq_push(&eq, q, str);
}


void ICACHE_FLASH_ATTR
dht_report_status(void *args) {
	dht_read();
}

 
void ICACHE_FLASH_ATTR
auto_report_dht_status_on(uint32_t interval) {
	os_timer_disarm(&dht_status_timer);
	os_timer_setfn(&dht_status_timer, 
			(os_timer_func_t*) dht_report_status, NULL);
	os_timer_arm(&dht_status_timer, interval, 1);
}


void ICACHE_FLASH_ATTR
auto_report_dht_status_off() {
	os_timer_disarm(&dht_status_timer);
}


void ICACHE_FLASH_ATTR
easyq_message_cb(void *arg, const char *queue, const char *msg, 
		uint16_t message_len) {
	if (strcmp(queue, FOTA_QUEUE) == 0) {
		if (msg[0] == 'R') {
			INFO("Rebooting to FOTA ROM\r\n");
			system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
			system_upgrade_reboot();
		}
		else if (msg[0] == 'I') {
			fota_report_status(FOTA_STATUS_QUEUE);
		}
	}
	else if (strcmp(queue, DHT_QUEUE) == 0) {
		if (msg[0] == 'I') {
			dht_report_status(NULL);
		}
		else if (msg[0] == 'S') {
			auto_report_dht_status_on(atoi(msg+1));
		}
		else if (msg[0] == 'E') {
			auto_report_dht_status_off();
		}
	}

}


void ICACHE_FLASH_ATTR
easyq_connect_cb(void *arg) {
	INFO("EASYQ: Connected to %s:%d\r\n", eq.hostname, eq.port);
	INFO("\r\n***** DHT22 "VERSION"****\r\n");
	const char * queues[] = {FOTA_QUEUE, DHT_QUEUE};
	easyq_pull_all(&eq, queues, 2);
	auto_report_dht_status_on(DHT_INTERVAL_MS);
}


void ICACHE_FLASH_ATTR
easyq_connection_error_cb(void *arg) {
	EasyQSession *e = (EasyQSession*) arg;
	INFO("EASYQ: Connection error: %s:%d\r\n", e->hostname, e->port);
	INFO("EASYQ: Reconnecting to %s:%d\r\n", e->hostname, e->port);
}


void easyq_disconnect_cb(void *arg)
{
	EasyQSession *e = (EasyQSession*) arg;
	INFO("EASYQ: Disconnected from %s:%d\r\n", e->hostname, e->port);
	easyq_delete(&eq);
}


void wifi_connect_cb(uint8_t status) {
    if(status == STATION_GOT_IP) {
        easyq_connect(&eq);
    } else {
        easyq_disconnect(&eq);
    }
}


void user_init(void) {
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    os_delay_us(60000);


	EasyQError err = easyq_init(&eq, EASYQ_HOSTNAME, EASYQ_PORT, EASYQ_LOGIN);
	if (err != EASYQ_OK) {
		ERROR("EASYQ INIT ERROR: %d\r\n", err);
		return;
	}
	eq.onconnect = easyq_connect_cb;
	eq.ondisconnect = easyq_disconnect_cb;
	eq.onconnectionerror = easyq_connection_error_cb;
	eq.onmessage = easyq_message_cb;
	
	dht_register_callback(dht_result);
	dht_init();

	//motor_init();
    WIFI_Connect(WIFI_SSID, WIFI_PSK, wifi_connect_cb);
    INFO("System started ...\r\n");
}


void ICACHE_FLASH_ATTR user_pre_init(void)
{
    if(!system_partition_table_regist(at_partition_table, 
				sizeof(at_partition_table)/sizeof(at_partition_table[0]),
				SPI_FLASH_SIZE_MAP)) {
		FATAL("system_partition_table_regist fail\r\n");
		while(1);
	}
}

