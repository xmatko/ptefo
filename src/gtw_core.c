 /*!
 * \file gtw_core.c
 * \brief Traitements principaux de la passerelle
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 05/09/2014
 *
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>

#include "common.h"
#include "tlog.h"
#include "socks_mgt.h"
#include "code5t.h"
#include "code5t_proc.h"
#include "heartbeat.h"
#include "config.h"
#include "main.h"
#include "gtw_core.h"

static void timer_expire(sigval_t arg);
static int traitement(sMain * pMain);

int TestFifoList(sMain * pMain)
{
    int msg_id = -1;
    unsigned int msg_len = 0;
    char * msg_rcv = (char *)malloc(BUF_SIZE * sizeof (char));
    
    sSockTab ** pSocketServ = pMain->pSocketsCli;
    sSockTab ** pSocketCli = pMain->pSocketsCli;
    
    sSockFifo *** tabFIFO_rcv = NULL;
    tabFIFO_rcv = (sSockFifo ***)malloc(FIFO_NUM_MAX * sizeof (sSockFifo**));
    memset(tabFIFO_rcv, 0, FIFO_NUM_MAX * sizeof (sSockFifo**));

    *(tabFIFO_rcv + FIFO_NC_CORE_SERV) = NULL;
    *(tabFIFO_rcv + FIFO_NC_CORE_CLI) = NULL;
    *(tabFIFO_rcv + FIFO_PDT_SERV) = &(pSocketServ[SOCK_DATA]->rcv_queue);
    *(tabFIFO_rcv + FIFO_PDT_CLI) = &(pSocketCli[SOCK_DATA]->rcv_queue);
    
    sSockFifo ** tmpFIFO = NULL;
    tmpFIFO = *(tabFIFO_rcv + FIFO_PDT_CLI);

    F_SockQueue_Remove(tmpFIFO, &msg_id, msg_rcv, &msg_len);
    
    free(msg_rcv);
    return EXIT_SUCCESS;

}



/*!
 * \fn static void cleanup_handler_UDPServer(void *arg)
 * \brief Cleanup handler pour le thread F_Th_UDPServer
 * \param arg: pointeur sur structure sMain
 */
static void cleanup_handler_GtwCore(void * arg)
{
    sMain * pMain = (sMain *)arg;
    SLOGT(LOG_DEBUG, "F_Th_GtwCore clean-up handler (%d)\n", pMain->dumb);
}

