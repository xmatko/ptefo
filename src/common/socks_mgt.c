/*!
 * \file socks_mgt.c
 * \brief Gestion des sockets
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 03/06/2014
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include "common.h"
#include "tlog.h"
#include "socks_mgt.h"

int F_Socket_InitStruct(sSockTab * pSockL)
{
    pSockL->rcv_maxsize = BUF_SIZE;
    pSockL->snd_maxsize = BUF_SIZE;
    pSockL->rcv_buf = (char *)malloc((pSockL->rcv_maxsize) * sizeof (char));
    pSockL->snd_buf = (char *)malloc((pSockL->snd_maxsize) * sizeof (char));
    pSockL->snd_trame_cpt = 0;
    pSockL->rcv_trame_cpt = 0;
    pSockL->rcv_queue = NULL;
    pSockL->snd_queue = NULL;
    F_SockQueue_Init(&(pSockL->rcv_queue), 100, NULL);
    F_SockQueue_Init(&(pSockL->snd_queue), 100, NULL);
    
    pSockL->port = 0;
    pSockL->addr_ip = (char *)malloc(ADDR_IP_LEN * sizeof (char));
    memset(pSockL->addr_ip, 0, ADDR_IP_LEN * sizeof (char));
    
    /* Allocation de la socket */
    pSockL->sin = (struct sockaddr_in *)malloc(sizeof (*(pSockL->sin)));
    memset((char *)pSockL->sin, 0, sizeof (*(pSockL->sin)));
    
    /* Allocation de la socket distante associée */
    pSockL->sremote =(struct sockaddr_in *)malloc(sizeof (*(pSockL->sremote)));
    memset((char *)pSockL->sremote, 0, sizeof (*(pSockL->sremote)));
    
    return EXIT_SUCCESS;
}

