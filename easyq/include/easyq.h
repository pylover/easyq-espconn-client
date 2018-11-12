
#ifndef EASYQ_H_
#define EASYQ_H_

#include "os_type.h"
#include "ip_addr.h"


#define EASYQ_TASK_QUEUE_SIZE	1
#define EASYQ_TASK_PRIO			2
#define EASYQ_RECV_BUFFER_SIZE  1024
#define EASYQ_SEND_BUFFER_SIZE  1024
#define EASYQ_RECONNECT_INTERVAL	5


typedef enum {
	EASYQ_SIG_CONNECT,
	EASYQ_SIG_RECONNECT,
	EASYQ_SIG_DISCONNECT,
	EASYQ_SIG_SEND,
	EASYQ_SIG_DELETE,
	
	// Callbacks
	EASYQ_SIG_SENT,
	EASYQ_SIG_CONNECTED,
	EASYQ_SIG_DISCONNECTED,
} EasyQSignal;

typedef enum {
	EASYQ_IDLE = 0,
	EASYQ_CONNECTING,
	EASYQ_CONNECTED,  // TODO: Rename it to READY
	EASYQ_DISCONNECTING,
	EASYQ_RECONNECT,
	EASYQ_RECONNECTING,
	EASYQ_SENDING,
	EASYQ_DELETE,
} EasyQStatus;


typedef enum {
	EASYQ_OK,
	EASYQ_ERR_ALREADY_INITIALIZED,		// 1
	EASYQ_ERR_ALREADY_CONNECTED,		// 2
	EASYQ_ERR_ALREADY_CONNECTING,		// 3
	EASYQ_ERR_DISCONNECTING,			// 4
	EASYQ_ERR_DELETING,					// 5
	EASYQ_ERR_NOT_CONNECTED,			// 6
	EASYQ_ERR_TCP_SEND,					// 7
	EASYQ_NOT_READY_FOR_SEND,			// 8
	EASYQ_ERR_ALREADY_SENDING,			// 9
} EasyQError;	


typedef void (*EasyQCallback)(void *);
typedef void (*EasyQMessageCallback)(void*, const char*, const char*, uint16_t);

typedef struct easy_session {
	struct espconn *tcpconn;
	char *hostname;
	uint16_t port;
	char *login;
	EasyQStatus status;
	ip_addr_t ip;
	ETSTimer timer; // TODO: rename it to reconnect_timer
	uint64_t ticks;
	EasyQCallback onconnect;
	EasyQCallback ondisconnect;
	EasyQCallback onconnectionerror;
	EasyQMessageCallback onmessage;
	char * send_buffer;
	size_t sendbuffer_length;
	char * recv_buffer;
	size_t recvbuffer_length;
} EasyQSession;


EasyQError ICACHE_FLASH_ATTR 
easyq_connect(EasyQSession *eq); 

EasyQError ICACHE_FLASH_ATTR 
easyq_disconnect(EasyQSession *eq); 

EasyQError ICACHE_FLASH_ATTR 
easyq_init(EasyQSession *eq, const char *hostname, uint16_t port, 
		const char *login);

//EasyQError ICACHE_FLASH_ATTR 
//easyq_delete(EasyQSession *eq);
//
//EasyQError ICACHE_FLASH_ATTR
//easyq_wait_for_send(EasyQSession *eq, uint8_t tries) {
//
//EasyQError ICACHE_FLASH_ATTR
//easyq_pull(EasyQSession *eq, const char *queue);
//
//EasyQError ICACHE_FLASH_ATTR
//easyq_pull_all(EasyQSession *eq, const char **queue, size_t qlen);
//
//EasyQError ICACHE_FLASH_ATTR
//easyq_ignore(EasyQSession *eq, const char *queue);
//
//EasyQError ICACHE_FLASH_ATTR
//easyq_push(EasyQSession *eq, const char *queue, const char *message);

#endif

