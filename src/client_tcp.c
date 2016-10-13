/*!
 * \file client_tcp.c
 * \brief Gestion de la partie cliente TCP  (Communication avec Nc_core)
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 08/09/2014
 *
 * Client TCP
 *
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
#include "socks_tcp_mgt.h"
#include "code5t.h"
#include "code5t_proc.h"
#include "heartbeat.h"
#include "config.h"
#include "asterisk_logger.h"
#include "Siema_NC_CORE_Asterisk_Utils.h"
#include "main.h"
#include "client_tcp.h"



static void cleanup_handler_TCPClient(void * arg)
{   
    sMain * pMain = (sMain *)arg;
    SLOGT(LOG_DEBUG, "F_Th_TCPClient clean-up handler (%d)\n", pMain->dumb);
}

void * F_Th_TCPClient(void * arg)
{
    sMain * pMain = (sMain *)arg;
    sConfigClientTCP * pConf = pMain->pConfig->pCliTCP;
    sSockTab * pSocketCli = NULL;
    int ret = 0;
    int nb_elem = -1;
    int msg_id = -1;
    int c_retry_count = 0;
    unsigned int msg_len = 0;
    struct timeval rcv_to, snd_to;
    struct timespec th_sleep;
    th_sleep.tv_sec = 0;
    th_sleep.tv_nsec = DEFAULT_THREAD_SLEEP_NS;
    
    size_t zap_str_len = 11 * sizeof (char);
    char * zap_str = (char *)malloc(zap_str_len);
    
    
    char * msg_to_snd = (char *)malloc(BUF_SIZE * sizeof (char));
    char * rcv_buffer = (char *)malloc(BUF_SIZE * sizeof (char));
    t_message_3num_nchaine * nc_msg = NULL;
    
    /* Init des variables gerant le code 5-tons */
    sCode5T *  pCodeSndToNcCore = NULL;
    F_Code5T_Init(&pCodeSndToNcCore);
    
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_cleanup_push(cleanup_handler_TCPClient, (void *)pMain);
    SLOGT(LOG_DEBUG, "F_Th_TCPClient: Démarrage thread\n"); 
    /* Démarrage de la boucle du thread */
    while(1) {
    
        nb_elem = F_SockQueue_GetNbElem(pMain->nc_cli_snd_queue);
        while (nb_elem > 0) {
            /* On commence par une tentative de connexion à la socket serveur. Si cette dernière échoue, aucun élément 
               ne sera défilé de la file et une nouvelle tentative pourra être entreprise */
            
            /* On créé la socket cliente et on la connecte au serveur TCP (nc_core) */
            rcv_to = pConf->reply_timeout;
            snd_to = pConf->send_timeout;
           
            if ( (ret = F_TCP_SocketCli_Create(&(pSocketCli), (char*)pConf->remote_server_ip, pConf->remote_server_port, rcv_to, snd_to)) < 0)
                die("F_TCP_SocketCli_Create()");
            SLOGT(LOG_INFO, "F_Th_TCPClient: Création socket TCP cliente fd %d\n", pSocketCli->fd);
            if (connect(pSocketCli->fd,(struct sockaddr *)pSocketCli->sremote, sizeof(*(pSocketCli->sremote))) == SOCKET_ERROR) {
                if ((errno == EINPROGRESS) || (errno == ETIMEDOUT) || (errno == ECONNREFUSED)) {
                    SLOGT(LOG_ERR, "F_Th_TCPClient: timeout de connexion à la socket serveur %s:%d!\n", (char *)inet_ntoa(pSocketCli->sremote->sin_addr), htons(pSocketCli->sremote->sin_port));
                    F_Socket_Delete(&(pSocketCli));
                    break;
                } else {  die("F_Th_TCPClient: connect()");  }
            } /* fin if (connect...) */
            /* Reset des tentatives de retry */
            c_retry_count = 0;
            
            /* On défile l'élément dans le buffer msg_to_snd */
            memset(msg_to_snd, 0, BUF_SIZE * sizeof (char));
            F_SockQueue_Remove(&(pMain->nc_cli_snd_queue), &msg_id, msg_to_snd, &msg_len);
            
            /* On transforme le message dépilé en élément 5-tons */
            F_Code5T_Reset(pCodeSndToNcCore);
            F_Code5T_Set_Str(pCodeSndToNcCore, (unsigned char *)msg_to_snd, TO_NC_CORE);
            F_Code5T_PDT2BIBL(pCodeSndToNcCore);
            couleur(C_YELLOW); SLOGT(LOG_INFO, "Code 5-tons à envoyer à Nc_core:\n"); couleur_r; 
            NLOG_INFO(F_Code5T_Infos(pCodeSndToNcCore));
            
            /* On transformer l'élement 5-tons en message de type MSG3num_n_chaine */
            nc_msg = SiemaNewMessage();
            nc_msg->sous_type = MSG_REC_PTO_5T;
            nc_msg->num1 = pCodeSndToNcCore->circuit_bibl;
            nc_msg->num2 = pCodeSndToNcCore->action - 48;       // Ouhh c'est pas beau... mais faut aller vite !
            nc_msg->num3 = 0;
            memset(zap_str, 0, zap_str_len);
            snprintf(zap_str, zap_str_len, "Zap/10%u-1", pCodeSndToNcCore->circuit_bibl);
            SiemaAjoutChaineMessage(nc_msg, zap_str, zap_str_len);
            SiemaAjoutChaineMessage(nc_msg, (char *)pCodeSndToNcCore->code_bibl_raw, CODE_BIBL_LEN);
            siema_ast_text_color(color_bblue, color_brown); SiemaAfficherMessage(nc_msg);
            
            /* On sérialise le message t_message_3num_nchaine.
               On réutilise la variable msg_to_snd pour contenir la chaine sérialisée. La taille de la chaine est retournée dans msg_len*/
            memset(msg_to_snd, 0, BUF_SIZE * sizeof (char));
            msg_len = SiemaSerialiserMessage(nc_msg, (unsigned char *)msg_to_snd, BUF_SIZE);
            SiemaDeleteMessage(nc_msg);
            
            /* On envoi le message t_message_3num_nchaine à nc_core */
            F_Socket_ResetSndBuf(pSocketCli);
            if((ret = send(pSocketCli->fd, msg_to_snd, msg_len, 0)) < 0) {
                SLOGT(LOG_ERR, "F_Th_TCPClient: send() error %s\n", strerror(errno));
            }
            couleur(C_MAGENTA);
            SLOGT(LOG_INFO, "[TCP] Send %d on %d bytes on socket fd %d from port %d to %s:%d => \n", ret, msg_len, pSocketCli->fd, ntohs(pSocketCli->sin->sin_port), 
                                                                      (char *)inet_ntoa(pSocketCli->sremote->sin_addr),
                                                                      htons(pSocketCli->sremote->sin_port));
            couleur_r;
            NLOG_INFO(log_hexprint((unsigned char *)msg_to_snd, ret));
            

            /* On attent la réponse du serveur */
            ret = 1;
            while (ret > 0) {
                memset(rcv_buffer, 0, BUF_SIZE * sizeof (char));
                ret = recv(pSocketCli->fd, rcv_buffer, BUF_SIZE * sizeof (char), 0);
                if(ret < 0) {
                    /* Aucune réponse ne nous parvient, on shutdown la connexion */
                    SLOGT(LOG_WARNING, "F_Th_TCPClient: recv() error %s\n", strerror(errno));
                    SLOGT(LOG_INFO, "F_Th_TCPClient: Client local termine la connexion\n");
                    shutdown(pSocketCli->fd, 0);
                } else if (ret == 0) {
                    /* Le serveur distant à clos la connexion */
                    SLOGT(LOG_INFO, "F_Th_TCPClient: Serveur distant a terminé la connexion\n");
                    F_Socket_Delete(&(pSocketCli));
                } else {
                    /* Le serveur à renvoyé des données */
                    SLOGT(LOG_INFO, "F_Th_TCPClient: Reçu %d octets\n", ret);
                }
            } /* Fin while */
            
            /* Mise à jour du nombre d'éléments restant dans la liste */
            nb_elem = F_SockQueue_GetNbElem(pMain->nc_cli_snd_queue);

            pthread_testcancel();
        } /* fin while (nb_elem > 0) */
        
        /* Si à la fin de la boucle, il reste des éléments dans la file, c'est qu'il y a eu un problème avec la socket. On temporise avant un nouvel essai */
        if (nb_elem > 0) {
            if (c_retry_count >= pConf->max_retry - 1 ) {
                couleur(C_RED); SLOGT(LOG_CRIT, "Erreur de communication avec le serveur TCP. Nb max de tentative atteinte (%d). Nouvelle tentative dans %d secondes !!!\n", pConf->max_retry,  30 * pConf->retry_delay); couleur_r;
                sleep(30 * pConf->retry_delay);
            } else {
                SLOGT(LOG_WARNING, "Erreur de communication avec le serveur TCP (tentive %d / %d). Nouvelle tentative dans %d secondes\n", c_retry_count + 1, pConf->max_retry, pConf->retry_delay);
                sleep(pConf->retry_delay);
                c_retry_count++;
            }
        }
            
        pthread_testcancel();
        nanosleep(&th_sleep, NULL);
    } /* fin while(1) */
    
    free(msg_to_snd);
    free(rcv_buffer);
    F_Code5T_Free(&pCodeSndToNcCore);

    /* Fin de la boucle du thread */
    SLOGT(LOG_DEBUG, "F_Th_TCPClient: Fin thread\n");
    pthread_cleanup_pop(0);
    pthread_exit(0);
}

