/*!
 * \file config.c
 * \brief Gestion de la configuration du programme
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 25/06/2014
 *
 * Client / Serveur UDP de communication avec les postes de tête
 * 
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

#include "common.h"
#include "tlog.h"
#include "config.h"
#include "main.h"

/*!
 * \def MILLISECONDES
 * \brief Multiplicateur pour convertir des millisecondes en secondes
 */
#define MILLISECONDES 1000
#define ONE_MILLION   1000000
 
/*!
 * \fn static void usage(void)
 * \brief Affichage du message d'aide à l'utilisation
 */
static void usage(void)
{
    printf("\nUsage:\n");
    printf("comm_ucc_pdt [options]\n");

    printf("\t-l\tHearbeat timeout (seconds)\n");
    printf("\t-v\tNiveau de verbosité (de 0 à 7)\n");
    printf("\n   * Options serveur\n");
    printf("\t-p\tPort de base du serveur\n");
    printf("\t-n\tNombre de sockets à ouvrir\n");

    printf("\n   * Options client\n");
    printf("\t-a\tAdresse IP du serveur distant\n");
    printf("\t-d\tPort du serveur distant\n");
    printf("\t-e\tEnvoi des messages de taille et de contenu aléatoire\n");
    printf("\t-w\tWait for a reply from server\n");
    printf("\t-t\tTimeout sur réponse du serveur\n");
    
    
    
    printf("\n\t-h\tAffiche ce message d'aide\n");
    printf("\n");
}

/*!
 * \fn static int init_server_config(sConfigServer ** pConfServ)
 * \brief Initialisation de la structure de config spécifique serveur
 * \param pConfServ: adresse du pointeur sur la structure à initialiser
 * \return EXIT_SUCCESS
 *
 * En sortie de la fonction, le pointeur sur la structure est alloué et son
 * contenu initialisé
 */
static int init_server_config(sConfigServer ** pConfServ)
{
    sConfigServer * pTmp = (sConfigServer *)malloc(sizeof (sConfigServer));
    pTmp->base_port = DEFAULT_SERVER_PORT;
    pTmp->nb_sockets = DEFAULT_NB_SOCKS;
    pTmp->hostname = (unsigned char *)malloc(HOSTNAME_MAX_LEN * sizeof (unsigned char));
    memset(pTmp->hostname, 0, HOSTNAME_MAX_LEN * sizeof (unsigned char));
    pTmp->l_timeout.tv_sec  = 0;
    pTmp->l_timeout.tv_usec = DEFAULT_LISTEN_TIMEOUT_MS * MILLISECONDES;
    
    *pConfServ = pTmp;
    
    return EXIT_SUCCESS;
}

static int init_server_tcp_config(sConfigServerTCP ** pConfServTCP)
{
    sConfigServerTCP * pTmp = (sConfigServerTCP *)malloc(sizeof (sConfigServerTCP));
    pTmp->port = DEFAULT_TCP_SERVER_PORT;
    pTmp->nb_max_clients = DEFAULT_TCP_SERVER_MAX_CLIENTS;
    pTmp->hostname = (unsigned char *)malloc(HOSTNAME_MAX_LEN * sizeof (unsigned char));
    memset(pTmp->hostname, 0, HOSTNAME_MAX_LEN * sizeof (unsigned char));
    
    pTmp->l_timeout.tv_sec  = 0;
    pTmp->l_timeout.tv_usec = 0;

    *pConfServTCP = pTmp;
    return EXIT_SUCCESS;
}

static int init_client_tcp_config(sConfigClientTCP ** pConfCliTCP)
{
    sConfigClientTCP * pTmp = (sConfigClientTCP *)malloc(sizeof (sConfigClientTCP));
    pTmp->remote_server_ip = (unsigned char *)malloc(ADDR_IP_LEN * sizeof (unsigned char));
    memset(pTmp->remote_server_ip, 0, ADDR_IP_LEN * sizeof (unsigned char));
    memcpy(pTmp->remote_server_ip, DEFAULT_REMOTE_TCP_SERVER_IP, ADDR_IP_LEN * sizeof (unsigned char));
    pTmp->remote_server_port = DEFAULT_REMOTE_TCP_SERVER_PORT;
    if (DEFAULT_TCP_CLI_RCV_TIMEOUT_MS * MILLISECONDES >= ONE_MILLION) {
        pTmp->reply_timeout.tv_sec = (int)(DEFAULT_TCP_CLI_RCV_TIMEOUT_MS / MILLISECONDES);
        pTmp->reply_timeout.tv_usec = 0;
    } else {
        pTmp->reply_timeout.tv_sec = 0;
        pTmp->reply_timeout.tv_usec = DEFAULT_TCP_CLI_RCV_TIMEOUT_MS * MILLISECONDES;    
    }
    if (DEFAULT_TCP_CLI_SND_TIMEOUT_MS * MILLISECONDES >= ONE_MILLION) {
        pTmp->send_timeout.tv_sec = (int)(DEFAULT_TCP_CLI_SND_TIMEOUT_MS / MILLISECONDES);
        pTmp->send_timeout.tv_usec = 0;
    } else {
        pTmp->send_timeout.tv_sec = 0;
        pTmp->send_timeout.tv_usec = DEFAULT_TCP_CLI_SND_TIMEOUT_MS * MILLISECONDES;
    }
    pTmp->retry_delay = DEFAUT_TCP_CLI_RETRY_DELAY_SEC;
    pTmp->max_retry = DEFAUT_TCP_CLI_MAX_RETRY;
    
    *pConfCliTCP = pTmp;
    return EXIT_SUCCESS;
}

