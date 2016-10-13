/*!
 * \file server_tcp.c
 * \brief Gestion de la partie serveur TCP (Communication avec Nc_core)
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 08/09/2014
 *
 * Serveur TCP
 *
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"
#include "tlog.h"
#include "socks_mgt.h"
#include "socks_tcp_mgt.h"
#include "code5t.h"
#include "code5t_proc.h"
#include "heartbeat.h"
#include "config.h"
#include "asterisk_logger.h"
#include "Siema_NC_CORE_Asterisk_Utils.h"
#include "main.h"
#include "server_tcp.h"

static void cleanup_handler_TCPServer(void * arg)
{
    sMain * pMain = (sMain *)arg;
    SLOGT(LOG_DEBUG, "F_Th_TCPServer clean-up handler (%d)\n", pMain->dumb);
}

void * F_Th_TCPServer(void * arg)
{
    sMain * pMain = (sMain *)arg;
    sSockTab * pSocketServeur = NULL;
    sConfigServerTCP * pConf = pMain->pConfig->pServTCP;

    fd_set master, read_fds;
    int fd_max  = 0;
    int newfd   = -1;
    int ret     = -1;
    int i       = -1;
    int nbytes  = -1;
    char * msg_to_snd = (char *)malloc(BUF_SIZE * sizeof (char));
    char * rcv_buffer = (char *)malloc(BUF_SIZE * sizeof (char));
    struct timespec th_sleep;
    th_sleep.tv_sec = 0;
    th_sleep.tv_nsec = DEFAULT_THREAD_SLEEP_NS;
    
    /* Init des variables gérant les messages échangés avec Nc_core*/
    struct sockaddr_in clientaddr;
    int addrlen = -1;
    t_message_3num_nchaine * nc_msg = NULL;
    t_chaine * ch_tmp = NULL;
    
    /* Init des variables gerant le code 5-tons */
    sCode5T *  pCodeRcvFromNcCore = NULL;
    F_Code5T_Init(&pCodeRcvFromNcCore);
    
    
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_cleanup_push(cleanup_handler_TCPServer, (void *)pMain);
    SLOGT(LOG_DEBUG, "F_Th_TCPServer: Démarrage thread \n");
    
    /* Creation de la socket TCP cliente (communication avec Nc_core) */
    if ( (ret = F_TCP_Socket_Create(&(pSocketServeur), pConf->port, pConf->nb_max_clients)) < 0)
    //if ( (ret = F_TCP_Socket_Create(&(pSocketServeur), 55005, 1)) < 0)
        die("F_TCP_Socket_Create()");
    SLOGT(LOG_INFO, "Création socket TCP serveur fd %d, bind sur le port d'écoute %u\n", pSocketServeur->fd, pSocketServeur->port);
    
    FD_ZERO(&master);
    /* Démarrage de la boucle du thread */
    while(1) {
        pthread_testcancel();
        /* Initialisation des sets contenant les FD des sockets de réception */
        FD_ZERO(&read_fds);
        /* Ajout de la socket serveur d'écoute au set master*/
        FD_SET(pSocketServeur->fd, &master);
        /* Recherche le numéro de socket le plus élévé */
        if (pSocketServeur->fd >= fd_max) {
            fd_max = pSocketServeur->fd;
        }
        /* Effectue une copie du set master pour le select */
        read_fds = master;
        ret = select(fd_max + 1, &read_fds , NULL , NULL , NULL);
        switch (ret) {
            case 0:     /* Timeout */
                /* couleur(C_RED); SLOGT(LOG_DEBUG, "Timeout\n");  couleur_r;
                */
                break;
                
            case -1:    /* Erreur */
                FD_ZERO(&read_fds);
                FD_ZERO(&master);
                die("select()");
                break;
        } /* end switch (ret) */
        
        /* Parcours les sockets du set */
        for (i = 0; i <= fd_max; i++) {
            /* Si des données sont disponible sur la socket i */
            if(FD_ISSET(i, &read_fds)) {
                /* Si cette socket est la socket d'écoute du serveur */
                if (i == pSocketServeur->fd) {
                    /* On accepte la nouvelle connexion */
                    addrlen = sizeof(clientaddr);
                    if ((newfd = accept(pSocketServeur->fd, (struct sockaddr *)&clientaddr, (socklen_t *)&addrlen)) != SOCKET_ERROR) {
                        /* On ajoute la nouvelle connexion au set master */
                        FD_SET(newfd, &master);
                        /* On recalcule le nouveau numéro de socket le plus élevé pour le prochain select */
                        if (newfd >= fd_max) {
                            fd_max = newfd;
                        }
                        SLOGT(LOG_INFO, "F_Th_TCPServer: Serveur on socket %d, listening on port %d had accepted a new connection from %s:%d on socket fd %d\n",
                            pSocketServeur->fd, ntohs(pSocketServeur->sin->sin_port),
                            (char *)inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), newfd);
                    } else {
                        SLOGT(LOG_ERR, "F_Th_TCPServer: Probleme avec accept()\n"); }
                } else {
                    /* Cette socket est une socket déjà connectée à un client */
                    memset(rcv_buffer, 0, BUF_SIZE * sizeof (char));
                    if((nbytes = recv(i, rcv_buffer, BUF_SIZE * sizeof (char), 0)) <= 0) {
                        /* got error or connection closed by client */
                        if(nbytes == 0) {
                            /* connection closed */
                            SLOGT(LOG_INFO, "F_Th_TCPServer: socket fd %d. La socket distante à terminé la connexion. On ferme notre socket\n", i);
                            close(i);
                            FD_CLR(i, &master);
                        }
                    } else {
                        /* Des données ont été reçues sur la socket */
                        couleur(C_CYAN);
                        SLOGT(LOG_INFO, "[TCP] Received %d bytes on socket fd %d, port %d from %s:%d => \n", nbytes, i, 
                                                                                    ntohs(pSocketServeur->sin->sin_port),
                                                                                    (char *)inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
                        couleur_r;
                        NLOG_INFO(log_hexprint((unsigned char *)rcv_buffer, nbytes));
                        
                        /* On converti les données reçue en message 3num_n_chaine */
                        nc_msg = SiemaDeserialiserMessage((unsigned char *)rcv_buffer, nbytes);
                        siema_ast_text_color(color_bgreen, color_brown); SiemaAfficherMessage(nc_msg);
                        
                        if (nc_msg->sous_type == MSG_EMETTRE_5T) {
                            /* nc_msg->num1 contient le numéro de circuit */
                            /* Le message doit contenir 1 seule chaine constituée du code 5-tons brut (incluant l'offset de la pyramide 5-tons) */
                            if (nc_msg->nb_chaines == 1) {
                                ch_tmp = nc_msg->liste_chaine;
                                if (ch_tmp->longueur == CODE_BIBL_LEN) {
                                    F_Code5T_Reset(pCodeRcvFromNcCore);
                                    F_Code5T_Set_BiblCode(pCodeRcvFromNcCore, (unsigned char *)ch_tmp->chaine, nc_msg->num1, FROM_NC_CORE);
                                    couleur(C_YELLOW); SLOGT(LOG_INFO, "Code 5-tons reçu depuis Nc_core:\n");  couleur_r;
                                    NLOG_INFO(F_Code5T_Infos(pCodeRcvFromNcCore));
                                    /* On ajoute les données à la FIFO de reception de notre socket */
                                    F_SockQueue_Add(&(pMain->nc_serv_rcv_queue), (char *)pCodeRcvFromNcCore->cstr, pCodeRcvFromNcCore->cstr_len);
                                } else {
                                    SLOGT(LOG_ERR, "F_Th_TCPServer: Mauvaise longueur de chaine BIBL (%d / %d)\n", ch_tmp->longueur, CODE_BIBL_LEN);
                                } /* fin if (ch_tmp->longueur == CODE_BIBL_LEN) */
                            } else {
                                SLOGT(LOG_ERR, "F_Th_TCPServer: Mauvais nombre de chaines contenues dans le message (Contient %d chaine(s) / Attendu 1 chaine)\n", nc_msg->nb_chaines);
                            } /* fin if (nc_msg->nb_chaines == 1) */
                        } else {
                            SLOGT(LOG_ERR, "F_Th_TCPServer: Message reçu de Nc_core non reconnu. Recu %d / Attendu %d\n", nc_msg->sous_type, MSG_EMETTRE_5T);
                        } /* fin if (nc_msg->sous_type == MSG_EMETTRE_5T) */
                        SiemaDeleteMessage(nc_msg);
                        
                        /* OPTIONNEL: Renvoi des données au client */
                        /* Pour le moment, on se contente de renvoyer une copie des données reçues */
                        /*
                        memset(msg_to_snd, 0, BUF_SIZE * sizeof (char));
                        memcpy(msg_to_snd, rcv_buffer, nbytes);
                        if((rlen = send(i, msg_to_snd, nbytes, 0)) == -1)
                            printf("F_Th_TCPServer: send back error\n");
                        
                        couleur(C_MAGENTA); printf("[NA] Send %d on %d bytes on socket fd %d to %s:%d => \n", rlen, nbytes, i, 
                                                            (char *)inet_ntoa(clientaddr.sin_addr), htons(clientaddr.sin_port));    
                        couleur_r;
                        NLOG_INFO(log_hexprint((unsigned char *)msg_to_snd, rlen));
                        */
                        
                        /* On garde la socket ouverte de notre coté (serveur). On attend simplement que ce soit le client distant qui ferme la socket */
                    }
                    
                }
            } /* Fin if FD_ISSET */
        } /* Fin for */
        pthread_testcancel();
        nanosleep(&th_sleep, NULL);

    } /* fin while(1) */
    
    free(msg_to_snd);
    free(rcv_buffer);
    F_Code5T_Free(&pCodeRcvFromNcCore);
    SLOGT(LOG_DEBUG, "Fin thread F_Th_UDPServer\n");
    pthread_cleanup_pop(0);
    pthread_exit(0);
}
