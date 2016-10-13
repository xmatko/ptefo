/*!
 * \file main.c
 * \brief Fichier principal du programme
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 25/06/2014
 *
 * Client / Serveur UDP de communication avec les postes de tête
 *
 * S:\\be\\Sncf\\PTE FO IFOTEC\\E_Développement matériel\\Spécifications\\Diffussion\n
 * S:\\be\\Sncf\\PTE FO IFOTEC\\5tons.pdf\n
 * S:\\be\\Sncf\\PTE FO IFOTEC\\D_Développement logiciel\\Programmation\\Prog Poste De Tete\n
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"
#include "tlog.h"
#include "code5t_proc.h"
#include "config.h"
#include "server_udp.h"
#include "client_udp.h"
#include "client_tcp.h"
#include "server_tcp.h"
#include "gtw_core.h"
#include "main.h"

#include "zaptel/trunk/zaptel.h"
#include "zaptel/trunk/pto4M.h"




/*!
 * \def PROG_NAME
 * \brief Nom du programme
 */
#define PROG_NAME   "comm_ucc_pdt"
/*!
 * \def PROG_VER
 * \brief Version du programme
 */
#define PROG_VER    "0.1.0"
/*!
 * \def PROG_DATE
 * \brief Date du programme
 */
#define PROG_DATE   "2014-06-02"


static int fd          = -1;
typedef struct pto4M_audio sPTO4M_Audio;
static char rcv_msg[BUF_SIZE * sizeof (char)] = {0};
static char snd_msg[BUF_SIZE * sizeof (char)] = {0};
static sPTO4M_Audio d_samples_rd_space;
static sPTO4M_Audio d_samples_wr_space;
static sPTO4M_Audio * d_samples_rd = &d_samples_rd_space;
static sPTO4M_Audio * d_samples_wr = &d_samples_wr_space;
static short d_tmp[ZT_CHUNKSIZE] = {0};
int traitement_audio(sMain * pMain);
static char exit_flag = 0;


/*!
 * \fn static int main_init(sMain ** pMain)
 * \brief Initialisation de la stucture principale du programme
 * \param pMain: pointeur sur structure sMain
 * \return EXIT_SUCCESS
 *
 * \warning Cette fonction initialise les pointeurs contenus dans la structure à NULL
 * mais ne les alloue pas. Les allocation mémoire de ces différents pointeurs se fait
 * lors de l'appel de leur fonction d'init respectives.
 */
static int main_init(sMain ** pMain)
{
    *pMain = (sMain *)malloc(sizeof (sMain));
    (*pMain)->pConfig     = NULL;
    (*pMain)->pSockets    = NULL;
    (*pMain)->pSocketsCli = NULL;
    (*pMain)->pCodes      = NULL;
    (*pMain)->pHB         = NULL;
    F_HB_Init(&((*pMain)->pHB));
    (*pMain)->inhib_traitements = 0;
    (*pMain)->ident_pdt = MASTER;
    (*pMain)->status_pdt = PDT_STATUS_OK;
    (*pMain)->nc_cli_snd_queue = NULL;
    (*pMain)->nc_serv_rcv_queue = NULL;
    
    F_SockQueue_Init(&((*pMain)->nc_cli_snd_queue), 100, "file de messages à envoyer vers nc_core");
    F_SockQueue_Init(&((*pMain)->nc_serv_rcv_queue), 100, "file de messages reçus depuis nc_core");

    (*pMain)->dumb = 0;
    (*pMain)->pAudioCircuit = NULL;

    F_PTO_List_Init(&((*pMain)->pPtoList));
    
    return EXIT_SUCCESS;
}

/*!
 * \fn static int clean_exit(sMain * pMain)
 * \brief Function de sortie propre du programme
 * \param pMain: pointeur sur structure sMain
 * \return EXIT_SUCCESS
 *
 * Cette fonction appelle les fonctions spécifiques de libération
 * de la mémoire allouées aux différents pointeurs
 */
