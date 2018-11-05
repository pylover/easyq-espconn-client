#include "easyq.h"
#include "debug.h"


os_event_t easyq_task_queue[EASYQ_TASK_QUEUE_SIZE];


LOCAL void ICACHE_FLASH_ATTR
_easyq_tcpconn_delete(EasyQSession *eq)
{
    if (eq->tcpconn != NULL) {
        INFO("Free espconn memory\r\n");
        espconn_delete(eq->tcpconn);
        if (eq->tcpconn->proto.tcp)
            os_free(eq->tcpconn->proto.tcp);
        os_free(eq->tcpconn);
        eq->tcpconn = NULL;
    }
}


LOCAL void ICACHE_FLASH_ATTR
_easyq_delete(EasyQSession *eq)
{
    INFO("Free EasyQ memory\r\n");
	_easyq_tcpconn_delete(eq);
    os_free(eq->hostname);
}


LOCAL void ICACHE_FLASH_ATTR
_easyq_tcpclient_disconnect_cb(void *arg)
{

    struct espconn *tcpconn = (struct espconn *)arg;
    EasyQSession *eq = (EasyQSession *)tcpconn->reverse;
    INFO("TCP: Disconnected callback\r\n");

	if(EASYQ_DELETE == eq->status) {
		_easyq_delete(eq);
		return;
    }

    if(EASYQ_DISCONNECT == eq->status) {
		_easyq_tcpconn_delete(eq);
        eq->status = EASYQ_IDLE;
    }
    else {
        eq->status = EASYQ_CONNECT;
    }

    system_os_post(EASYQ_TASK_PRIO, 0, (os_param_t)eq);
}


void ICACHE_FLASH_ATTR
_easyq_tcpclient_recon_cb(void *arg, sint8 errType)
{
    struct espconn *tcpconn = (struct espconn *)arg;
    EasyQSession *eq = (EasyQSession *)tcpconn->reverse;
    INFO("TCP: Reconnect to %s:%d\r\n", eq->hostname, eq->port);
	eq->status = EASYQ_CONNECT;
}



void ICACHE_FLASH_ATTR
_easyq_tcpclient_connect_cb(void *arg)
{
    struct espconn *tcpconn = (struct espconn *)arg;
    EasyQSession *eq = (EasyQSession *)tcpconn->reverse;
	eq->status = EASYQ_CONNECTED;
	espconn_regist_disconcb(eq->tcpconn, _easyq_tcpclient_disconnect_cb);
    system_os_post(EASYQ_TASK_PRIO, 0, (os_param_t)eq);
}


LOCAL void ICACHE_FLASH_ATTR
_easyq_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
    struct espconn *tcpconn = (struct espconn *)arg;
    EasyQSession *eq = (EasyQSession *)tcpconn->reverse;

    if (ipaddr == NULL)
    {
        INFO("DNS: Found, but got no ip, try to reconnect\r\n");
		_easyq_tcpconn_delete(eq);
        eq->status = EASYQ_CONNECT;
		system_os_post(EASYQ_TASK_PRIO, 0, (os_param_t)eq);
        return;
    }

    INFO("DNS: found ip %d.%d.%d.%d\n",
         *((uint8 *) &ipaddr->addr),
         *((uint8 *) &ipaddr->addr + 1),
         *((uint8 *) &ipaddr->addr + 2),
         *((uint8 *) &ipaddr->addr + 3));

    if (eq->ip.addr == 0 && ipaddr->addr != 0)
    {
        os_memcpy(eq->tcpconn->proto.tcp->remote_ip, &ipaddr->addr, 4);
        espconn_connect(eq->tcpconn);
        INFO("TCP: connecting...\r\n");
    }
}


LOCAL void ICACHE_FLASH_ATTR 
_easyq_connect(EasyQSession *eq) {
    eq->tcpconn = (struct espconn *)os_zalloc(sizeof(struct espconn));
    eq->tcpconn->type = ESPCONN_TCP;
    eq->tcpconn->state = ESPCONN_NONE;
    eq->tcpconn->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
    eq->tcpconn->proto.tcp->local_port = espconn_port();
    eq->tcpconn->proto.tcp->remote_port = eq->port;
    eq->tcpconn->reverse = eq; 
    espconn_regist_connectcb(eq->tcpconn, _easyq_tcpclient_connect_cb);
    espconn_regist_reconcb(eq->tcpconn, _easyq_tcpclient_recon_cb);

    if (UTILS_StrToIP(eq->hostname, &eq->tcpconn->proto.tcp->remote_ip)) {
        INFO("TCP: Connect to ip  %s:%d\r\n", eq->hostname, eq->port);
        espconn_connect(eq->tcpconn);
    }
    else {
        INFO("TCP: Connect to domain %s:%d\r\n", eq->hostname, eq->port);
        espconn_gethostbyname(eq->tcpconn, eq->hostname, &eq->ip, 
				_easyq_dns_found);
    }
}


LOCAL void ICACHE_FLASH_ATTR 
_easyq_disconnect(EasyQSession *eq) {
    espconn_disconnect(eq->tcpconn);
}
