/*!
 * \file socks_tcp_mgt.h
 * \brief Fichier d'entête du module de gestion des sockets TCP (Communication avec Nc_core)
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 09/2014
 *
 */

#ifndef _SOCKS_TCP_MGT_H_
#define _SOCKS_TCP_MGT_H_


#define DEFAULT_REMOTE_TCP_SERVER_IP    "127.0.0.1"
#define DEFAULT_REMOTE_TCP_SERVER_PORT  3001
//#define DEFAULT_REMOTE_TCP_SERVER_IP    "10.106.2.10"
//#define DEFAULT_REMOTE_TCP_SERVER_PORT  55006

#define DEFAULT_TCP_SERVER_PORT         3010
//#define DEFAULT_TCP_SERVER_PORT         55005

#define DEFAULT_TCP_SERVER_MAX_CLIENTS  1
#define DEFAULT_TCP_CLI_RCV_TIMEOUT_MS  1000
#define DEFAULT_TCP_CLI_SND_TIMEOUT_MS  1000
#define TCP_BUF_SIZE                    1024
#define DEFAUT_TCP_CLI_RETRY_DELAY_SEC  5
#define DEFAUT_TCP_CLI_MAX_RETRY        3

int F_TCP_Socket_Create(sSockTab ** pSocket, int port, int max_clients);
int F_TCP_SocketCli_Create(sSockTab ** pSocket, char * dst_ip, int dst_port, struct timeval rcv_to, struct timeval snd_to);

#endif /* _SOCKS_TCP_MGT_H_ */