static int clean_exit(sMain * pMain)
{
    int ret = -1;

    /* Ferme les sockets serveur et clientes et libére la mémoire allouée aux pointeurs de type sSockTab (sockets) */
    if ( (ret = F_Socket_DeleteAll(&(pMain->pSockets), pMain->pConfig->pServ->nb_sockets)) != 0)
        die("F_Socket_DeleteAll");

    if ( (ret = F_Socket_DeleteAll(&(pMain->pSocketsCli), pMain->pConfig->pCli->nb_sockets)) != 0)
        die("F_Socket_DeleteAll");

    /* Debug pointeurs
    printf("EXIT pMain->pCodes %x\n", (unsigned long)(pMain->pCodes));
    
    printf("EXIT pMain->pCodes[0] %x\n", (unsigned long)(pMain->pCodes[0]));
    printf("EXIT *(pMain->pCodes) %x\n", (unsigned long)*(pMain->pCodes));
    
    printf("EXIT pMain->pCodes[1] %x\n", (unsigned long)(pMain->pCodes[1]));
    printf("EXIT *(pMain->pCodes + 1) %x\n", (unsigned long)*(pMain->pCodes + 1));
    
    printf("EXIT pMain->pCodes[2] %x\n", (unsigned long)(pMain->pCodes[2]));
    printf("EXIT *(pMain->pCodes + 2) %x\n", (unsigned long)*(pMain->pCodes + 2));
    */
    
    /* Debug pointeurs
    printf("FINAL pMain->pSockets = %x\n", pMain->pSockets);
    printf("FINAL pMain->pSocketsCli = %x\n", pMain->pSocketsCli);
    printf("FINAL pMain->pCodes %x\n", (unsigned long)(pMain->pCodes));
    printf("FINAL pMain->pCodeRcv = %x\n", pMain->pCodeRcv);
    */
    
    /* En sortie des fonctions de libération appellées ci-avant, 
       les pointeurs passés en paramètres doivent être nuls */
    if ((pMain->pSockets != NULL)
        || (pMain->pSocketsCli != NULL)
        || (pMain->pCodes != NULL)
        ) {
            SLOGT(LOG_ERR, "Erreur de libération d'un des pointeurs\n");
        }
     
    SLOGT(LOG_NOTICE, "Exit (clean)\n");
    F_NLog_Close();
    couleur_r; fflush(stdout);
    return EXIT_SUCCESS;
}

/*!
 * \fn static void * Th_IntHandler(void * arg)
 * \brief Signal handler thread
 * \param arg: pointeur sur structure sMain
 */
static void * Th_IntHandler(void * arg)
{
    int sig = -1;
    int i = -1;
    int nb_audio_threads = 0;
    sMain * pMain = (sMain *)arg;
    sigset_t * set = (sigset_t *)&(pMain->set);
    
    nb_audio_threads = DEFAULT_NB_SOCKS - 1;
    while (sig < 0)
    {
        sigwait(set, &sig);
        SLOGT(LOG_INFO, "\n\nSignal handling thread got signal %d\n", sig);
        switch (sig) {
            case SIGINT:
                exit_flag = 1;
                SLOGT(LOG_INFO, "Cancel running thread to prepare clean exit\n");
                pthread_cancel(pMain->th_GtwCore);
                pthread_cancel(pMain->th_ClientTCP);
                pthread_cancel(pMain->th_ServerTCP);
                pthread_cancel(pMain->th_ClientUDPData);
                pthread_cancel(pMain->th_ProcessAudio);
                pthread_cancel(pMain->th_ServeurUDP);
                for (i = 0; i < nb_audio_threads; i++)
                    pthread_cancel(pMain->th_ClientUDPAudio[i]);
                break;
            default:
                sig = -1;
        }
        usleep(DEFAULT_THREAD_SLEEP_US);
    }
    
    SLOGT(LOG_DEBUG, "Fin thread Th_IntHandler\n");
    pthread_exit(0);
}


/*!
 * \fn int main(int argc, char *argv[])
 * \brief Point d'entrée du programme
 * \return 0 si aucune erreur, -1 en cas d'erreur
 *
 * - Initialisations\n
 * - Ouverture de la socket serveur\n
 * - Boucle de réception\n
 *
 */
