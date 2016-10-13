/*!
 * \file config.h
 * \brief Gestion de la configuration du programme
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 25/06/2014
 *
 * Client / Serveur UDP de communication avec les postes de tête
 * 
 */

#ifndef _CONFIG_MAIN_H_
#define _CONFIG_MAIN_H_
#include <sys/time.h>
/*!
 * \struct sConfigServer
 * \brief Contient la configuration du serveur
 */
typedef struct tConfigServer
{
    short unsigned int   base_port;         /*!< Port de base du serveur */
    char     nb_sockets;                    /*!< Nombre de sockets à ouvrir par le serveur */
    unsigned char   * hostname;             /*!< Contient le nom d'hôte du serveur */
    struct timeval l_timeout;               /*!< Timeout appliqué à l'écoute sur les sockets serveurs */
} sConfigServer;

/*!
 * \struct sConfigClient
 * \brief Contient la configuration du client
 */
typedef struct tConfigClient
{
    unsigned char  * remote_server_ip;   /*!< Adresse IP du serveur distant à contacter */
    short unsigned int  remote_server_base_port;                /*!< Port du serveur distant à contacter */
    char    nb_sockets;                             /*!< Nombre de sockets cliente à ouvrir */
    char    random_msg;                             /*!< OBSOLETE: Flag de debug */
    char    waitreply;                              /*!< OBSOLETE: Flag de debug */
    struct timeval reply_timeout;                  /*!< Timeout appliqué à l'écoute sur les sockets serveurs */
} sConfigClient;


typedef struct tConfigClientTCP
{
    unsigned char * remote_server_ip;
    short unsigned int remote_server_port;
    struct timeval reply_timeout;
    struct timeval send_timeout;
    int retry_delay;
    int max_retry;
} sConfigClientTCP;


typedef struct tConfigServerTCP
{
    short unsigned int port;
    char nb_max_clients;
    unsigned char * hostname;
    struct timeval l_timeout;
} sConfigServerTCP;

/*!
 * \struct sConfigMain
 * \brief Contient la configuration globale du programme
 */
typedef struct tConfigMain
{
    sConfigServer * pServ;  /*!< Pointeur sur la configuration serveur */
    sConfigClient * pCli;   /*!< Pointeur sur la configuration cliente */
    sConfigServerTCP * pServTCP;
    sConfigClientTCP * pCliTCP;
    int             hb_timeout; /*!< Timeout au bout duquel le signal de vie est déclaré mort si pas de retour reçu, en secondes */
    int             verbose_lvl;
} sConfigMain;

/*!
 * \fn int F_Config_Init(sConfigMain ** pConf)
 * \brief Initialisation de la configuration globale
 * \param[in,out] pConf: adresse du pointeur sur la configuration globale
 * \return 0 si OK, -1 si erreur
 *
 * Cette fonction alloue et initialise la configuration globale du programme
 */
int F_Config_Init(sConfigMain ** pConf);

/*!
 * \fn int F_Config_Read_CmdLineArgs(sConfigMain * pConf, int argc, char * argv[])
 * \brief Lecture des arguments de la ligne de commande et mise à jour de la configuration globale
 * \param[in,out] pConf: pointeur sur la configuration globale
 * \param[in] argc: nombre d'arguments de la ligne de commande
 * \param[in] argv[]: pointeur sur le tableau contenant les arguments de la ligne de commande
 * \return 0 si OK, -1 si erreur
 */
int F_Config_Read_CmdLineArgs(sConfigMain * pConf, int argc, char * argv[]);

/*!
 * \fn void F_Config_Disp(sConfigMain * pConf)
 * \brief Affiche la configuration sur la sortie standard
 * \param[in] pConf: pointeur sur la configuration globale
 * \return 0 si OK, -1 si erreur
 */
void F_Config_Disp(sConfigMain * pConf);

#endif /* _CONFIG_MAIN_H_ */