int F_Socket_Create(sSockTab ** pSocket, int port)
{
    sSockTab * pSockL = NULL;
    char * sock_name = (char *)calloc(256U, sizeof (char));
    pSockL = (sSockTab *)malloc(sizeof (sSockTab));
    int sockopt_value = -1;
    
    /* Init des valeurs de la structure */
    F_Socket_InitStruct(pSockL);
    
    /* Création d'une socket UDP */
    pSockL->fd = -1;
    if((pSockL->fd = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        return INVALID_SOCKET;
    }

    /* Description de la socket */
    pSockL->sin->sin_family = AF_INET;
    pSockL->sin->sin_addr.s_addr = htonl(INADDR_ANY);
    pSockL->sin->sin_port = htons(port);
    
    /* Options de la socket */
    sockopt_value = 1;
    if (setsockopt(pSockL->fd, SOL_SOCKET, SO_REUSEADDR, &sockopt_value, (socklen_t)sizeof (sockopt_value)) < 0)
        die("setsockopt()");
    
    /* Bind la socket à la structure de description */
    if (bind(pSockL->fd, (struct sockaddr *)pSockL->sin, sizeof(*(pSockL->sin))) == SOCKET_ERROR) {
        die("bind()");
        return SOCKET_ERROR;
    }
   
    
    sprintf(sock_name, "socket %d:%d send", pSockL->fd, port);
    F_SockQueue_SetName(pSockL->snd_queue, sock_name);
    sprintf(sock_name, "socket %d:%d receive", pSockL->fd, port);
    F_SockQueue_SetName(pSockL->snd_queue, sock_name);
    
    /* Initialisation des buffers de la socket */
    F_Socket_ResetBuf(pSockL);
    
    struct timespec gts;
    clock_gettime(CLOCK_MONOTONIC, &gts);
    pSockL->ref_time_s = gts.tv_sec;
    
    *pSocket = pSockL;
    return EXIT_SUCCESS;
}

int F_Socket_CreateAll(sSockTab *** pSocks, short int baseport, char nb_socks)
{
    int i = -1;
    int ret = -1;
    sSockTab ** pTmp = (sSockTab **)malloc(nb_socks * sizeof (sSockTab *));
    
    for (i = 0; i < nb_socks; i++) {
        if ( (ret = F_Socket_Create(&(pTmp[i]), baseport + i)) < 0 ) // (pMain->pSocket)+i  <=> &(pMain->pSocket[i])
            die("F_Socket_Create()");
        SLOGT(LOG_INFO, "Création socket UDP serveur fd %d, bind sur le port %d\n", pTmp[i]->fd, baseport + i);
    }
    *pSocks = pTmp;
    return EXIT_SUCCESS;
}



int F_SocketCli_Create(sSockTab ** pSocket, char * dst_ip, int dst_port, struct timeval reply_to)
{
    sSockTab * pSockL = NULL;
    pSockL = (sSockTab *)malloc(sizeof (sSockTab));
    char * sock_name = (char *)calloc(256U, sizeof (char));
    int sockopt_value = -1;
    int slen = -1;
    
    /* Init des valeurs de la structure */
    /*
    pSockL->rcv_maxsize = BUF_SIZE;
    pSockL->snd_maxsize = BUF_SIZE;
    pSockL->rcv_buf = (char *)malloc((pSockL->rcv_maxsize) * sizeof (char));
    pSockL->snd_buf = (char *)malloc((pSockL->snd_maxsize) * sizeof (char));
    pSockL->snd_trame_cpt = 0;
    pSockL->rcv_trame_cpt = 0;
    pSockL->rcv_queue = NULL;
    pSockL->snd_queue = NULL;
    pSockL->port = 0;
    pSockL->addr_ip = (char *)malloc(ADDR_IP_LEN * sizeof (char));
    memset(pSockL->addr_ip, 0, ADDR_IP_LEN * sizeof (char));
    */
    F_Socket_InitStruct(pSockL);
    

    /* Création d'une socket UDP */
    pSockL->fd = -1;
    if ((pSockL->fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
        return INVALID_SOCKET;
    }
    
    /* Description de la socket locale
       La structure locale sin n'est généralement pas utilisée dans le cas d'une socket cliente UDP.
       Néanmoins, nous l'utilisons ici pour fixer le port source à 0 afin de pouvoir récuperer le port 
       aléatoire choisi par le système lors du sendto()
     */
    pSockL->sin->sin_family = AF_INET;
    pSockL->sin->sin_addr.s_addr = htonl(INADDR_ANY);
    pSockL->sin->sin_port = 0;  /* On fixe le port source à 0 afin de pouvoir récupere le port aléatoire choisi par le système lors du sendto */
    if (bind(pSockL->fd, (struct sockaddr *)pSockL->sin, sizeof(*(pSockL->sin))) == SOCKET_ERROR) {
        die("bind socket client()");
        return SOCKET_ERROR;
    }
    slen = sizeof (*(pSockL->sin));
    getsockname(pSockL->fd, (struct sockaddr *)pSockL->sin, (socklen_t *)&slen);
    
    /* Options de la socket */
    if (setsockopt(pSockL->fd, SOL_SOCKET, SO_RCVTIMEO, &reply_to, (socklen_t)sizeof (reply_to)) < 0)
        die("setsockopt()");
    sockopt_value = 1;
    if (setsockopt(pSockL->fd, SOL_SOCKET, SO_REUSEADDR, &sockopt_value, (socklen_t)sizeof (sockopt_value)) < 0)
        die("setsockopt()");
    sockopt_value = 1024;
    if (setsockopt(pSockL->fd, SOL_SOCKET, SO_RCVBUF, &sockopt_value, (socklen_t)sizeof (sockopt_value)) < 0)
        die("setsockopt()");
    if (setsockopt(pSockL->fd, SOL_SOCKET, SO_SNDBUF, &sockopt_value, (socklen_t)sizeof (sockopt_value)) < 0)
        die("setsockopt()");
    
    /* Description de la socket distante associée */
    pSockL->sremote->sin_family = AF_INET;
    pSockL->port = dst_port;
    pSockL->sremote->sin_port = htons(pSockL->port);
    memcpy(pSockL->addr_ip, dst_ip, ADDR_IP_LEN * sizeof (char));
    if (inet_aton(pSockL->addr_ip , &(pSockL->sremote->sin_addr)) == 0)
        die("inet_aton()");
    
    sprintf(sock_name, "socket %d:%d send", pSockL->fd, dst_port);
    F_SockQueue_SetName(pSockL->snd_queue, sock_name);
    sprintf(sock_name, "socket %d:%d receive", pSockL->fd, dst_port);
    F_SockQueue_SetName(pSockL->snd_queue, sock_name);
    
    /* Initialisation des buffers de la socket */
    F_Socket_ResetBuf(pSockL);
    
    *pSocket = pSockL;
    return EXIT_SUCCESS;
}


int F_SocketCli_CreateAll(sSockTab *** pSocks, char * remote_ip, short int remote_baseport, struct timeval r_timeout, char nb_socks)
{
    int i = -1;
    int ret = -1;
    sSockTab ** pTmp = (sSockTab **)malloc(nb_socks * sizeof (sSockTab *));
    for (i = 0; i < nb_socks; i++) {
        if ( (ret = F_SocketCli_Create(&(pTmp[i]), remote_ip, remote_baseport + i, r_timeout)) < 0)
            die("F_SocketCli_Creer()");
        SLOGT(LOG_INFO, "Création socket UDP cliente fd %d\n", pTmp[i]->fd);
    }

    *pSocks = pTmp;
    
    return EXIT_SUCCESS;
}

int F_SocketCli_Reset(sSockTab * pSocket)
{
    pSocket->sremote->sin_family = AF_INET;
    pSocket->sremote->sin_port = htons(pSocket->port);
    
    if (inet_aton(pSocket->addr_ip , &(pSocket->sremote->sin_addr)) == 0) {
        die("inet_aton()");
    }
    F_Socket_ResetBuf(pSocket);
    return EXIT_SUCCESS;
}


int F_Socket_Delete(sSockTab ** pSocket)
{
    /* Equivalences avec: sSockTab * pTmp = *pSocket;
            pTmp == *pSockets;
           *pTmp == **pSockets;
           *(pTmp + 1) == pTmp[1] == *(*pSockets + 1)
           (pTmp + 1) == &(pTmp[1]) == (*pSockets + 1)
    */
    /* WARNING: A la fin de cette fonction, le pointeur passé en paramètre n'est plus alloué ! */
    SLOGT(LOG_INFO, "Fermeture et libération de la socket fd %d\n", (*pSocket)->fd);
    close((*pSocket)->fd);
    if ((*pSocket)->rcv_buf != NULL) {
        free ((*pSocket)->rcv_buf);
        (*pSocket)->rcv_buf = NULL;
    }
    if ((*pSocket)->snd_buf != NULL) {
        free ((*pSocket)->snd_buf);
        (*pSocket)->snd_buf = NULL;
    }
    if ((*pSocket)->sin != NULL) {
        free ((*pSocket)->sin);
        (*pSocket)->sin = NULL;
    }
    if ((*pSocket)->sremote != NULL) {
        free ((*pSocket)->sremote);
        (*pSocket)->sremote = NULL;
    }
    
    F_SockQueue_Delete(&((*pSocket)->rcv_queue));
    F_SockQueue_Delete(&((*pSocket)->snd_queue));
    
    if (*pSocket != NULL) {
        free (*pSocket);
        *pSocket = NULL;
    } else {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int F_Socket_DeleteAll(sSockTab *** pSockets, int nb_socks)
{
    int i = -1;
    int ret = -1;
    /* Equivalences avec: sSockTab ** pTmp = *pSockets;
            pTmp == *pSockets;
           *pTmp == **pSockets;
           *(pTmp + 1) == pTmp[1] == *(*pSockets + 1)
           (pTmp + 1) == &(pTmp[1]) == (*pSockets + 1)
    */
    for (i = 0; i < nb_socks; i++) {
        //if ( (ret = F_Socket_Delete(&(pTmp[i]))) != 0)
        if ( (ret = F_Socket_Delete((*pSockets + i))) != 0)
            die("F_Socket_Delete()");
    }

    if (*pSockets != NULL) {
        free (*pSockets);
        *pSockets = NULL;
    } else {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int F_Socket_Lire(sSockTab * pSocket)
{
    int slen = -1;
    //memset((char *)pSocket->sremote, 0, sizeof (*(pSocket->sremote)));
    slen = sizeof (*(pSocket->sremote));
    
    if ((pSocket->rcv_len = recvfrom(pSocket->fd, pSocket->rcv_buf, BUF_SIZE, 0, (struct sockaddr *)pSocket->sremote, (socklen_t *)&slen)) == -1) {
        if (errno == EAGAIN) {
            /*
            couleur(C_RED);
            SLOGT(LOG_INFO, "recvfrom() Timeout. Continue anyway...\n");
            couleur_r;
            */
        }
        else {
            SLOGT(LOG_INFO, "F_Socket_Lire: recvfrom() (%s)\n", strerror(errno));
        }
    }
    else {
        couleur(C_CYAN);
        SLOGT(LOG_INFO, "[%ld] Received %d bytes on socket fd %d, port %d from %s:%d => \n", pSocket->rcv_trame_cpt, pSocket->rcv_len, pSocket->fd, ntohs(pSocket->sin->sin_port),
                                                                                             inet_ntoa(pSocket->sremote->sin_addr), ntohs(pSocket->sremote->sin_port));
        couleur_r;
        NLOG_INFO(log_hexprint((unsigned char *)pSocket->rcv_buf, pSocket->rcv_len));
        pSocket->rcv_trame_cpt++;
    }
    return EXIT_SUCCESS;
}

int F_Socket_Lire_Raw(sSockTab * pSocket, char * buf, int * len)
{
    int slen = -1;
    //memset((char *)pSocket->sremote, 0, sizeof (*(pSocket->sremote)));
    slen = sizeof (*(pSocket->sremote));
    
    if ((*len = recvfrom(pSocket->fd, buf, BUF_SIZE, 0, (struct sockaddr *)pSocket->sremote, (socklen_t *)&slen)) == -1) {
        if (errno == EAGAIN) {
            /*
            couleur(C_RED);
            SLOGT(LOG_INFO, "recvfrom() Timeout. Continue anyway...\n");
            couleur_r;
            */
        }
        else {
            SLOGT(LOG_INFO, "F_Socket_Lire: recvfrom() (%s)\n", strerror(errno));
        }
    }
    else {
        couleur(C_CYAN);
        SLOGT(LOG_INFO, "[%ld] Received %d bytes on socket fd %d, port %d from %s:%d => \n", pSocket->rcv_trame_cpt, *len, pSocket->fd, ntohs(pSocket->sin->sin_port),
                                                                                             inet_ntoa(pSocket->sremote->sin_addr), ntohs(pSocket->sremote->sin_port));
        couleur_r;
        NLOG_INFO(log_hexprint((unsigned char *)buf, *len));
        pSocket->rcv_trame_cpt++;
    }
    return EXIT_SUCCESS;
}

int F_Socket_Ecrire(sSockTab * pSocket, const char * msg, unsigned int len)
{
    int slen = -1;
    int rlen = -1;
    if (len == 0) {
        SLOGT(LOG_INFO, "Longueur de message à envoyer nulle: %d octets\n", len);
        return EXIT_FAILURE;
    }
    if ((int)len >= pSocket->snd_maxsize) {
        SLOGT(LOG_INFO, "Message à envoyer trop long: %d octets (max %d)\n", len, pSocket->snd_maxsize);
        return EXIT_FAILURE;
    }
    /*
    memcpy(pSocket->snd_buf, msg, len);
    pSocket->snd_len = len;
    slen = sizeof (*(pSocket->sremote));
    if ((rlen = sendto(pSocket->fd, pSocket->snd_buf, pSocket->snd_len, 0, (struct sockaddr *)pSocket->sremote, slen)) != pSocket->snd_len) {
        die("sendto()");
    }
    */
    slen = sizeof (*(pSocket->sremote));
    if ((rlen = sendto(pSocket->fd, msg, len, 0, (struct sockaddr *)pSocket->sremote, slen)) != len) {
        die("sendto()");
    }

    couleur(C_MAGENTA);
    SLOGT(LOG_INFO, "[%ld] Send %d on %d bytes on socket fd %d from port %d to %s:%d => \n", pSocket->snd_trame_cpt, rlen, pSocket->snd_len, pSocket->fd, ntohs(pSocket->sin->sin_port),
                                                                                    (char *)inet_ntoa(pSocket->sremote->sin_addr), htons(pSocket->sremote->sin_port));
    couleur_r;
    NLOG_INFO(log_hexprint((unsigned char *)pSocket->snd_buf, rlen));
    pSocket->snd_trame_cpt++;
    return EXIT_SUCCESS;
}


void F_Socket_ResetSndBuf(sSockTab * pSocket)
{
    pSocket->snd_len = -1;
    memset((char *)pSocket->snd_buf, 0, pSocket->snd_maxsize * sizeof (char));
}


void F_Socket_ResetRcvBuf(sSockTab * pSocket)
{
    pSocket->rcv_len = -1;
    memset((char *)pSocket->rcv_buf, 0, pSocket->rcv_maxsize * sizeof (char));
}

void F_Socket_ResetBuf(sSockTab * pSocket)
{
    F_Socket_ResetSndBuf(pSocket);
    F_Socket_ResetRcvBuf(pSocket);
}


void F_Socket_PrintInfos(sSockTab * pSocket)
{
    couleur(C_MAGENTA);
    SLOGT(LOG_INFO, "\nBuffer de réception: ");
    couleur(C_CYAN);
    SLOGT(LOG_INFO, "%s\n", pSocket->rcv_buf);
    couleur(C_MAGENTA);
    SLOGT(LOG_INFO, "Taille: ");
    couleur(C_CYAN);
    SLOGT(LOG_INFO, "%d\n", pSocket->rcv_len);
    couleur(C_MAGENTA);
    SLOGT(LOG_INFO, "Buffer d'émission: ");
    couleur(C_CYAN);
    SLOGT(LOG_INFO, "%s\n", pSocket->snd_buf);
    couleur(C_MAGENTA);
    SLOGT(LOG_INFO, "Taille: ");
    couleur(C_CYAN);
    SLOGT(LOG_INFO, "%d\n\n", pSocket->snd_len);
    couleur_r;
}









int F_SockQueue_Init(sSockFifo ** ppFifo, unsigned int max_size, const char * fifo_name)
{
    sSockFifo * pNew = NULL;
    pNew = (sSockFifo *)calloc(1U, sizeof (sSockFifo));
    
    if (pNew != NULL)
    {
        pNew->max_size = max_size;
        F_SockQueue_SetName(pNew, fifo_name);
        pNew->sem = calloc(1U, sizeof (sem_t));
    }
    pthread_mutex_init(&(pNew->mutex_fifo), NULL);
    sem_init(pNew->sem, 0, 0);
    *ppFifo = pNew;
    return EXIT_SUCCESS;
}

int F_SockQueue_SetName(sSockFifo * pFifo, const char * fifo_name)
{
    if ((pFifo != NULL) && (fifo_name != NULL)) {
        pthread_mutex_lock(&(pFifo->mutex_fifo));
        if (pFifo->name != NULL) {
            free(pFifo->name);
        }
        pFifo->name = strdup(fifo_name);
        pthread_mutex_unlock(&(pFifo->mutex_fifo));
        if (pFifo->name != NULL) {
            return EXIT_SUCCESS;
        }
    }
    return EXIT_FAILURE;
}

const char * F_SockQueue_GetName(sSockFifo * pFifo)
{
    char * tmp = NULL;

    pthread_mutex_lock(&(pFifo->mutex_fifo));
    tmp = pFifo->name;
    pthread_mutex_unlock(&(pFifo->mutex_fifo));
    return tmp;
}

int F_SockQueue_Delete(sSockFifo ** ppFifo)
{
    if (*ppFifo != NULL) {
        pthread_mutex_lock(&((*ppFifo)->mutex_fifo));
        struct tSockFifo_Elem * pTmp = (*ppFifo)->pHead;
        while (pTmp != NULL) {
            struct tSockFifo_Elem * pDel = pTmp;
            pTmp = pTmp->pNext;
            free(pDel);
        }
        pthread_mutex_unlock(&((*ppFifo)->mutex_fifo));
        sem_destroy((*ppFifo)->sem);
        pthread_mutex_destroy(&((*ppFifo)->mutex_fifo));
        free(*ppFifo);
        *ppFifo = NULL;
    }
    return EXIT_SUCCESS;
}


int F_SockQueue_Add(sSockFifo ** ppFifo, char * data, unsigned int data_size)
{
    unsigned int len = 0;

    if (*ppFifo != NULL) {     /* On vérifie si notre liste a été allouée */
    
        if ((*ppFifo)->slength >= (*ppFifo)->max_size) {
            printf("F_SockQueue_Add: ERREUR D'AJOUT A LA FIFO\n");
            return -1;
        }
        struct tSockFifo_Elem * pNewElem = malloc(sizeof *pNewElem); /* Création d'un nouveau node */
        if (pNewElem != NULL) {    /* On vérifie si le malloc n'a pas échoué */
            pNewElem->pNext = NULL;   /* On fait pointer pNext vers NULL */
            if ((data_size > 0) & (data_size <= BUF_SIZE))
                len = data_size;
            else
                len = BUF_SIZE;
            pNewElem->data = (char *)malloc(len * sizeof (char));
            memcpy(pNewElem->data, data, len * sizeof (char));
            pNewElem->data_size = len;
            pthread_mutex_lock(&((*ppFifo)->mutex_fifo));
            if ((*ppFifo)->pTail == NULL) {
                /* Cas où notre liste est vide (pointeur vers fin de liste à  NULL) */
                (*ppFifo)->pHead = pNewElem; /* On fait pointer la tête de liste vers le nouvel élément */
            } else {
                /* Cas où des éléments sont déjà présents dans la liste */
                (*ppFifo)->pTail->pNext = pNewElem; /* On relie le dernier élément de la liste vers notre nouvel élément (début du chaînage) */
            }
            (*ppFifo)->pTail = pNewElem; /* On fait pointer la fin de liste vers notre nouvel élément (fin du chaînage: 3 étapes) */
            sem_post((*ppFifo)->sem);
            ++((*ppFifo)->slength); /* Incrémentation de la taille de la liste */
            pthread_mutex_unlock(&((*ppFifo)->mutex_fifo));
        }
    }
    return ((*ppFifo)->slength); 
}


int F_SockQueue_Remove(sSockFifo ** ppFifo, int * get_id, char * get_data, unsigned int * get_size)
{
    int ret = -1;
    struct timespec semw;
    
    semw.tv_sec = 0;
    semw.tv_nsec = 100000000; /* 100 ms */

    if (*ppFifo != NULL) {
        #ifdef _SOCK_TIMEDWAIT_
        if(sem_timedwait((*ppFifo)->sem, &semw) < 0) {
            ret = errno;
            return ret;
        }
        #else
        if (sem_wait((*ppFifo)->sem) < 0) {
            return errno;
        }
        #endif

        struct tSockFifo_Elem * pTmp = (*ppFifo)->pHead;
        
        if (pTmp != NULL) {
            *get_size = 0;
            /* Valeur à retourner */
            if (get_data != NULL) {
                memcpy(get_data, pTmp->data, pTmp->data_size * sizeof (char));
                *get_size = pTmp->data_size;
            }
            *get_id = 0;
            free(pTmp->data);
            pTmp->data = NULL;
            pthread_mutex_lock(&((*ppFifo)->mutex_fifo));
            (*ppFifo)->pHead = pTmp->pNext;
            if (pTmp->pNext == NULL) {
                (*ppFifo)->pTail = NULL;
            }
            free(pTmp);
            --((*ppFifo)->slength);
            pthread_mutex_unlock(&((*ppFifo)->mutex_fifo));
        }
    }
    return ret;
}


    
void F_SockQueue_Clear(sSockFifo ** ppFifo)
{
    int msg_id = -1;
    unsigned int msg_len = 0;
    
    if (*ppFifo != NULL) {
        while ((*ppFifo)->pHead != NULL) {
            F_SockQueue_Remove(ppFifo, &msg_id, NULL, &msg_len);
        }
    }
}



int F_SockQueue_GetNbElem(sSockFifo * pFifo)
{
    int i = 0;
    if (pFifo != NULL) {
        pthread_mutex_lock(&(pFifo->mutex_fifo));
        i = (int)pFifo->slength;
        pthread_mutex_unlock(&(pFifo->mutex_fifo));
    }
    return i;
}

void F_SockQueue_Print(sSockFifo * pFifo)
{
    int i = 0;
    int nb_elem = 0;
    if (pFifo != NULL) {
        pthread_mutex_lock(&(pFifo->mutex_fifo));
        nb_elem = F_SockQueue_GetNbElem(pFifo);
        printf("Nombre d'éléments dans la liste = %d\n", nb_elem);
        struct tSockFifo_Elem * pTmp = pFifo->pHead;
        while (pTmp != NULL) {
            printf("Element %d: id %d, data %s, taille %u\n", i, pTmp->id, pTmp->data, pTmp->data_size);
            fflush(stdout);
            i++;
            pTmp = pTmp->pNext;
        }
        pthread_mutex_unlock(&(pFifo->mutex_fifo));
    }

}