/* Ce thread remplace à la fois F_Th_UDPServer_Data et 
   Il s'agit du coeur du processus de traitement de la gateway entre les PDT et les cartes BIBL (via Nc_core)
*/
void * F_Th_GtwCore(void * arg)
{
    sMain * pMain = (sMain *)arg;
    sSockTab ** pSocketServ = pMain->pSockets;
    sSockTab ** pSocketCli = pMain->pSocketsCli;
   // sSockFifo *** tabFIFO_rcv = NULL;
   // sSockFifo *** tabFIFO_snd = NULL;
   // sSockFifo ** tmpFIFO = NULL;
   // 
   // sCode5T *  pCodeSnd = NULL;
   // sCode5T *  pCodeRcv = NULL;
   //
   // struct timespec th_sleep;
   // th_sleep.tv_sec = 0;
   // th_sleep.tv_nsec = DEFAULT_THREAD_SLEEP_NS;
   // 
   // int hbstatus = HB_UNKNOWN;
   // int hbstatusprev = HB_UNKNOWN;
   // unsigned char hbval = 0;
   // char is_valid = -1;
   //
   // int msg_id  = -1;
   // int src_fifo_num = -1;
   // int dst_fifo_num = -1;
   // unsigned int msg_len    = 0;
   // char * msg_rcv = (char *)malloc(BUF_SIZE * sizeof (char));
   //
   // int nb_elem = -1;
   // int sens = -1;
   //
   pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
   pthread_cleanup_push(cleanup_handler_GtwCore, (void *)pMain);
   SLOGT(LOG_DEBUG, "F_Th_GtwCore: Démarrage thread\n"); fflush(stdout);
   // F_Code5T_Init(&pCodeSnd);
   // F_Code5T_Init(&pCodeRcv);
   //
   // 
   // /* Initialisation du tableau des FIFO de réception */
   // tabFIFO_rcv = (sSockFifo ***)malloc(FIFO_NUM_MAX * sizeof (sSockFifo**));
   // memset(tabFIFO_rcv, 0, FIFO_NUM_MAX * sizeof (sSockFifo**));
   // *(tabFIFO_rcv + FIFO_NC_CORE_SERV) = &(pMain->nc_serv_rcv_queue);        // Remarque: 1 seule FIFO de reception des données de nc_core. On l'affecte donc aux
   // *(tabFIFO_rcv + FIFO_NC_CORE_CLI) = &(pMain->nc_serv_rcv_queue);         //           deux FIFO de réception prévue au départ, ce qui sera tranparent pour l'algo de routage.
   // *(tabFIFO_rcv + FIFO_PDT_SERV) = &(pSocketServ[SOCK_DATA]->rcv_queue);
   // *(tabFIFO_rcv + FIFO_PDT_CLI) = &(pSocketCli[SOCK_DATA]->rcv_queue);
   // 
   // /* Initialisation du tableau des FIFO d'envoi */
   // tabFIFO_snd = (sSockFifo ***)malloc(FIFO_NUM_MAX * sizeof (sSockFifo**));
   // memset(tabFIFO_snd, 0, FIFO_NUM_MAX * sizeof (sSockFifo**));
   // *(tabFIFO_snd + FIFO_NC_CORE_SERV) = &(pMain->nc_cli_snd_queue);        // Remarque: 1 seule FIFO d'émission des données de nc_core. On l'affecte donc aux
   // *(tabFIFO_snd + FIFO_NC_CORE_CLI) = &(pMain->nc_cli_snd_queue);         //           deux FIFO d'émission prévue au départ, ce qui sera tranparent pour l'algo de routage.
   // *(tabFIFO_snd + FIFO_PDT_SERV) = &(pSocketServ[SOCK_DATA]->snd_queue);
   // *(tabFIFO_snd + FIFO_PDT_CLI) = &(pSocketCli[SOCK_DATA]->snd_queue);
   
    /*
    F_SockQueue_SetName(**(tabFIFO_rcv + FIFO_PDT_SERV), "file du serveur UDP des messages reçus depuis le poste de tête");
    F_SockQueue_SetName(**(tabFIFO_rcv + FIFO_PDT_CLI), "file du client UDP des messages reçus depuis le poste de tête");
    F_SockQueue_SetName(**(tabFIFO_snd + FIFO_PDT_SERV), "file du serveur UDP des messages à envoyer vers le poste de tête");
    F_SockQueue_SetName(**(tabFIFO_snd + FIFO_PDT_CLI), "file du client UDP des messages à envoyer vers le poste de tête");
    */
   
    F_SockQueue_SetName(pSocketServ[SOCK_DATA]->rcv_queue, "file du serveur UDP des messages reçus depuis le poste de tête");
    F_SockQueue_SetName(pSocketCli[SOCK_DATA]->rcv_queue, "file du client UDP des messages reçus depuis le poste de tête");
    F_SockQueue_SetName(pSocketServ[SOCK_DATA]->snd_queue, "file du serveur UDP des messages à envoyer vers le poste de tête");
    F_SockQueue_SetName(pSocketCli[SOCK_DATA]->snd_queue, "file du client UDP des messages à envoyer vers le poste de tête");
    
    F_ProcAudio_Set_ActiveAudio(pMain->pAudioCircuit, 1, CC_AUDIO_STATUS_ACTIVE);
    
    ///* Démarrage de la boucle du thread */
    //while(1)
    //{
    //    /* Pour chaque FIFO définie dans le tableau des FIFO de réception
    //       on va dépiler les messages contenus, les traiter et les ajouter
    //       à la FIFO d'envoi adéquate afin de router le message */
    //    for (src_fifo_num = 0; src_fifo_num < FIFO_NUM_MAX; src_fifo_num++) {
    //        tmpFIFO = *(tabFIFO_rcv + src_fifo_num);
    //        if ((src_fifo_num == FIFO_PDT_SERV) || (src_fifo_num == FIFO_PDT_CLI)) {
    //            sens = FROM_PDT;
    //        } else {
    //            sens = FROM_NC_CORE;
    //        }
    //        if (tmpFIFO != NULL) {
    //            /* Dépiler les messages de la file nc_core_reception */
    //            nb_elem = F_SockQueue_GetNbElem(*(tmpFIFO));
    //            while (nb_elem > 0) {
    //                //couleur(C_WHITE); SLOGT(LOG_DEBUG, "Retrait depuis la %s\n", F_SockQueue_GetName(*tmpFIFO)); couleur_r;
    //                memset(msg_rcv, 0, BUF_SIZE * sizeof (char));
    //                F_SockQueue_Remove(tmpFIFO, &msg_id, msg_rcv, &msg_len);
    //                
    //                /* Mise en forme et décodage du message 5-tons */
    //                F_Code5T_Reset(pCodeRcv);
    //                F_Code5T_Set_Str(pCodeRcv, (unsigned char *)msg_rcv, sens);
    //                F_Code5T_PDT2BIBL(pCodeRcv);
    //                /* Traitement du message reçu et determination du routage */
    //                F_Code5T_Reset(pCodeSnd);
    //                dst_fifo_num = -1;
    //                F_GtwCore_Traite_Msg(pMain, pCodeRcv, src_fifo_num, &pCodeSnd, &dst_fifo_num);
    //                /* Au retour de la fonction F_GtwCore_Traite_Msg, pCodeSnd contient le code à transferer à la fifo numéro dst_fifo_num
    //                  Si dst_fifo_num = -1, c'est qu'il n'y a rien à transferer (réponse à une trame de vie par exemple) */
    //
    //                /* On ajoute ici le message à router dans la FIFO concernée. Ces infos sont retournées par F_GtwCore_Traite_Msg() */
    //                if ((dst_fifo_num != -1) && (dst_fifo_num < FIFO_NUM_MAX)) {
    //                    /* Affichage des infos du code à transferer */
    //                    F_Code5T_PDT2BIBL(pCodeSnd);
    //                    //couleur(C_YELLOW); SLOGT(LOG_INFO, "Code 5-tons à transferer:\n");  couleur_r;
    //                    //NLOG_INFO(F_Code5T_Infos(pCodeSnd));
    //                    if (*(tabFIFO_snd + dst_fifo_num) != NULL) {
    //                        //couleur(C_WHITE); SLOGT(LOG_INFO, "Ajout à la %s\n", F_SockQueue_GetName(**(tabFIFO_snd + dst_fifo_num))); couleur_r;
    //                        F_SockQueue_Add(*(tabFIFO_snd + dst_fifo_num), (char *)pCodeSnd->cstr, pCodeSnd->cstr_len);
    //                    } else {SLOGT(LOG_DEBUG, "PAS d'ajout à la file %d\n", dst_fifo_num); }
    //                } else { SLOGT(LOG_INFO, "Rien à transferer\n"); }
    //                pthread_testcancel();
    //                nb_elem = F_SockQueue_GetNbElem(*(tmpFIFO));
    //            } /* Fin while nb_rcv_elem */
    //        }
    //    } /* Fin for */
    //    
    //    F_GtwCode_RemovePTOInactives(pMain);
    //    
    //    /* Construction du message heartbeat à envoyer */
    //    hbval = F_HB_GetValue(pMain->pHB);
    //    if (hbval != F_HB_GetValuePrev(pMain->pHB)) {
    //        F_Code5T_Reset(pCodeSnd);
    //        F_Code5T_Set_Num(pCodeSnd, TO_PDT, CTP_SYSTEM, CID_HEARTBEAT, &(hbval), CODE_VALUE_LEN);
    //        NLOG_INFO(F_Code5T_Infos(pCodeSnd));
    //        F_Code5T_GetValidity(pCodeSnd, &is_valid);
    //        if (is_valid == 1) {
    //            F_HB_UpdatePrev(pMain->pHB);
    //            F_HB_UpdateStatus(pMain->pHB);
    //            if (F_HB_GetStatus(pMain->pHB) != HB_DEAD) {
    //                /* Ajout du message dans la file d'envoi */
    //                F_SockQueue_Add(&(pSocketCli[SOCK_DATA]->snd_queue), (char *)pCodeSnd->cstr, pCodeSnd->cstr_len);
    //                SLOGT(LOG_NOTICE, "\tEnvoi de la trame de vie (0x%02X)\n", hbval);
    //            }
    //        } else {
    //            SLOGT(LOG_WARNING, "MESSAGE INVALIDE! Pas de traitement (type %d, id %d, taille valeur %d)\n", pCodeSnd->type, pCodeSnd->id, pCodeSnd->value_size);
    //        }
    //    }
    //    
    //    /* Vérification du status du heartbeat */
    //    hbstatus = F_HB_GetStatus(pMain->pHB);
    //    hbstatusprev = F_HB_GetStatusPrev(pMain->pHB);
    //    if ((hbstatus != hbstatusprev) && (hbstatus == HB_DEAD)) {
    //        SLOGT(LOG_ERR, "Erreur d'acquittement de la trame de vie (snd %u / rcv %u) !!!\n", F_HB_GetValue(pMain->pHB), F_HB_GetValueRcv(pMain->pHB));
    //        /* Inhibition des traitements */
    //        couleur(C_YELLOW); SLOGT(LOG_WARNING, "Le signal de vie n'est plus détecté - Inhibition des traitements\n"); couleur_r;
    //        F_HB_UpdateStatus(pMain->pHB);
    //        F_Set_InhibStatus(pMain, 1);
    //        /* Mise à jour du status du PDT */
    //        F_Set_PDTStatus(pMain, PDT_STATUS_HS);
    //    }
    //    
    //    pthread_testcancel();
    //    nanosleep(&th_sleep, NULL);
    //} /* Fin while(1) */
    ///* Fin de la boucle du thread */
    //
    //free(msg_rcv);
    //free(tabFIFO_rcv);
    //free(tabFIFO_snd);
    //
    //F_Code5T_Free(&pCodeSnd);
    //F_Code5T_Free(&pCodeRcv);

        
    struct sigevent sev;
    struct itimerspec its;

    /* Create the timer */
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_value.sival_ptr = pMain;
    sev.sigev_notify_function = timer_expire;
    sev.sigev_notify_attributes = NULL;

    if (timer_create(CLOCK_MONOTONIC, &sev, &(pMain->gtw_core_timerid)) == -1)   die("timer_create");

    /* Armement du timer */
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 100 * M_MILLION;    /* 100 ms */
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;

    timer_settime(pMain->gtw_core_timerid, 0, &its, NULL);
    
    /* Début de la boucle du thread */
    while (1) {
        sleep(1);
        pthread_testcancel();
    } /* fin while (1) */
    
    SLOGT(LOG_DEBUG, "F_Th_GtwCore: Fin thread\n");
    pthread_cleanup_pop(0);
    pthread_exit(0);
}



