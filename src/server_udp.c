/*!
 * \file server_udp.c
 * \brief Gestion de la partie serveur UDP
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 03/06/2014
 *
 * Test d'un serveur UDP
 *
 * S:\\be\\Sncf\\PTE FO IFOTEC\\E_Développement matériel\\Spécifications\\Diffussion\n
 * S:\\be\\Sncf\\PTE FO IFOTEC\\5tons.pdf\n
 * S:\\be\\Sncf\\PTE FO IFOTEC\\D_Développement logiciel\\Programmation\\Prog Poste De Tete\n
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"
#include "tlog.h"
#include "socks_mgt.h"
#include "code5t.h"
#include "code5t_proc.h"
#include "heartbeat.h"
#include "config.h"
#include "main.h"
#include "server_udp.h"


/*!
 * \fn static void cleanup_handler_UDPServer(void *arg)
 * \brief Cleanup handler pour le thread F_Th_UDPServer
 * \param arg: pointeur sur structure sMain
 */
static void cleanup_handler_UDPServer(void * arg)
{
    sMain * pMain = (sMain *)arg;
    SLOGT(LOG_DEBUG, "F_Th_UDPServer clean-up handler (%d)\n", pMain->dumb);
}


/*!
 * \fn int F_ServUDP_Listening(sMain * pMain)
 * \brief Mise en écoute du serveur UDP sur ses sockets
 * \param[in,out] pMain: pointeurs sur la structure principale du programme
 * \return 0 si OK
 *
 * Cette fonction est appellée périodiquement par le thread principal du serveur (F_Th_UDPServer).\n
 * Elle se met en écoute sur les 5 sockets serveur (1 data + 4 audio).
 * Dès que des données sont reçues sur une des sockets, la fonction ajoute les données
 * dans la file associée à la socket. Ces données sont dépilées en temps voulu par les threads
 * F_Th_UDPServer_Data et/ou F_Th_UDPServer_Audio
 */
static int F_ServUDP_Listening(sMain * pMain)
{
    int ret = -1;
    int i = -1;
    int nb_snd_elem = -1;
    int msg_id = -1;
    int fd_max = 0;
    unsigned int msg_len = 0;
    fd_set Masque_R;
    //, Masque_W, Masque_E;
    struct timeval ltimeout;
    char * msg_to_snd = (char *)malloc(BUF_SIZE * sizeof (char));
    
    sConfigMain * pConf  = pMain->pConfig;
    sSockTab ** pSockets = pMain->pSockets;

    /* Initialisation des sets contenant les FD des sockets de réception */
    FD_ZERO(&Masque_R);
    //FD_ZERO(&Masque_E);
    //FD_ZERO(&Masque_W);
    
    #if _OLD_SCHED_
    for (i = 0; i < pConf->pServ->nb_sockets; i++) {
    #endif
    for (i = 0; i < 1; i++) {
        FD_SET(pSockets[i]->fd, &Masque_R);
        if (pSockets[i]->fd > fd_max)
            fd_max = pSockets[i]->fd;
    }
    fd_max++;   /* fd_max + 1, pour select()*/
    
    /* Reinitialisation du timeout pour select */
    ltimeout.tv_sec = pConf->pServ->l_timeout.tv_sec;
    ltimeout.tv_usec = pConf->pServ->l_timeout.tv_usec;
    
    /* En attente de données sur le set de file descriptor des sockets */
    pthread_testcancel();
    ret = select(fd_max, &Masque_R , NULL , NULL , &(ltimeout));
    switch (ret) {
        case 0:     /* Timeout */
            /* couleur(C_RED); SLOGT(LOG_DEBUG, "Timeout\n");  couleur_r;
            */
            break;
            
        case -1:    /* Erreur */
            FD_ZERO(&Masque_R);
            //FD_ZERO(&Masque_E);
            //FD_ZERO(&Masque_W);
            die("select()");
            break;
    } /* end switch (ret) */
    
    /* Parcours les sockets du set et lecture du contenu */
    for (i = 0; i < pConf->pServ->nb_sockets; i++) {
        if (FD_ISSET(pSockets[i]->fd , &Masque_R)) {
            F_Socket_ResetRcvBuf(pSockets[i]);
            if ((ret = F_Socket_Lire(pSockets[i])) != EXIT_SUCCESS)
                die("F_Socket_Lire()");
            
            /* Ajout des données lues à la fifo de réception */
            if ( F_SockQueue_Add(&(pSockets[i]->rcv_queue), pSockets[i]->rcv_buf, pSockets[i]->rcv_len) < 0)
                die("F_ServUDP_Listening: F_SockQueue_Add()");
        } /* Fin if (FD_ISSET) */
    } /* Fin for */

    /**********************************/
    /* Uniquement pour la socket DATA */
    /**********************************/
    /* Défile les données depuis la fifo d'emission */
   //nb_snd_elem = F_SockQueue_GetNbElem(pSockets[SOCK_DATA]->snd_queue);
   //while (nb_snd_elem > 0) {
   //    /* On défile l' élément*/
   //    memset(msg_to_snd, 0, BUF_SIZE * sizeof (char));
   //    F_SockQueue_Remove(&(pSockets[SOCK_DATA]->snd_queue), &msg_id, msg_to_snd, &msg_len);
   //    F_Socket_ResetSndBuf(pSockets[SOCK_DATA]);
   //    /* Envoie la donnée dépilée sur la socket */
   //    if ((ret = F_Socket_Ecrire(pSockets[SOCK_DATA], msg_to_snd, msg_len)) != EXIT_SUCCESS)
   //        die("F_Socket_Ecrire()");
   //    pthread_testcancel();
   //    nb_snd_elem = F_SockQueue_GetNbElem(pSockets[SOCK_DATA]->snd_queue);
   //} /* fin while (nb_elem) */
    pthread_testcancel();
    
    free(msg_to_snd);
    return EXIT_SUCCESS;
}


void * F_Th_UDPServer(void * arg)
{
    sMain * pMain = (sMain *)arg;
    struct timespec th_sleep;
    th_sleep.tv_sec = 0;
    th_sleep.tv_nsec = DEFAULT_THREAD_SLEEP_NS;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_cleanup_push(cleanup_handler_UDPServer, (void *)pMain);
    SLOGT(LOG_DEBUG, "F_Th_UDPServer: Démarrage thread \n");
    //couleur(C_YELLOW); SLOGT(LOG_INFO, "Serveur PID %d: Waiting for data...\n", getpid()); couleur_r; fflush(stdout);
    /* Démarrage de la boucle du thread */
    while(1) {
        pthread_testcancel();
        F_ServUDP_Listening(pMain);
        nanosleep(&th_sleep, NULL);
    } /* fin while(1) */
    SLOGT(LOG_DEBUG, "Fin thread F_Th_UDPServer\n");
    pthread_cleanup_pop(0);
    pthread_exit(0);
}