/*!
 * \fn static int init_client_config(sConfigClient ** pConfCli)
 * \brief Initialisation de la structure de config spécifique client
 * \param pConfCli: adresse du pointeur sur la structure à initialiser
 * \return EXIT_SUCCESS
 *
 * En sortie de la fonction, le pointeur sur la structure est alloué et son
 * contenu initialisé
 */
static int init_client_config(sConfigClient ** pConfCli)
{
    sConfigClient * pTmp = (sConfigClient *)malloc(sizeof (sConfigClient));
    pTmp->remote_server_ip = (unsigned char *)malloc(ADDR_IP_LEN * sizeof (unsigned char));
    memset(pTmp->remote_server_ip, 0, ADDR_IP_LEN * sizeof (unsigned char));
    memcpy(pTmp->remote_server_ip, DEFAULT_REMOTE_SERVER_IP, ADDR_IP_LEN * sizeof (unsigned char));
    pTmp->remote_server_base_port = DEFAULT_REMOTE_SERVER_PORT;
    pTmp->nb_sockets = DEFAULT_NB_SOCKS;
    pTmp->random_msg = 0;
    pTmp->waitreply = 0;
    pTmp->reply_timeout.tv_sec  = 0;
    pTmp->reply_timeout.tv_usec = DEFAULT_REPLY_TIMEOUT_MS * MILLISECONDES;
    *pConfCli = pTmp;
    return EXIT_SUCCESS;
}

/*!
 * \fn int F_Config_Init(sConfigMain ** pConf)
 * \brief Initialisation de la structure de config principale
 * \param pConf: adresse du pointeur sur la structure de config à initialiser
 * \return EXIT_SUCCESS
 *
 * En sortie de la fonction, le pointeur sur la structure est alloué et son
 * contenu initialisé
 */
int F_Config_Init(sConfigMain ** pConf)
{
    sConfigMain * pTmp = (sConfigMain *)malloc(sizeof (sConfigMain));
    init_server_config(&(pTmp->pServ));
    init_client_config(&(pTmp->pCli));
    init_server_tcp_config(&(pTmp->pServTCP));
    init_client_tcp_config(&(pTmp->pCliTCP));
    pTmp->hb_timeout = DEFAULT_HB_TIMEOUT;
    pTmp->verbose_lvl = DEFAULT_VERB_LVL;
    *pConf = pTmp;
    return EXIT_SUCCESS;
}

void F_Config_Disp(sConfigMain * pConf)
{
    couleur(C_MAGENTA); SLOGT(LOG_NOTICE, "Configuration:\n");
    couleur(C_YELLOW); SLOGT(LOG_NOTICE, "  [Main]\n"); couleur(C_BLUE);
    SLOGT(LOG_NOTICE, "      Heartbeat timeout      : %d s\n", pConf->hb_timeout);
    SLOGT(LOG_NOTICE, "      Log verbose level      : %d\n", pConf->verbose_lvl);
    couleur(C_YELLOW); SLOGT(LOG_NOTICE, "\n  [Serveur UDP]\n"); couleur(C_BLUE);
    SLOGT(LOG_NOTICE, "      UDP server base port   : %d\n", pConf->pServ->base_port);
    SLOGT(LOG_NOTICE, "      Nb de sockets          : %d\n", pConf->pServ->nb_sockets);
    SLOGT(LOG_NOTICE, "      Nom hote               : %s\n", pConf->pServ->hostname);
    SLOGT(LOG_NOTICE, "      Listen timeout         : %d s %d ms\n", (int)pConf->pServ->l_timeout.tv_sec, (int)(pConf->pServ->l_timeout.tv_usec / MILLISECONDES));
    couleur(C_YELLOW); SLOGT(LOG_NOTICE, "\n  [Client UDP]\n"); couleur(C_BLUE);
    SLOGT(LOG_NOTICE, "      Remote server IP       : %s\n", pConf->pCli->remote_server_ip);
    SLOGT(LOG_NOTICE, "      Remote server port     : %u\n", pConf->pCli->remote_server_base_port);
    SLOGT(LOG_NOTICE, "      Send random messages   : %d\n", pConf->pCli->random_msg);
    SLOGT(LOG_NOTICE, "      Wait for a reply       : %d\n", pConf->pCli->waitreply);
    SLOGT(LOG_NOTICE, "      Reply timeout          : %d s %d ms\n", (int)pConf->pCli->reply_timeout.tv_sec, (int)(pConf->pCli->reply_timeout.tv_usec / MILLISECONDES));
    couleur(C_YELLOW); SLOGT(LOG_NOTICE, "\n  [Serveur UDP]\n"); couleur(C_BLUE);
    SLOGT(LOG_NOTICE, "      Nom hote               : %s\n", pConf->pServTCP->hostname);
    SLOGT(LOG_NOTICE, "      Port d'écoute          : %d\n", pConf->pServTCP->port);
    SLOGT(LOG_NOTICE, "      Nb max clients         : %d\n", pConf->pServTCP->nb_max_clients);
    couleur(C_YELLOW); SLOGT(LOG_NOTICE, "\n  [Client TCP]\n"); couleur(C_BLUE);
    SLOGT(LOG_NOTICE, "      Remote server IP       : %s\n", pConf->pCliTCP->remote_server_ip);
    SLOGT(LOG_NOTICE, "      Remote server port     : %u\n", pConf->pCliTCP->remote_server_port);
    SLOGT(LOG_NOTICE, "      Connect timeout        : %d s %d ms\n", (int)pConf->pCliTCP->send_timeout.tv_sec, (int)(pConf->pCliTCP->send_timeout.tv_usec / MILLISECONDES));
    SLOGT(LOG_NOTICE, "      Retry delay            : %d s\n", pConf->pCliTCP->retry_delay);
    SLOGT(LOG_NOTICE, "      Max retry              : %d s\n", pConf->pCliTCP->max_retry);
    SLOGT(LOG_NOTICE, "      Reply timeout          : %d s %d ms\n\n", (int)pConf->pCliTCP->reply_timeout.tv_sec, (int)(pConf->pCliTCP->reply_timeout.tv_usec / MILLISECONDES));
    couleur_r; fflush(stdout);

}