int main(int argc, char *argv[])
{
    int ret, i;
    int isthok = -1;
    int nb_audio_threads = DEFAULT_NB_SOCKS - 1;
    pthread_t           th_SignalHander;
    pthread_attr_t      th_SignalHander_attr;
    pthread_t           th_HB_Mgt;
    pthread_attr_t      th_HB_Mgt_attr;
    void * res_th = NULL;
    sThArgCliAudio  ** th_cli_audio_arg = NULL;
    sMain * pMain = NULL;

    main_init(&pMain);
    F_NLog_Init(PROG_NAME, NLOG_DEST_STDOUT|NLOG_DEST_SYSLOG, LOG_DEBUG);
    F_Config_Init(&(pMain->pConfig));
    if ((ret = F_Config_Read_CmdLineArgs(pMain->pConfig, argc, argv)) != EXIT_SUCCESS) die("read_cmdarg()");
    F_NLog_SetLogLevel(pMain->pConfig->verbose_lvl);
    if (gethostname((char *)pMain->pConfig->pServ->hostname, HOSTNAME_MAX_LEN * sizeof (pMain->pConfig->pServ->hostname)))  die("gethostname()");
    couleur(C_GREEN); SLOGT(LOG_NOTICE, "== %s ==\n", PROG_NAME); couleur_r;
    F_Config_Disp(pMain->pConfig);
    
    //F_Code5T_TestUsage();
    //F_PTO_Test_Usage();
    //exit(0);
    
    /* Creation des sockets UDP serveur */
    F_Socket_CreateAll (&(pMain->pSockets), pMain->pConfig->pServ->base_port, pMain->pConfig->pServ->nb_sockets);

    /* Creation des sockets UDP clientes */
    F_SocketCli_CreateAll(&(pMain->pSocketsCli), (char *)pMain->pConfig->pCli->remote_server_ip, 
                                                 pMain->pConfig->pCli->remote_server_base_port,
                                                 pMain->pConfig->pCli->reply_timeout,
                                                 pMain->pConfig->pCli->nb_sockets);

    /* Initialisation de la gestion des signaux */
    sigemptyset(&(pMain->set));
    sigaddset(&(pMain->set), SIGINT);
    sigaddset(&(pMain->set), SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &(pMain->set), NULL);
    isthok = 0;
    if (pthread_attr_init(&th_SignalHander_attr) != 0) isthok = 1;
    if( pthread_create(&th_SignalHander, &th_SignalHander_attr, Th_IntHandler, (void *)pMain) < 0 ) isthok = 1;
    if(isthok != 0) SLOGT(LOG_CRIT, "Erreur lancement thread th_SignalHander\n");
    
    /* Lancement du thread de gestion du compteur de vie */
    isthok = 0;
    if (pthread_attr_init(&th_HB_Mgt_attr) != 0) isthok = 1;
    if( pthread_create(&th_HB_Mgt, &th_HB_Mgt_attr, F_HB_Mgt, (void *)pMain) < 0 ) isthok = 1;
    if(isthok != 0)  SLOGT(LOG_CRIT, "Erreur lancement thread th_HB_Mgt\n");

    /* Lancement du thread client TCP */
    isthok = 0;
    if (pthread_attr_init(&(pMain->th_ClientTCP_attr)) != 0) isthok = 1;
    if (pthread_attr_setdetachstate(&(pMain->th_ClientTCP_attr), PTHREAD_CREATE_JOINABLE) != 0) isthok = 1;
    if( pthread_create(&(pMain->th_ClientTCP), &(pMain->th_ClientTCP_attr), F_Th_TCPClient, (void *)pMain) < 0 ) isthok = 1;
    if(isthok != 0)  SLOGT(LOG_CRIT, "Erreur lancement thread th_ClientTCP\n");

    /* Lancement du thread serveur TCP */
    isthok = 0;
    if (pthread_attr_init(&(pMain->th_ServerTCP_attr)) != 0)  isthok = 1;
    if (pthread_attr_setdetachstate(&(pMain->th_ServerTCP_attr), PTHREAD_CREATE_JOINABLE) != 0)  isthok = 1;
    if( pthread_create(&(pMain->th_ServerTCP), &(pMain->th_ServerTCP_attr), F_Th_TCPServer, (void *)pMain) < 0 ) isthok = 1;
    if(isthok != 0)  SLOGT(LOG_CRIT, "Erreur lancement thread th_ClientTCP\n");

    /* Lancement du thread du serveur UDP */
    isthok = 0;
    if (pthread_attr_init(&(pMain->th_ServeurUDP_attr)) != 0)  isthok = 1;
    if (pthread_attr_setdetachstate(&(pMain->th_ServeurUDP_attr), PTHREAD_CREATE_JOINABLE) != 0)  isthok = 1;
    if( pthread_create(&(pMain->th_ServeurUDP), &(pMain->th_ServeurUDP_attr), F_Th_UDPServer, (void *)pMain) < 0 )  isthok = 1;
    if(isthok != 0)  SLOGT(LOG_CRIT, "Erreur lancement thread th_ServeurUDP\n");

    /* Lancement du thread du client UDP data */
    isthok = 0;
    if (pthread_attr_init (&(pMain->th_ClientUDPData_attr)) != 0)  isthok = 1;
    if (pthread_attr_setdetachstate (&(pMain->th_ClientUDPData_attr), PTHREAD_CREATE_JOINABLE) != 0)  isthok = 1;
    if (pthread_create(&(pMain->th_ClientUDPData), &(pMain->th_ClientUDPData_attr), F_Th_UDPClient_Data, (void *)pMain) < 0)  isthok = 1;
    if(isthok != 0) SLOGT(LOG_CRIT, "Erreur lancement thread th_ClientUDPData\n");
    
    if (nb_audio_threads > 0) {
        /* Lancement du thread de traitement audio */
        F_ProcAudio_Init_ActiveAudio(&(pMain->pAudioCircuit), nb_audio_threads);
        
        
        isthok = 0;
        if (pthread_attr_init (&(pMain->th_ProcessAudio_attr)) != 0)  isthok = 1;
        if (pthread_attr_setdetachstate (&(pMain->th_ProcessAudio_attr), PTHREAD_CREATE_JOINABLE) != 0)   isthok = 1;
        if (pthread_create(&(pMain->th_ProcessAudio), &(pMain->th_ProcessAudio_attr), F_Th_Process_Audio, (void *)pMain) < 0)  isthok = 1;
        if(isthok != 0)  SLOGT(LOG_CRIT, "Erreur lancement thread th_ProcessAudio\n");
        
        
        th_cli_audio_arg = (sThArgCliAudio **)malloc (nb_audio_threads * sizeof (sThArgCliAudio *));
        pMain->th_ClientUDPAudio = (pthread_t *)malloc (nb_audio_threads * sizeof(pthread_t));
        pMain->th_ClientUDPAudio_attr = (pthread_attr_t *)malloc (nb_audio_threads * sizeof(pthread_attr_t));

        for (i = 0; i < nb_audio_threads; i++) {
            /* Lancement des threads client UDP audio */
            //SLOGT(LOG_INFO, "Lancement thread th_ClientUDPAudio num %d\n", i);
            th_cli_audio_arg[i] = (sThArgCliAudio *)malloc(sizeof (sThArgCliAudio));
            th_cli_audio_arg[i]->pMain = (void *)pMain;
            th_cli_audio_arg[i]->th_num = i;

            isthok = 0;
            if (pthread_attr_init ((pMain->th_ClientUDPAudio_attr + i)) != 0) isthok = 1;
            if (pthread_attr_setdetachstate ((pMain->th_ClientUDPAudio_attr + i), PTHREAD_CREATE_JOINABLE) != 0) isthok = 1;
            if (pthread_create((pMain->th_ClientUDPAudio + i), (pMain->th_ClientUDPAudio_attr + i), F_Th_UDPClient_Audio, (void *)th_cli_audio_arg[i]) < 0) isthok = 1;
            if(isthok != 0) SLOGT(LOG_CRIT, "Erreur lancement thread th_ClientUDPAudio num %d\n", i);
        } /* Fin for(nb_audio_threads) */
    } /* Fin if (nb_audio_threads > 0) */

    /* Lancement du thread de traitement principal de la gateway */
    isthok = 0;
    if (pthread_attr_init (&(pMain->th_GtwCore_attr)) != 0) isthok = 1;
    if (pthread_attr_setdetachstate (&(pMain->th_GtwCore_attr), PTHREAD_CREATE_JOINABLE) != 0) isthok = 1;
    if (pthread_create(&(pMain->th_GtwCore), &(pMain->th_GtwCore_attr), F_Th_GtwCore, (void *)pMain) < 0)  isthok = 1;
    if(isthok != 0) SLOGT(LOG_CRIT, "Erreur lancement thread th_ClientUDPData\n");


    /**************************************************************************************/
    /** A CE POINT, TOUS LES THREADS SONT DEMARRES                                       **/ 
    /**************************************************************************************/
    while (!exit_flag)
    {
        traitement_audio(pMain);
    }

    
    
    //* Attente de la fin des threads */
    pthread_join(pMain->th_GtwCore, &res_th);
    pthread_join(pMain->th_ClientTCP, &res_th);
    pthread_join(pMain->th_ServerTCP, &res_th);
    pthread_join(pMain->th_ServeurUDP, &res_th);
    pthread_join(pMain->th_ClientUDPData, &res_th);
    pthread_join(pMain->th_ProcessAudio, &res_th);

    if (nb_audio_threads > 0) {
        for (i = 0; i < nb_audio_threads; i++)
            pthread_join(pMain->th_ClientUDPAudio[i], &res_th);
        free(th_cli_audio_arg);
    } /* Fin if (nb_audio_threads > 0) */
    
    /* Terminaison propre du programme */
    clean_exit(pMain);
    return EXIT_SUCCESS;
}

