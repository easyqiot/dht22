#ifndef _IO_CONFIG_H__
#define _IO_CONFIG_H__

#define EASYQ_RECV_BUFFER_SIZE  4096
#define EASYQ_SEND_BUFFER_SIZE  512 
#define EASYQ_HOSTNAME			"192.168.8.44"
#define EASYQ_PORT				1085

#define EASYQ_LOGIN				"dht"
#define FOTA_QUEUE				EASYQ_LOGIN":fota"
#define FOTA_STATUS_QUEUE		EASYQ_LOGIN":fota:status"
#define DHT_QUEUE						EASYQ_LOGIN
#define DHT_STATUS_QUEUE				EASYQ_LOGIN":status"

#endif