/*!
 * \fn int F_Config_Read_CmdLineArgs(sConfigMain * pConf, int argc, char * argv[])
 * \brief Lecture et traitement des arguments passés en ligne de commande
 * \param pConf: adresse du pointeur sur la structure de configuration sConfigServer
 * \param argc: nombre d'arguements passés en ligne de commande
 * \param argv: pointeur sur le tableau des arguments passés en ligne de commande
 * \return 0 si OK, -1 en cas d'erreur
 */
int F_Config_Read_CmdLineArgs(sConfigMain * pConf, int argc, char * argv[])
{
    //sConfigServer * pConfL = NULL;
    //pConfL = (sConfigServer *)malloc(sizeof (sConfigServer));
    int c = -1;
    sConfigServer * pServer = pConf->pServ;
    sConfigClient * pCli = pConf->pCli;
    while ( (c = getopt(argc, argv, "p:n:a:d:t:v:l:rewh") ) != -1) {
        switch (c) {
            /* Port de base du serveur */
            case 'p':
                if (opt_isnum(optarg) == 1) {
                    pServer->base_port = atoi(optarg);
                } else {
                    printf("Error: Option -p requires a numeric argument\n");
                    usage();
                    return EXIT_FAILURE;
                }
                break;
            /* Nb de sockets à ouvrir par le serveur */
            case 'n':
                if (opt_isnum(optarg) == 1) {
                    pServer->nb_sockets = atoi(optarg);
                } else {
                    printf("Error: Option -n requires a numeric argument\n");
                    usage();
                    return EXIT_FAILURE;
                }
                break;

            /* Adresse IP du serveur distant */
            case 'a':
                strcpy((char *)pCli->remote_server_ip, optarg);
                break;
            /* Port du serveur distant */
            case 'd':
                if (opt_isnum(optarg) == 1) {
                    pCli->remote_server_base_port = atoi(optarg);
                } else {
                    printf("Error: Option -p requires a numeric argument\n");
                    usage();
                    return EXIT_FAILURE;
                }
                break;
            /* Envoi des messages de taille et de contenu aléatoire */
            case 'e':
                pCli->random_msg = 1;
                break;
            /* Wait for a reply from server */
            case 'w':
                pCli->waitreply = 1;
                break;
            /* Timeout sur réponse du serveur */
            case 't':
                if (opt_isnum(optarg) == 1) {
                    pCli->reply_timeout.tv_usec = atoi(optarg) * MILLISECONDES;
                } else {
                    printf("Error: Option -t requires a numeric argument\n");
                    usage();
                    return EXIT_FAILURE;
                }
                
                break;
            case 'l':
                if (opt_isnum(optarg) == 1) {
                    pConf->hb_timeout = atoi(optarg);
                } else {
                    printf("Error: Option -l requires a numeric argument\n");
                    usage();
                    return EXIT_FAILURE;
                }
                break;
            case 'v':
                if (opt_isnum(optarg) == 1) {
                    pConf->verbose_lvl = atoi(optarg);
                } else {
                    printf("Error: Option -v requires a numeric argument\n");
                    usage();
                    return EXIT_FAILURE;
                }
                break;
            /* Affichage de l'aide */
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
                break;

            case '?':
                if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
    
            default:
                usage();
                return EXIT_FAILURE;
        } /* Fin switch(c) */
    } /* Fin while getopt */
    //*pConf = pConfL;
    return EXIT_SUCCESS;
}