static void timer_expire(sigval_t arg)
{
    int gor = -1;
    //struct timespec gts1, gts2, gts3;
    
    //clock_gettime(CLOCK_MONOTONIC, &gts1);
    
    sMain * pMain = (sMain *)arg.sival_ptr;
    gor = timer_getoverrun(pMain->gtw_core_timerid);
    if (gor > 2) {
        SLOGT(LOG_WARNING, "Gtw Timer overrun %d\n", gor);
    }
    traitement(pMain);
    //clock_gettime(CLOCK_MONOTONIC, &gts2);
    //gts3.tv_sec = gts2.tv_sec - gts1.tv_sec;
    //gts3.tv_nsec = gts2.tv_nsec - gts1.tv_nsec;
    //
    //SLOGT(LOG_NOTICE, "Durée du traitement gtw_core:  %d µs\n", gts3.tv_nsec/1000);
}


static int traitement(sMain * pMain)
{

    sSockTab ** pSocketServ = pMain->pSockets;
    sSockTab ** pSocketCli = pMain->pSocketsCli;
    sSockFifo *** tabFIFO_rcv = NULL;
    sSockFifo *** tabFIFO_snd = NULL;
    sSockFifo ** tmpFIFO = NULL;
    
    sCode5T *  pCodeSnd = NULL;
    sCode5T *  pCodeRcv = NULL;

    struct timespec th_sleep;
    th_sleep.tv_sec = 0;
    th_sleep.tv_nsec = DEFAULT_THREAD_SLEEP_NS;
    
    int hbstatus = HB_UNKNOWN;
    int hbstatusprev = HB_UNKNOWN;
    unsigned char hbval = 0;
    char is_valid = -1;

    int msg_id  = -1;
    int src_fifo_num = -1;
    int dst_fifo_num = -1;
    unsigned int msg_len    = 0;
    char * msg_rcv = (char *)malloc(BUF_SIZE * sizeof (char));

    int nb_elem = -1;
    int sens = -1;

    //SLOGT(LOG_DEBUG, "F_Th_GtwCore: Démarrage thread\n"); fflush(stdout);
    F_Code5T_Init(&pCodeSnd);
    F_Code5T_Init(&pCodeRcv);

    
    /* Initialisation du tableau des FIFO de réception */
    tabFIFO_rcv = (sSockFifo ***)malloc(FIFO_NUM_MAX * sizeof (sSockFifo**));
    memset(tabFIFO_rcv, 0, FIFO_NUM_MAX * sizeof (sSockFifo**));
    *(tabFIFO_rcv + FIFO_NC_CORE_SERV) = &(pMain->nc_serv_rcv_queue);        // Remarque: 1 seule FIFO de reception des données de nc_core. On l'affecte donc aux
    *(tabFIFO_rcv + FIFO_NC_CORE_CLI) = &(pMain->nc_serv_rcv_queue);         //           deux FIFO de réception prévue au départ, ce qui sera tranparent pour l'algo de routage.
    *(tabFIFO_rcv + FIFO_PDT_SERV) = &(pSocketServ[SOCK_DATA]->rcv_queue);
    *(tabFIFO_rcv + FIFO_PDT_CLI) = &(pSocketCli[SOCK_DATA]->rcv_queue);
    
    /* Initialisation du tableau des FIFO d'envoi */
    tabFIFO_snd = (sSockFifo ***)malloc(FIFO_NUM_MAX * sizeof (sSockFifo**));
    memset(tabFIFO_snd, 0, FIFO_NUM_MAX * sizeof (sSockFifo**));
    *(tabFIFO_snd + FIFO_NC_CORE_SERV) = &(pMain->nc_cli_snd_queue);        // Remarque: 1 seule FIFO d'émission des données de nc_core. On l'affecte donc aux
    *(tabFIFO_snd + FIFO_NC_CORE_CLI) = &(pMain->nc_cli_snd_queue);         //           deux FIFO d'émission prévue au départ, ce qui sera tranparent pour l'algo de routage.
    *(tabFIFO_snd + FIFO_PDT_SERV) = &(pSocketServ[SOCK_DATA]->snd_queue);
    *(tabFIFO_snd + FIFO_PDT_CLI) = &(pSocketCli[SOCK_DATA]->snd_queue);

    /* Pour chaque FIFO définie dans le tableau des FIFO de réception
       on va dépiler les messages contenus, les traiter et les ajouter
       à la FIFO d'envoi adéquate afin de router le message */
    for (src_fifo_num = 0; src_fifo_num < FIFO_NUM_MAX; src_fifo_num++) {
        tmpFIFO = *(tabFIFO_rcv + src_fifo_num);
        if ((src_fifo_num == FIFO_PDT_SERV) || (src_fifo_num == FIFO_PDT_CLI)) {
            sens = FROM_PDT;
        } else {
            sens = FROM_NC_CORE;
        }
        if (tmpFIFO != NULL) {
            /* Dépiler les messages de la file nc_core_reception */
            nb_elem = F_SockQueue_GetNbElem(*(tmpFIFO));
            while (nb_elem > 0) {
                //couleur(C_WHITE); SLOGT(LOG_DEBUG, "Retrait depuis la %s\n", F_SockQueue_GetName(*tmpFIFO)); couleur_r;
                memset(msg_rcv, 0, BUF_SIZE * sizeof (char));
                F_SockQueue_Remove(tmpFIFO, &msg_id, msg_rcv, &msg_len);
                
                /* Mise en forme et décodage du message 5-tons */
                F_Code5T_Reset(pCodeRcv);
                F_Code5T_Set_Str(pCodeRcv, (unsigned char *)msg_rcv, sens);
                F_Code5T_PDT2BIBL(pCodeRcv);
                /* Traitement du message reçu et determination du routage */
                F_Code5T_Reset(pCodeSnd);
                dst_fifo_num = -1;
                F_GtwCore_Traite_Msg(pMain, pCodeRcv, src_fifo_num, &pCodeSnd, &dst_fifo_num);
                /* Au retour de la fonction F_GtwCore_Traite_Msg, pCodeSnd contient le code à transferer à la fifo numéro dst_fifo_num
                  Si dst_fifo_num = -1, c'est qu'il n'y a rien à transferer (réponse à une trame de vie par exemple) */

                /* On ajoute ici le message à router dans la FIFO concernée. Ces infos sont retournées par F_GtwCore_Traite_Msg() */
                if ((dst_fifo_num != -1) && (dst_fifo_num < FIFO_NUM_MAX)) {
                    /* Affichage des infos du code à transferer */
                    F_Code5T_PDT2BIBL(pCodeSnd);
                    //couleur(C_YELLOW); SLOGT(LOG_INFO, "Code 5-tons à transferer:\n");  couleur_r;
                    //NLOG_INFO(F_Code5T_Infos(pCodeSnd));
                    if (*(tabFIFO_snd + dst_fifo_num) != NULL) {
                        //couleur(C_WHITE); SLOGT(LOG_INFO, "Ajout à la %s\n", F_SockQueue_GetName(**(tabFIFO_snd + dst_fifo_num))); couleur_r;
                        F_SockQueue_Add(*(tabFIFO_snd + dst_fifo_num), (char *)pCodeSnd->cstr, pCodeSnd->cstr_len);
                    } else {SLOGT(LOG_DEBUG, "PAS d'ajout à la file %d\n", dst_fifo_num); }
                } else { SLOGT(LOG_INFO, "Rien à transferer\n"); }
                pthread_testcancel();
                nb_elem = F_SockQueue_GetNbElem(*(tmpFIFO));
            } /* Fin while nb_rcv_elem */
        }
    } /* Fin for */
    
    F_GtwCode_RemovePTOInactives(pMain);
    
    /* Construction du message heartbeat à envoyer */
    hbval = F_HB_GetValue(pMain->pHB);
    if (hbval != F_HB_GetValuePrev(pMain->pHB)) {
        F_Code5T_Reset(pCodeSnd);
        F_Code5T_Set_Num(pCodeSnd, TO_PDT, CTP_SYSTEM, CID_HEARTBEAT, &(hbval), CODE_VALUE_LEN);
        NLOG_INFO(F_Code5T_Infos(pCodeSnd));
        F_Code5T_GetValidity(pCodeSnd, &is_valid);
        if (is_valid == 1) {
            F_HB_UpdatePrev(pMain->pHB);
            F_HB_UpdateStatus(pMain->pHB);
            if (F_HB_GetStatus(pMain->pHB) != HB_DEAD) {
                /* Ajout du message dans la file d'envoi */
                F_SockQueue_Add(&(pSocketCli[SOCK_DATA]->snd_queue), (char *)pCodeSnd->cstr, pCodeSnd->cstr_len);
                SLOGT(LOG_NOTICE, "\tEnvoi de la trame de vie (0x%02X)\n", hbval);
            }
        } else {
            SLOGT(LOG_WARNING, "MESSAGE INVALIDE! Pas de traitement (type %d, id %d, taille valeur %d)\n", pCodeSnd->type, pCodeSnd->id, pCodeSnd->value_size);
        }
    }
    
    /* Vérification du status du heartbeat */
    hbstatus = F_HB_GetStatus(pMain->pHB);
    hbstatusprev = F_HB_GetStatusPrev(pMain->pHB);
    if ((hbstatus != hbstatusprev) && (hbstatus == HB_DEAD)) {
        SLOGT(LOG_ERR, "Erreur d'acquittement de la trame de vie (snd %u / rcv %u) !!!\n", F_HB_GetValue(pMain->pHB), F_HB_GetValueRcv(pMain->pHB));
        /* Inhibition des traitements */
        couleur(C_YELLOW); SLOGT(LOG_WARNING, "Le signal de vie n'est plus détecté - Inhibition des traitements\n"); couleur_r;
        F_HB_UpdateStatus(pMain->pHB);
        F_Set_InhibStatus(pMain, 1);
        /* Mise à jour du status du PDT */
        F_Set_PDTStatus(pMain, PDT_STATUS_HS);
    }
    
    
    free(msg_rcv);
    free(tabFIFO_rcv);
    free(tabFIFO_snd);
 
    F_Code5T_Free(&pCodeSnd);
    F_Code5T_Free(&pCodeRcv);
    
    return EXIT_SUCCESS;
}


