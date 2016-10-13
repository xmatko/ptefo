/*!
 * \file socks_tcp_mgt.c
 * \brief Gestion des sockets TCP (Communication avec Nc_core)
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 08/09/2014
 *
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"
#include "tlog.h"
#include "socks_mgt.h"
#include "socks_tcp_mgt.h"


int F_TCP_Socket_Create(sSockTab ** pSocket, int port, int max_clients)
{
    sSockTab * pSockL = NULL;
    pSockL = (sSockTab *)malloc(sizeof (sSockTab));
    int sockopt_value = -1;
    
    /* Init des valeurs de la structure */
    F_Socket_InitStruct(pSockL);
    
    /* Création d'un socket TCP serveur */
    pSockL->fd = -1;
    if((pSockL->fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        return INVALID_SOCKET;
    }
    
    /* Description de la socket */
    pSockL->sin = (struct sockaddr_in *)malloc(sizeof (*(pSockL->sin)));
    memset((char *)pSockL->sin, 0, sizeof (*(pSockL->sin)));
    pSockL->sin->sin_family = AF_INET;
    pSockL->sin->sin_addr.s_addr = htonl(INADDR_ANY);
    pSockL->port = (unsigned short)port;
    pSockL->sin->sin_port = htons(port);

    /* Options de la socket */
    sockopt_value = 1;
    if (setsockopt(pSockL->fd, SOL_SOCKET, SO_REUSEADDR, &sockopt_value, (socklen_t)sizeof (sockopt_value)) < 0)
        die("F_TCP_Socket_Create: setsockopt()");
        
    /* Bind de la socket au port defini dans la structure de description */
    if (bind(pSockL->fd, (struct sockaddr *)pSockL->sin, sizeof(*(pSockL->sin))) == SOCKET_ERROR) {
        die("F_TCP_Socket_Create: bind()");
        return SOCKET_ERROR;
    }
    
    /* Mise en écoute de la socket serveur TCP */
    if(listen(pSockL->fd, max_clients) == SOCKET_ERROR) {
        die("F_TCP_Socket_Create: listen()");
        return SOCKET_ERROR;
    }
    
    /* Initialisation des buffers de la socket */
    F_Socket_ResetBuf(pSockL);
    
    *pSocket = pSockL;
    return EXIT_SUCCESS;
}


int F_TCP_SocketCli_Create(sSockTab ** pSocket, char * dst_ip, int dst_port, struct timeval rcv_to, struct timeval snd_to)
{
    sSockTab * pSockL = NULL;
    pSockL = (sSockTab *)malloc(sizeof (sSockTab));
    int sockopt_value = -1;
    int slen = -1;
    
    /* Init des valeurs de la structure */
    F_Socket_InitStruct(pSockL);
    
    /* Création d'un socket TCP cliente */
    pSockL->fd = -1;
    if((pSockL->fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        return INVALID_SOCKET;
    }
    
    /* Description de la socket locale
       La structure sin n'est généralement pas utilisée dans le cas d'une socket cliente UDP.
       Néanmoins, nous l'utilisons ici pour fixer le port source à 0 afin de pouvoir récuperer le port 
       aléatoire choisi par le système lors du sendto()
     */
    pSockL->sin->sin_family = AF_INET;
    pSockL->sin->sin_addr.s_addr = htonl(INADDR_ANY);
    pSockL->sin->sin_port = 0;  /* On fixe le port source à 0 afin de pouvoir récupere le port aléatoire choisi par le système lors du sendto */
    if (bind(pSockL->fd, (struct sockaddr *)pSockL->sin, sizeof(*(pSockL->sin))) == SOCKET_ERROR) {
        die("F_TCP_SocketCli_Create: bind socket client()");
        return SOCKET_ERROR;
    }
    slen = sizeof (*(pSockL->sin));
    getsockname(pSockL->fd, (struct sockaddr *)pSockL->sin, (socklen_t *)&slen);
    
    /* Options de la socket */
    if (setsockopt(pSockL->fd, SOL_SOCKET, SO_SNDTIMEO, &snd_to, (socklen_t)sizeof (snd_to)) < 0)
        die("setsockopt() SO_SNDTIMEO");
    if (setsockopt(pSockL->fd, SOL_SOCKET, SO_RCVTIMEO, &rcv_to, (socklen_t)sizeof (rcv_to)) < 0)
        die("setsockopt() SO_RCVTIMEO");
    sockopt_value = 1;
    if (setsockopt(pSockL->fd, SOL_SOCKET, SO_REUSEADDR, &sockopt_value, (socklen_t)sizeof (sockopt_value)) < 0)
        die("setsockopt() SO_REUSEADDR");
    sockopt_value = 1024;
    if (setsockopt(pSockL->fd, SOL_SOCKET, SO_RCVBUF, &sockopt_value, (socklen_t)sizeof (sockopt_value)) < 0)
        die("setsockopt() SO_RCVBUF");
    if (setsockopt(pSockL->fd, SOL_SOCKET, SO_SNDBUF, &sockopt_value, (socklen_t)sizeof (sockopt_value)) < 0)
        die("setsockopt() SO_SNDBUF");
    
    /* Description de la socket distante associée (socket serveur à connecter)*/
    pSockL->sremote->sin_family = AF_INET;
    pSockL->port = dst_port;
    pSockL->sremote->sin_port = htons(pSockL->port);
    memcpy(pSockL->addr_ip, dst_ip, ADDR_IP_LEN * sizeof (char));
    if (inet_aton(pSockL->addr_ip , &(pSockL->sremote->sin_addr)) == 0)
        die("F_TCP_SocketCli_Create: inet_aton()");
    
    /* Initialisation des buffers de la socket */
    F_Socket_ResetBuf(pSockL);
    
    *pSocket = pSockL;
    return EXIT_SUCCESS;
}


