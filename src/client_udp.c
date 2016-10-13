/*!
 * \file client_udp.c
 * \brief Gestion de la partie cliente UDP
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 01/07/2014
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "common.h"
#include "tlog.h"
#include "socks_mgt.h"
#include "code5t.h"
#include "code5t_proc.h"
#include "heartbeat.h"
#include "config.h"
#include "main.h"
#include "client_udp.h"


/*!
 * \fn static void cleanup_handler_data(void * arg)
 * \brief Cleanup handler pour le thread F_Th_UDPClientData
 * \param arg: pointeur sur structure sMain
 */
static void cleanup_handler_data(void * arg)
{
    sMain * pMain = (sMain *)arg;
    SLOGT(LOG_DEBUG, "F_Th_UDPClientData clean-up handler (%d)\n", pMain->dumb);
}

/*!
 * \fn static void cleanup_handler_audio(void * arg)
 * \brief Cleanup handler pour le thread F_Th_UDPClientAudio
 * \param arg: pointeur sur structure sThArgCliAudio
 */
static void cleanup_handler_audio(void * arg)
{   
    sThArgCliAudio * pArg = (sThArgCliAudio *)arg;
    SLOGT(LOG_DEBUG, "F_Th_UDPClientAudio no %d clean-up handler\n", pArg->th_num);
    free(pArg);
}

void * F_Th_UDPClient_Data(void * arg)
{
    sMain * pMain = (sMain *)arg;
    sSockTab * pSocketData = pMain->pSocketsCli[SOCK_DATA];
    int nb_elem = -1;
    int ret = -1;
    int msg_id = -1;
    unsigned int msg_len = 0;
    char * msg_to_snd = (char *)malloc(BUF_SIZE * sizeof (char));
    struct timespec th_sleep;
    th_sleep.tv_sec = 0;
    th_sleep.tv_nsec = DEFAULT_THREAD_SLEEP_NS;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_cleanup_push(cleanup_handler_data, (void *)pMain);
    SLOGT(LOG_DEBUG, "F_Th_UDPClient_Data: Démarrage thread\n"); fflush(stdout);
    /* Démarrage de la boucle du thread */
    while(1) {
        nb_elem = F_SockQueue_GetNbElem(pSocketData->snd_queue);
        while (nb_elem > 0) {
            /* On défile l'élément */
            memset(msg_to_snd, 0, BUF_SIZE * sizeof (char));
            F_SockQueue_Remove(&(pSocketData->snd_queue), &msg_id, msg_to_snd, &msg_len);
            F_Socket_ResetSndBuf(pSocketData);
            /* On envoi l'élément défilé */
            F_Socket_Ecrire(pSocketData, msg_to_snd, msg_len);
            nb_elem = F_SockQueue_GetNbElem(pSocketData->snd_queue);
        }
        
        /* couleur(C_YELLOW); SLOGT(LOG_INFO, "F_Th_UDPClient_Data: Client PID %d: Waiting for reply data...\n", getpid()); couleur_r;
        */
        F_Socket_ResetRcvBuf(pSocketData);
        if ((ret = F_Socket_Lire(pSocketData)) != EXIT_SUCCESS)  die("F_Th_UDPClient_Data: F_Socket_Lire()");
        
        if(pSocketData->rcv_len > 0) {
            F_SockQueue_Add(&(pSocketData->rcv_queue), pSocketData->rcv_buf, pSocketData->rcv_len);
        } else {
            F_SocketCli_Reset(pSocketData);
        }

        pthread_testcancel();
        //nanosleep(&th_sleep, NULL);
    } /* fin while(1) */
    
    /* Fin de la boucle du thread */
    free(msg_to_snd);
    SLOGT(LOG_DEBUG, "F_Th_UDPClient_Data: Fin thread\n");
    pthread_cleanup_pop(0);
    pthread_exit(0);
}

void * F_Th_UDPClient_Audio(void * arg)
{
    sThArgCliAudio * pArg = (sThArgCliAudio *)arg;
    sMain * pMain = (sMain *)pArg->pMain;
    int circuit = pArg->th_num + 1;
    sSockTab * pSocketAudio = pMain->pSocketsCli[circuit];
    
    int nb_elem = -1;
    int msg_id = -1;
    unsigned int msg_len = 0;
    char * msg_to_snd = (char *)malloc(BUF_SIZE * sizeof (char));
    struct timespec th_sleep;
    th_sleep.tv_sec = 0;
    th_sleep.tv_nsec = DEFAULT_THREAD_SLEEP_NS;
    
    //usleep(DEFAULT_THREAD_SLEEP_US + (2 * pArg->th_num * DEFAULT_THREAD_SLEEP_US));
    
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_cleanup_push(cleanup_handler_audio, (void *)pArg);
    SLOGT(LOG_DEBUG, "F_Th_UDPClient_Audio: Démarrage thread num %d\n", pArg->th_num);
    /* Début de la boucle du thread */
    while (1) {
        nb_elem = F_SockQueue_GetNbElem(pSocketAudio->snd_queue);
        while (nb_elem > 0) {
            /* On défile l'élément */
            memset(msg_to_snd, 0, BUF_SIZE * sizeof (char));
            F_SockQueue_Remove(&(pSocketAudio->snd_queue), &msg_id, msg_to_snd, &msg_len);
            // sem_timedwait()
            // if (!timeout) {
            F_Socket_ResetSndBuf(pSocketAudio);
            /* On envoi l'élément défilé */
            F_Socket_Ecrire(pSocketAudio, msg_to_snd, msg_len);
            nb_elem = F_SockQueue_GetNbElem(pSocketAudio->snd_queue);
            // }
        }

        /* Une fois les éléments dépilés et envoyé, rien n'est attendu en retour.
           Tout ce qui est reçu l'est par l'intermédiaire de la socket serveur correspondante
        */
         
        //memset(msg_to_snd, 0, BUF_SIZE * sizeof (char));
        //if (F_SockQueue_Remove(&(pSocketAudio->snd_queue), &msg_id, msg_to_snd, &msg_len) != ETIMEDOUT) {
        //    F_Socket_ResetSndBuf(pSocketAudio);
        //    /* On envoi l'élément défilé */
        //    F_Socket_Ecrire(pSocketAudio, msg_to_snd, msg_len);
        //    nb_elem = F_SockQueue_GetNbElem(pSocketAudio->snd_queue);
        //}
        //
        //pthread_testcancel();

        nanosleep(&th_sleep, NULL);

    } /* fin while (1) */
    
    /* Fin de la boucle du thread */
    free(msg_to_snd);
    SLOGT(LOG_DEBUG, "F_Th_UDPClient_Audio: Fin thread num %d\n", pArg->th_num);
    pthread_cleanup_pop(0);
    pthread_exit(0);
}
