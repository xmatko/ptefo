/*!
 * \file main.h
 * \brief Fichier d'entête principal du programme
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 25/06/2014
 *
 * Client / Serveur UDP de communication avec les postes de tête
 * 
 */

#ifndef _COMM_PDT_H_
#define _COMM_PDT_H_

#include <pthread.h>

#include "config.h"
#include "heartbeat.h"
#include "socks_mgt.h"
#include "socks_tcp_mgt.h"
#include "code5t.h"
#include "process_audio.h"
#include "pto_mgt.h"

/*!
 * \def DEFAULT_THREAD_SLEEP_US
 * \brief Temporisation par déaut de mise en veille des threads à chaque boucle de traitement, en microsecondes
 */
#define DEFAULT_THREAD_SLEEP_US  50000
#define DEFAULT_THREAD_SLEEP_NS  20000000 /* 20 ms */


enum tIdenPDT {
    MASTER      = 1,
    LASTTERM    = 2
};

enum tStatusPDT {
    PDT_STATUS_OK,
    PDT_STATUS_HS
};

/*!
 * \struct sMain
 * \brief Structure de donnée principale du programme
 *
 * Cette structure contient l'ensemble des références aux autres structures utilisées
 * dans le programme.
 */
typedef struct tMain 
{
    sConfigMain      *  pConfig;                /*!< Pointeur sur la configuration du serveur */
    sSockTab        **  pSockets;               /*!< Tableau de pointeurs sur les sockets gérées par le serveur */
    sSockTab        **  pSocketsCli;            /*!< Tableau de pointeurs sur les sockets clientes */
    sCode5T         **  pCodes;                 /*!< Tableau de pointeurs sur les codes 5-tons connus */
    sHeartbeat       *  pHB;                    /*!< Pointeur sur structure de heartbeat */
    char                inhib_traitements;      /*!< Flag d'inhibition du traitement des échanges lorsque PDT HS (signal de vie) */
    enum tIdenPDT       ident_pdt;
    enum tStatusPDT     status_pdt;
    sAudioStatus     *  pAudioCircuit;
    sPTO_List        *  pPtoList;               /*!< Liste des PTO en traitement */
    timer_t             gtw_core_timerid;
    pthread_t           th_ServeurUDP;          /*!< Thread principal du serveur */
    pthread_attr_t      th_ServeurUDP_attr;     /*!< Attributs du thread principal du serveur  */

    pthread_t           th_ClientUDPData;       /*!< Thread de gestion des données du client */
    pthread_attr_t      th_ClientUDPData_attr;  /*!< Attributs du thread de gestion des données du client */

    pthread_t           th_GtwCore;             /*!< Thread de traitement et de routage des messages reçus */
    pthread_attr_t      th_GtwCore_attr;        /*!< Attributs du thread de traitement et de routage des messages reçus */
    
    pthread_t         * th_ClientUDPAudio;     /*!< Threads de gestion de l'audio du client */
    pthread_attr_t    * th_ClientUDPAudio_attr;    /*!< Attributs des threads de gestion de l'audio du client */
    struct sched_param  th_ClientUDPAudio_sched_param;

    pthread_t           th_ProcessAudio;       /*!< Threads de gestion de l'audio du serveur */
    pthread_attr_t      th_ProcessAudio_attr;  /*!< Attributs des threads de gestion de l'audio du serveur */
    struct sched_param  th_ProcessAudio_sched_param;
    
    pthread_t           th_ClientTCP;           /*!< Thread de gestion du client TCP (communication vers nc_core à notre initiative) */
    pthread_attr_t      th_ClientTCP_attr;      /*!< Attribut du thread de gestion du client TCP */
    
    pthread_t           th_ServerTCP;           /*!< Thread de gestion du serveur TCP (communication depuis nc_core à l'initiative de ce dernier) */
    pthread_attr_t      th_ServerTCP_attr;      /*!< Attribut du thread de gestion du serveur TCP */
    
    sSockFifo        *  nc_cli_snd_queue;       /*!< File d'attente des messages à envoyer à nc_core */
    sSockFifo        *  nc_serv_rcv_queue;      /*!< File d'attente des messages reçus de nc_core */

    sigset_t            set;                    /*!< Set de signaux pour la gestion des signaux par le thread SignalHander */
    int                 dumb;                   /*!< Dumb variable. Unused */
} sMain;

#endif /* _COMM_PDT_H_ */