int F_GtwCore_Traite_Msg(sMain * pMain, sCode5T * pCodeRcv, int src_fifo_num, sCode5T ** ppCodeAns, int * dst_fifo_num)
{
    /* Vérification de la validité du message reçu */
    char is_valid = -1;
    
    F_Code5T_GetValidity(pCodeRcv, &is_valid);
    if (is_valid != 1) {
        couleur(C_RED); SLOGT(LOG_WARNING, "MESSAGE INVALIDE! Pas de traitement (type %d, id %d, taille valeur %d)\n", pCodeRcv->type, pCodeRcv->id, pCodeRcv->value_size); couleur_r;
        return EXIT_FAILURE;
    }
    
    /* Affichage des informations sur le message reçu */
    couleur(C_YELLOW); SLOGT(LOG_NOTICE, "Code 5-tons recu à traiter:\n");  couleur_r;
    NLOG_NOTICE(F_Code5T_Infos(pCodeRcv));
    
    /* Determination du type de message reçu */
    switch(pCodeRcv->type) {
        case CTP_CIRCUIT_1:
        case CTP_CIRCUIT_2:
        case CTP_CIRCUIT_3:
        case CTP_CIRCUIT_4:
            F_GtwCore_Traite_Msg_TypePTO(pMain, pCodeRcv, src_fifo_num, ppCodeAns, dst_fifo_num);
            break;
        case CTP_AUDIO_TEST:
            SLOGT(LOG_WARNING, "\tFIXME: Message de type audio pas encore pris en compte \n");
            break;
        case CTP_SYSTEM:
            F_GtwCore_Traite_Msg_TypeSystem(pMain, pCodeRcv, src_fifo_num, ppCodeAns, dst_fifo_num);
            break;
        default:
            couleur(C_RED); SLOGT(LOG_ERR, "\tType de message non reconnu...\n"); couleur_r; /* On ne devrait jamais arriver dans ce cas, la validité du message ayant été testée en amont */
            break;
    } /* Fin switch(pCodeRcv->type) */
    return EXIT_SUCCESS;
}

int F_GtwCore_Traite_Msg_TypePTO(sMain * pMain, sCode5T * pCodeRcv, int src_fifo_num, sCode5T ** ppCodeAns, int * dst_fifo_num)
{
    int pto_idx = -1;
    int circuit = pCodeRcv->type;
    unsigned char action5t = pCodeRcv->code_val[0];
    sPTO * pto = NULL;
    
    /* Vérification qu'il s'agit bien d'un message de type PTO */
    if ((circuit >= CTP_CIRCUIT_1) && (circuit <= CTP_CIRCUIT_4)) {
        /* S'agit-il d'un PTO déja en traitement ? => Parcours la liste des PTO en traitement */
        //F_PTO_List_DisplayContent(pMain->pPtoList);

        pto_idx = F_PTO_List_FindPtoId(pMain->pPtoList, pCodeRcv->id);
        if (pto_idx == -1) {
            /* S'agit-il d'un message d'action d'appeller un PTO ou d'appeller un pupitre ? => Analyse le code action du message et la fifo source */
            /* Il s'agit d'un appel emis depuis un PTO vers un pupitre exterieur ou bien d'une déclaration d'un défaut d'alim d'un PTO*/
            if ((src_fifo_num == FIFO_PDT_SERV) && ((action5t == CVL_C5T_1) || (action5t == CVL_C5T_4))) {
                /* Provient de la socket serveur de comm avec PDT: Ajout d'un PTO de type serveur */
                pto_idx = F_PTO_List_Append(&(pMain->pPtoList), pCodeRcv->id, circuit, PTO_STATUS_INIT, COMM_INIT_BY_PTO);
            }
            /* Il s'agit d'un appel emis depuis un pupitre vers un PTO */
            else if ((src_fifo_num == FIFO_NC_CORE_SERV) && (action5t == CVL_C5T_2)) {
                /* Provient de la socket serveur de comm Nc_core: Ajout d'un PTO de type client */
                pto_idx = F_PTO_List_Append(&(pMain->pPtoList), pCodeRcv->id, circuit, PTO_STATUS_INIT, COMM_INIT_BY_NC_CORE);
            }
            /* Il ne s'agit d'aucun cas ci-dessus: ERREUR */
            else {
                couleur(C_RED);
                SLOGT(LOG_ERR, "Un nouveau PTO ne peut être pris en compte que suite à un message d'appel (src_fifo = %d, action = %u)\n", src_fifo_num, action5t);
                couleur_r;
                return EXIT_FAILURE;
            }
        }
        
        /* A ce point, soit le PTO à été trouvé dans la liste des PTO actifs, soit il y a été ajouté
           On traite alors l'action contenue dans le message
        */
        //F_GtwCore_Traite_Msg_ActionPTO(pMain, pto_idx, pCodeRcv, src_fifo_num, ppCodeAns, dst_fifo_num);
        
        /* On récupère une copie de ce PTO */
        pto = F_PTO_List_GetPto(pMain->pPtoList, pto_idx);

        /* On vérifie que le message concernant ce PTO concerne le même circuit que le PTO récupéré dans la liste */
        if (pto->circuit == pCodeRcv->type) {
            /* Puis on effectue le traitement en fonction de l'action contenue dans le message 5-tons pour ce PTO */
            F_GtwCore_Traite_Msg_ActionPTO_Global(pMain, pto_idx, pCodeRcv, src_fifo_num, ppCodeAns, dst_fifo_num);
            
            //switch (pto->comm_type) {
            //    case COMM_INIT_BY_PTO:        /* La communication à été initiée par un poste de tête */
            //        F_GtwCore_Traite_Msg_ActionPTO_Serveur(pMain, pto_idx, pCodeRcv, src_fifo_num, ppCodeAns, dst_fifo_num);
            //        break;
            //    case COMM_INIT_BY_NC_CORE:    /* La communication à été initiée par notre client (appel depuis un pupitre transmis via Nc_core) */
            //        F_GtwCore_Traite_Msg_ActionPTO_Client(pMain, pto_idx, pCodeRcv, src_fifo_num, ppCodeAns, dst_fifo_num);
            //        break;
            //    default:
            //        couleur(C_RED); SLOGT(LOG_ERR, "\tF_GtwCore_Traite_Msg_ActionPTO: Erreur de comm_type\n"); couleur_r; 
            //} /* Fin switch */
        } else {
            couleur(C_RED); SLOGT(LOG_ERR, "\tF_GtwCore_Traite_Msg_ActionPTO: Erreur de correspondance de circuit (PTO actif circuit %d / Code recu circuit %d)!\n", pto->circuit, pCodeRcv->type);
            couleur_r; 
        }
        free(pto);
        /* Au retour de la fonction F_GtwCore_Traite_Msg_ActionPTO, *ppCodeAns contient le code à transferer à la fifo numéro *dst_fifo_num */
        
    } /* Fin if */
    return EXIT_SUCCESS;
}



int F_GtwCore_Traite_Msg_ActionPTO_Global(sMain * pMain, int pto_idx, sCode5T * pCodeRcv, int src_fifo_num, sCode5T ** ppCodeAns, int * dst_fifo_num)
{
    unsigned char action5t = pCodeRcv->code_val[0];
    unsigned char action5t_ret = CVL_C5T_0;
    *dst_fifo_num = -1;
    F_Code5T_Reset(*ppCodeAns);
    enum eTransfertPermission valid_trans = TP_REFUSED;
    /* On récupère l'élement PTO ACTIF parmi la liste */
    sPTO * pto = F_PTO_List_GetPto(pMain->pPtoList, pto_idx);
    int pto_idx_online = -1;
    int pto_nb_online = -1;
    
    switch (action5t) {
        case CVL_C5T_0:
            /* Indication de sonnerie d'un PTO  (GTW <- PDT) */
            /* Rien a transferer. Cela indique juste à la gateway que le pupitre sonne */
            switch (pto->status) {
                case PTO_STATUS_CALLED:
                case PTO_STATUS_CALLBACK:
                    couleur(C_GREEN); SLOGT(LOG_NOTICE, "\tPTO %d indique qu'il sonne sur circuit %d...\n", pCodeRcv->id, pCodeRcv->type); couleur_r;
                    /* Mise à jour du status du PTO */
                    pto->status = PTO_STATUS_RINGING;
                    break;
                default:
                    couleur(C_RED); SLOGT(LOG_WARNING, "\tPTO %d indique qu'il sonne sur circuit %d...Mais n'a pas été appellé !!!\n", pCodeRcv->id, pCodeRcv->type); couleur_r;
                    pto->status = PTO_STATUS_UNKNOWN;
            }
            break;
            
        case CVL_C5T_1:
            /* Ce code est à traiter différement selon l'entité qui à initiée la communication */
            switch (pto->status) {
                case PTO_STATUS_INIT:
                    /* Dans ce cas, il s'agit du poste de tête qui veut initier un appel vers un pupitre */
                    couleur(C_GREEN); SLOGT(LOG_NOTICE, "\tLe PTO %d appelle sur circuit %d...\n", pCodeRcv->id, pCodeRcv->type); couleur_r;
                    pto->status = PTO_STATUS_CALLING;
                    valid_trans = TP_ALLOWED;
                    break;
                case PTO_STATUS_RINGING:
                case PTO_STATUS_CALLBACK:
                case PTO_STATUS_CALLED:
                    /* Dans ce cas, il s'agit d'un PTO qui à décroché un appel initié par un pupitre */
                    couleur(C_GREEN); SLOGT(LOG_NOTICE, "\tLe PTO %d à décroché sur circuit %d en réponse à l'appel d'un pupitre...\n", pCodeRcv->id, pCodeRcv->type); couleur_r;
                    pto->status = PTO_STATUS_ONLINE;
                    valid_trans = TP_ALLOWED;
                    /* Etablissement de la comm audio sur le circuit */
                    F_ProcAudio_Set_ActiveAudio(pMain->pAudioCircuit, pCodeRcv->type, CC_AUDIO_STATUS_ACTIVE);
                    break;
                case PTO_STATUS_CALLING:
                case PTO_STATUS_ONLINE:
                    couleur(C_YELLOW); SLOGT(LOG_NOTICE, "\tUn appel est déja en cours pour le PTO %d sur circuit %d: %s\n", pCodeRcv->id, pCodeRcv->type, str_ptostatus(pto->status)); couleur_r;
                    valid_trans = TP_REFUSED;
                    break;
                default:
                    couleur(C_RED); SLOGT(LOG_ERR, "\tErreur de cohérence PTO %d sur circuit %d. Status PTO: %s\n", pCodeRcv->id, pCodeRcv->type, str_ptostatus(pto->status)); couleur_r;
                    pto->status = PTO_STATUS_UNKNOWN;
                    valid_trans = TP_REFUSED;
            }
            /* Dans les deux cas, ce message doit être transferé à Nc_core */
            if (valid_trans == TP_ALLOWED) {
                *dst_fifo_num = FIFO_NC_CORE_CLI;
                action5t_ret = action5t;
                F_Code5T_Set_Num(*ppCodeAns, FROM_PDT, pto->circuit, pto->id, &(action5t_ret), CODE_VALUE_LEN);
            }
            break;
            
        case CVL_C5T_2:
            if (src_fifo_num == FIFO_NC_CORE_SERV) {
                /* Ce code est à traiter différement selon l'entité qui à initiée la communication */
                switch (pto->status) {
                    case PTO_STATUS_INIT:
                        /* Dans ce cas, il s'agit du pupitre qui veut initier un appel vers un PTO */
                        couleur(C_GREEN); SLOGT(LOG_NOTICE, "\tUn pupitre appelle le PTO %d sur circuit %d...\n", pCodeRcv->id, pCodeRcv->type); couleur_r;
                        pto->status = PTO_STATUS_CALLED;
                        valid_trans = TP_ALLOWED;
                        break;
                    case PTO_STATUS_CALLING:
                        /* Dans ce cas, il s'agit du pupitre qui à décroché un appel initié par un PTO
                        Etablissement de la comm audio sur le circuit (GTW -> PDT) */
                        couleur(C_GREEN); SLOGT(LOG_NOTICE, "\tUn pupitre à décroché à l'appel du PTO %d sur circuit %d...Etablissement de la comm audio\n", pCodeRcv->id, pCodeRcv->type); couleur_r;
                        pto->status = PTO_STATUS_ONLINE;
                        valid_trans = TP_ALLOWED;
                        /* Etablissement de la comm audio sur le circuit */
                        F_ProcAudio_Set_ActiveAudio(pMain->pAudioCircuit, pCodeRcv->type, CC_AUDIO_STATUS_ACTIVE);
                        break;
                    case PTO_STATUS_CALLED:
                    case PTO_STATUS_ONLINE:
                        couleur(C_YELLOW); SLOGT(LOG_NOTICE, "\tUn appel est déja en cours pour le PTO %d sur circuit %d: %s\n", pCodeRcv->id, pCodeRcv->type, str_ptostatus(pto->status)); couleur_r;
                        valid_trans = TP_REFUSED;
                        break;
                    default:
                        couleur(C_RED); SLOGT(LOG_ERR, "\tErreur de cohérence PTO %d sur circuit %d. Status PTO: %s\n", pCodeRcv->id, pCodeRcv->type, str_ptostatus(pto->status)); couleur_r;
                        pto->status = PTO_STATUS_UNKNOWN;
                        valid_trans = TP_REFUSED;
                }
                /* Dans les deux cas, le message doit être transferé au PDT */
                if (valid_trans == TP_ALLOWED) {
                    *dst_fifo_num = FIFO_PDT_CLI;
                    action5t_ret = action5t;
                    F_Code5T_Set_Num(*ppCodeAns, TO_PDT, pto->circuit, pto->id, &(action5t_ret), CODE_VALUE_LEN);
                }
            } else {
                couleur(C_RED); SLOGT(LOG_ERR, "\tERREUR: Ce message ne peut être reçu que de Nc_core. PTO %d sur circuit %d. Status PTO: %s\n", pCodeRcv->id, pCodeRcv->type, str_ptostatus(pto->status)); couleur_r;
            }
            break;
        
        case CVL_C5T_3:
            /* Un PTO raccroche un appel (GTW <- PDT) */
            switch (pto->status) {
                case PTO_STATUS_ONLINE:
                    couleur(C_GREEN); SLOGT(LOG_NOTICE, "\tLe PTO %d raccroche sur circuit %d...\n", pCodeRcv->id, pCodeRcv->type); couleur_r;
                    pto->status = PTO_STATUS_ENDCALL;
                    valid_trans = TP_ALLOWED;

                    /* On recherche les autres PTO en ligne sur ce circuit. Si il en reste plus d'un, on garde la comm audio sur le circuit, sinon on la termine */
                    pto_nb_online = F_PTO_List_FindOnlinePto_ByCircuit(pMain->pPtoList, pCodeRcv->type);
                    if (pto_nb_online == 1) {
                        pto_idx_online = F_PTO_List_FindCircuit(pMain->pPtoList, pCodeRcv->type);
                        /* On vérifie que l'unique PTO restant correspond à celui traité actuellement */
                        if (pto_idx_online == pto_idx) {
                            F_ProcAudio_Set_ActiveAudio(pMain->pAudioCircuit, pCodeRcv->type, CC_AUDIO_STATUS_INACTIVE);
                        } else {
                            SLOGT(LOG_WARNING, "Erreur de correspondance de l'unique PTO restant online sur le circuit %d (idx %d) et le PTO traité ici (idx %d)\n", pCodeRcv->type, pto_idx_online, pto_idx);
                        }
                    }
                    
                    break;
                case PTO_STATUS_CALLED:
                case PTO_STATUS_RINGING:
                case PTO_STATUS_CALLBACK:
                    couleur(C_RED); SLOGT(LOG_NOTICE, "\tLe PTO %d n'a pas décroché sur circuit %d à l'échéance du timeout...\n", pCodeRcv->id, pCodeRcv->type); couleur_r;
                    pto->status = PTO_STATUS_ENDCALL;
                    valid_trans = TP_ALLOWED;
                    break;
                case PTO_STATUS_CALLING:
                    couleur(C_RED); SLOGT(LOG_NOTICE, "\tLe PTO %d a raccroché avant d'avoir obtenu l'appel sur circuit %d...\n", pCodeRcv->id, pCodeRcv->type); couleur_r;
                    pto->status = PTO_STATUS_ENDCALL;
                    valid_trans = TP_ALLOWED;
                    break;
                default:
                    couleur(C_RED); SLOGT(LOG_ERR, "\tErreur de cohérence PTO %d sur circuit %d. Status PTO: %s\n", pCodeRcv->id, pCodeRcv->type, str_ptostatus(pto->status)); couleur_r;
                    pto->status = PTO_STATUS_UNKNOWN;
                    valid_trans = TP_REFUSED;
            }

            /* Le message doit être transferé à Nc_core */
            *dst_fifo_num = FIFO_NC_CORE_CLI;
            action5t_ret = action5t;
            F_Code5T_Set_Num(*ppCodeAns, FROM_PDT, pto->circuit, pto->id, &(action5t_ret), CODE_VALUE_LEN);


            break;
            
        case CVL_C5T_4:
            /* Un PTO déclare un défaut d'alimentation (GTW <- PDT) */
            couleur(C_YELLOW); SLOGT(LOG_NOTICE, "\tLe PTO %d sur circuit %d déclare un défaut d'alimentation (%u) !\n", pCodeRcv->id, pCodeRcv->type, action5t); couleur_r;

            /* FIXME: pour le moment, ce message est identifié mais n'est pas transferé à Nc_core. Une clarification doit être réalisée en ce qui
            concerne le comportement d'un PTO en défaut d'alim:
                le défaut peut-il être recu pendant une comm audio ?
                Si un PTO est déclaré en défaut d'alim, peut-il recevoir ou emettre un appel audio ?
                Quelles conditions permettent de réinitialiser son état ?
            */
            ///* Mise à jour du status du PTO */
            //pto->status = PTO_STATUS_POWER_DEF;
            //
            ///* Le message doit être transferé à Nc_core */
            //*dst_fifo_num = FIFO_NC_CORE_CLI;
            //action5t_ret = action5t;
            //F_Code5T_Set_Num(*ppCodeAns, FROM_PDT, pto->circuit, pto->id, &(action5t_ret), CODE_VALUE_LEN);
            break;
            
        case CVL_C5T_5:
            /* Un PTO envoi un retour d'appel (GTW <- PDT) */
            couleur(C_GREEN); SLOGT(LOG_NOTICE, "\tPTO %d sur circuit %d envoie un retour d'appel (%u) !\n", pCodeRcv->id, pCodeRcv->type, action5t); couleur_r;
            /* Mise à jour du status du PTO */
            pto->status = PTO_STATUS_CALLBACK;
            //*dst_fifo_num = FIFO_NC_CORE_CLI;
            //action5t_ret = action5t;
            //F_Code5T_Set_Num(*ppCodeAns, FROM_PDT, pto->circuit, pto->id, &(action5t_ret), CODE_VALUE_LEN);
            break;
            
        case CVL_C5T_6:
            /* Un PTO n'est pas en ligne (GTW <- PDT) */
            switch (pto->status) {
                case PTO_STATUS_CALLED:
                    couleur(C_RED); SLOGT(LOG_NOTICE, "\tPTO %d sur circuit %d n'est pas en ligne (%u)\n", pCodeRcv->id, pCodeRcv->type, action5t); couleur_r;
                    pto->status = PTO_STATUS_OFFLINE;
                    valid_trans = TP_ALLOWED;
                default:
                    pto->status = PTO_STATUS_UNKNOWN;
                    valid_trans = TP_REFUSED;
            }
            /* On affecte les données du code retour à transferer !!! CAS SPECIFIQUE !!!*/
            action5t_ret = CVL_C5T_3;
            F_Code5T_Set_Num(*ppCodeAns, FROM_PDT, pto->circuit, pto->id, &(action5t_ret), CODE_VALUE_LEN);
            *dst_fifo_num = FIFO_NC_CORE_CLI;
            break;
            
        default:
            couleur(C_RED); SLOGT(LOG_ERR, "\tPTO %d sur circuit %d envoi un code 5-tons non reconnu (%u)\n", pCodeRcv->id, pCodeRcv->type, action5t); couleur_r;
            /* Mise à jour du status du PTO */
            pto->status = PTO_STATUS_UNKNOWN;
            valid_trans = TP_REFUSED;

    } /* fin switch (action5t) */

    /* Mise à jour du PTO dans la liste des PTO */
    F_PTO_List_Update(&(pMain->pPtoList), pto_idx, pto->id, pto->circuit, pto->status, pto->comm_type);
    /* Si, pour une raison ou une autre, le transfert n'est pas autorisé, on réinitialise les valeurs retour */
    if (valid_trans != TP_ALLOWED) {
        *dst_fifo_num = -1;
        F_Code5T_Reset(*ppCodeAns);
    }
    couleur(C_WHITE); SLOGT(LOG_NOTICE, "PTO %d sur circuit %d, Nouveau status: %s\n", pCodeRcv->id, pCodeRcv->type, str_ptostatus(pto->status));
    free(pto);
    
    return EXIT_SUCCESS;
}



int F_GtwCore_Traite_Msg_TypeSystem(sMain * pMain, sCode5T * pCodeRcv, int src_fifo_num, sCode5T ** ppCodeAns, int * dst_fifo_num)
{
    *dst_fifo_num = -1;
    F_Code5T_Reset(*ppCodeAns);
    unsigned char syscode_val = 0xFF;
    int pto_idx = -1;
    unsigned char action5t_ret = CVL_C5T_0;
    
    /* Vérification qu'il s'agit bien d'un message de type systeme */
    if (pCodeRcv->type == CTP_SYSTEM) {
        syscode_val = pCodeRcv->code_val[0];
        switch(pCodeRcv->id) {
            case CID_FREE_CIRCUIT:      /* Message en provenance de Nc_core, à l'initiative de ce dernier, à transferer au PDT */
                if (src_fifo_num == FIFO_NC_CORE_SERV) {
                    if (syscode_val == 0) {
                        SLOGT(LOG_NOTICE, "\tLibération de tous les circuit (%u)\n", syscode_val);
                    } else {
                        couleur(C_GREEN); SLOGT(LOG_NOTICE, "\tLibération du circuit no %u\n", syscode_val - 48); couleur_r;
                        
                        sPTO * pto = NULL;
                        pto_idx = F_PTO_List_FindCircuit(pMain->pPtoList, syscode_val - 48);
                        while (pto_idx != -1) {
                            pto = F_PTO_List_GetPto(pMain->pPtoList, pto_idx);
                            pto->status = PTO_STATUS_ENDCALL;
                            F_PTO_List_Update(&(pMain->pPtoList), pto_idx, pto->id, pto->circuit, pto->status, pto->comm_type);
                            couleur(C_WHITE); SLOGT(LOG_NOTICE, "PTO %d sur circuit %d, Nouveau status: %s\n", pto->id, pto->circuit, str_ptostatus(pto->status)); couleur_r;
                            free(pto);
                            pto = NULL;
                            F_PTO_List_Remove(&(pMain->pPtoList), pto_idx);
                            pto_idx = F_PTO_List_FindCircuit(pMain->pPtoList, syscode_val - 48);
                            sleep(1);
                        }

                        *dst_fifo_num = FIFO_PDT_CLI;
                        action5t_ret = syscode_val;
                        F_Code5T_Set_Num(*ppCodeAns, TO_PDT, CTP_SYSTEM, CID_FREE_CIRCUIT, &(action5t_ret), CODE_VALUE_LEN);
                    
                        /* Terminaison de la comm audio sur le circuit */
                        F_ProcAudio_Set_ActiveAudio(pMain->pAudioCircuit, syscode_val - 48, CC_AUDIO_STATUS_INACTIVE);
                    }
                } else {
                    SLOGT(LOG_ERR, "\tMessage reçu sur la mauvaise socket (%d). Rejeté !\n", src_fifo_num);
                }
                break;

            case CID_HEARTBEAT:
                SLOGT(LOG_NOTICE, "\tEnvoi de la trame de vie (0x%02X)\n", syscode_val);
                SLOGT(LOG_WARNING, "\tCeMessage ne devrait pas être reçu sur une socket. Il est envoyé spontanément par la gateway à intervalles régulier\n");
                break;

            case CID_HEARTBEAT_ACK:     /* Message recu sur la socket cliente PDT */
                if (src_fifo_num == FIFO_PDT_CLI) {
                    SLOGT(LOG_NOTICE, "\tRéponse à la trame de vie (reçu 0x%02X)\n", syscode_val);
                    if (F_HB_GetStatus(pMain->pHB) != HB_DEAD) {
                        /* Mise à jour du compteur de vie reçu */
                        F_HB_ReceiveValue(pMain->pHB, syscode_val);
                        F_HB_UpdateStatus(pMain->pHB);
                    } else {
                        SLOGT(LOG_WARNING, "\tRejetée car status heartbeat = HB_DEAD\n");
                    }
                } else {
                    SLOGT(LOG_ERR, "\tRéponse à la trame de vie reçue sur la mauvaise socket (%d). Rejetée !\n", src_fifo_num);
                }
                break;
                
            case CID_LAST_TERM:     /* Message recu sur la socket cliente PDT */
                if (src_fifo_num == FIFO_PDT_CLI) {
                    if(syscode_val == CVL_C5T_8) {
                        SLOGT(LOG_NOTICE, "\tDernière borne\n");
                        /* Mise à jour de l'identification du poste de tête */
                        F_Set_PDTIdent(pMain, LASTTERM);
                    } else {
                        SLOGT(LOG_WARNING, "\tDernière borne MESSAGE CORROMPU\n");
                    }
                }
                break;
                
            case CID_BIBL_STATUS:   /* Message recu sur la socket serveur Nc_core */
                if (src_fifo_num == FIFO_NC_CORE_SERV) {
                    SLOGT(LOG_NOTICE, "\tCarte BIBL HS ou manquante\n");
                }
                break;
                
            case CID_PDT_START:     /* Message recu sur la socket serveur PDT */
                if (src_fifo_num == FIFO_PDT_SERV) {
                    SLOGT(LOG_NOTICE, "\tCode de fonctionnement au démarrage\n");
                    /* On réinitialise le flag d'inhibition des traitements */
                    F_Set_PDTIdent(pMain, MASTER);
                    F_Set_PDTStatus(pMain, PDT_STATUS_OK);
                    
                    /* On nettoie les messages contenus dans la file d'envoi */
                    if (F_SockQueue_GetNbElem(pMain->pSocketsCli[SOCK_DATA]->snd_queue) > 0)
                        F_SockQueue_Clear(&(pMain->pSocketsCli[SOCK_DATA]->snd_queue));
                    
                    F_HB_Reset(pMain->pHB);
                    SLOGT(LOG_NOTICE, "\tReset de la trame de vie\n");
                    if (F_Get_InhibStatus(pMain) == 1) {
                        SLOGT(LOG_NOTICE, "\tLevée de l'inhibition des traitements\n");
                        F_Set_InhibStatus(pMain, 0);
                    }
                }
                break;
                
            default:
                SLOGT(LOG_ERR, "\tIdentifiant de message non reconnu...\n");
                break;
        } /* Fin switch(pCodeRcv->id) */
    } /* Fin if (pCodeRcv->type == CTP_SYSTEM) */
    
    return EXIT_SUCCESS;
}

int F_GtwCode_RemovePTOInactives(sMain * pMain)
{
    int i = 0;
    int nb_elem = -1;
    sPTO * pto = NULL;
    nb_elem = F_PTO_List_GetNbElements(pMain->pPtoList);
    //F_PTO_List_DisplayContent(pMain->pPtoList);
    
    for (i = 0; i < nb_elem; i++) {
        pto = F_PTO_List_GetPto(pMain->pPtoList, i);
        if (   (pto->status == PTO_STATUS_ENDCALL)
            || (pto->status == PTO_STATUS_UNKNOWN)
            || (pto->status == PTO_STATUS_OFFLINE)
           ){
            SLOGT(LOG_NOTICE, "Suppression PTO idx %d\n", i);
            F_PTO_List_Remove(&(pMain->pPtoList), i);
            if (F_PTO_List_GetNbElements(pMain->pPtoList) > 0)
                i--;
        }
    }
    return EXIT_SUCCESS;
}
