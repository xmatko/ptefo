/*!
 * \file socks_mgt.h
 * \brief Fichier d'entete du module de gestion des sockets
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 06/2014
 *
 */

#ifndef _SOCKS_MGT_H_
#define _SOCKS_MGT_H_
#include <semaphore.h>

/*!
 * \def DEFAULT_REMOTE_SERVER_IP
 * \brief Adresse IP par défaut du serveur distant à contacter par le client 
 */
#define DEFAULT_REMOTE_SERVER_IP    "10.106.6.110"
//#define DEFAULT_REMOTE_SERVER_IP    "10.106.2.10"
/*!
 * \def ADDR_IP_LEN
 * \brief Taille maxi de la chaine contenant une adresse IP
 */
#define ADDR_IP_LEN                 16
/*!
 * \def HOSTNAME_MAX_LEN
 * \brief Taille maxi de la chaine contenant le hostname
 */
#define HOSTNAME_MAX_LEN            256
/*!
 * \def DEFAULT_REMOTE_SERVER_PORT
 * \brief Port de base par défaut du serveur distant à contacter par le client 
 */
#define DEFAULT_REMOTE_SERVER_PORT  12350
/*!
 * \def DEFAULT_SERVER_PORT
 * \brief Port de base d'écoute par défaut du serveur local 
 */
#define DEFAULT_SERVER_PORT         12350
/*!
 * \def DEFAULT_SERVER_AUTOREPLY
 * \brief OBSOLETE: Pour debug
 */
#define DEFAULT_SERVER_AUTOREPLY    0
/*!
 * \def DEFAULT_NB_SOCKS
 * \brief Nombre de sockets UDP par défaut à ouvrir par le serveur et par le client
 */
#define DEFAULT_NB_SOCKS            2
/*!
 * \def DEFAULT_LISTEN_TIMEOUT_MS
 * \brief Timeout par défaut appliqué lors de l'écoute sur les sockets serveur
 */
#define DEFAULT_LISTEN_TIMEOUT_MS   100
/*!
 * \def DEFAULT_REPLY_TIMEOUT_MS
 * \brief Timeout par défaut appliqué lors de l'attente d'une réponse, en millisecondes
 */
#define DEFAULT_REPLY_TIMEOUT_MS    100

/*!
 * \def BUF_SIZE
 * \brief Taille max des buffers d'envoi et de réception pour chaque socket
 */
#define BUF_SIZE                    512
/*!
 * \def INVALID_SOCKET
 * \brief Code retour d'erreur lors de l'ouverture de la socket
 */
#define INVALID_SOCKET              -1
/*!
 * \def SOCKET_ERROR
 * \brief Code retour d'erreur lors du bind de la socket
 */
#define SOCKET_ERROR                -1


/*!
 * \enum SOCK_ID socks_mgt.h
 * \brief Liste des identifiant des sockets
 *
 */
enum SOCK_ID {
    SOCK_DATA   = 0,
    SOCK_AUDIO1 = 1,
    SOCK_AUDIO2 = 2,
    SOCK_AUDIO3 = 3,
    SOCK_AUDIO4 = 4
};

/*!
 * \struct sSockFifo_Elem socks_mgt.h
 * \brief Structure de gestion d'une FIFO pour stocker les messages reçus ou à envoyer d'une socket donnée
 */
typedef struct tSockFifo_Elem
{
    char * data;                    /*!< Message stocké */
    unsigned int data_size;         /*!< Taille des données du message en octets. Si \a data_size est null, on prend la valeur maximale \a BUF_SIZE */
    int id;                         /*!< Identifiant du message stocké dans la file */
    struct tSockFifo_Elem * pNext;        /*!< Pointeur sur l'élément suivant de la file */
} sSockFifo_Elem;

typedef struct tSockFifo
{
    size_t slength;
    size_t max_size;
    char * name;
    struct tSockFifo_Elem * pHead;
    struct tSockFifo_Elem * pTail;
    pthread_mutex_t     mutex_fifo;
    sem_t   * sem;
} sSockFifo;


/*!
 * \struct sSockTab socks_mgt.h
 * \brief Structure regroupant toutes les informations liées à une socket
 */
typedef struct tSockTab 
{
    int                     fd;         /*!< File descriptor identifiant la socket */
    struct sockaddr_in *    sin;        /*!< Description de la socket */
    struct sockaddr_in *    sremote;    /*!< Description de la socket distante associée */
    char * rcv_buf;                     /*!< Buffer de réception des données dans la socket */
    char * snd_buf;                     /*!< Buffer d'envoi des données de la socket */
    int rcv_len;                        /*!< Nombre d'octets reçus par la socket */
    int snd_len;                        /*!< Nombre d'octets envoyé par la socket */
    int rcv_maxsize;                    /*!< Taille maximum pour le buffer de réception en octets */
    int snd_maxsize;                    /*!< Taille maximum pour le buffer d'envoi en octets */
    long int snd_trame_cpt;             /*!< Compteur de trames envoyées pour la socket */
    long int rcv_trame_cpt;             /*!< Compteur de trames reçues pour la socket */
    long int ref_time_s;
    short unsigned int port;            /*!< Port du serveur distant à contacter par la socket cliente */
    char * addr_ip;                     /*!< Addresse IP du serveur distant à contacter par la socket cliente */
    
    sSockFifo * rcv_queue;              /*!< File permettant de stocker les données reçues sur la socket */
    sSockFifo * snd_queue;              /*!< File permettant de stocker les données à envoyer sur la socket */
} sSockTab;



int F_SockQueue_Init(sSockFifo ** ppFifo, unsigned int max_size, const char * fifo_name);
int F_SockQueue_SetName(sSockFifo * pFifo, const char * fifo_name);
int F_SockQueue_Delete(sSockFifo ** ppFifo);
const char * F_SockQueue_GetName(sSockFifo * pFifo);

/*!
 * \fn void F_SockQueue_Add(sSockFifo_Elem ** pFifo, char * data)
 * \brief Ajout un élément à une file
 * \param[in,out] pFifo: adresse du pointeur sur la file à traiter
 * \param[in] data: donnée à ajouter à la file pFifo sous forme de chaine de caractère de taille max BUF_SIZE
 *
 * \warning Lors de l'ajout du premier élément dans la file, cette dernière doit être initialisée à NULL
 */
int F_SockQueue_Add(sSockFifo ** ppFifo, char * data, unsigned int data_size);


/*!
 * \fn int F_SockQueue_Remove(sSockFifo_Elem ** pFifo, char * get_data)
 * \brief Retire un élément à une file
 * \param[in,out] pFifo: adresse du pointeur sur la file à traiter
 * \param[out] get_id: adresse de la variable dans laquelle sera retournée l'identifiant de l'élement
 * \param[out] get_data: adresse de la variable dans laquelle sera retournée le message de l'élément retiré de la file sous forme de chaine de caractère
 * \param[out] get_size: adresse de la variable dans laquelle sera retournée la taille du message
 *
 * Une fois le dernier élément de la file retiré, cette dernière est réinitialisée à NULL
 */
//void F_SockQueue_Remove(sSockFifo ** pFifo, int * get_id, char * get_data, unsigned int * get_size);
int F_SockQueue_Remove(sSockFifo ** ppFifo, int * get_id, char * get_data, unsigned int * get_size);


/*!
 * \fn void F_SockQueue_Clear(sSockFifo_Elem ** pFifo)
 * \brief Vide complétement la file passée en paramètre
 * \param[in,out] pFifo: adresse du pointeur sur la file à vider
 *
 * En sortie de la fonction, la file pFifo est réinitialisée à NULL
 */
void F_SockQueue_Clear(sSockFifo ** pFifo);


/*!
 * \fn int F_SockQueue_GetNbElem(sSockFifo_Elem * pFifo)
 * \brief Récupere le nombre d'élément contenus dans la file passée en paramètre
 * \param[in] pFifo: pointeur sur la file à traiter
 * \return nombre d'élément dans la file
 */
int F_SockQueue_GetNbElem(sSockFifo * pFifo);



/*!
 * \fn void F_SockQueue_Print(sSockFifo_Elem * pFifo)
 * \brief Affiche les informations sur les éléments contenus dans la file passée en paramètre sur la sortie standart. Utile pour le debug.
 * \param[in] pFifo: pointeur sur la file à traiter
 */
void F_SockQueue_Print(sSockFifo * pFifo);



int F_Socket_InitStruct(sSockTab * pSockL);

/*!
 * \fn int F_Socket_Create(sSockTab ** pSocket, int port)
 * \brief Créer et initialise une socket UDP (serveur)
 * \param[in,out] pSocket: adresse du pointeur sur élément sSockTab, contenant toutes les infos liées à la socket
 * \param[in] port: numéro du port auquel la socket doit être attachée
 * \return 0 si OK, -1 si erreur
 */
int F_Socket_Create(sSockTab ** pSocket, int port);

/*!
 * \fn int F_Socket_CreateAll(sSockTab *** pSocks, short int baseport, char nb_socks)
 * \brief Créer et initialise toutes les nb_socks sockets UDP serveur pointées par le tableau de socket pSocks
 * \param[in,out] pSocks: adresse du tableau de pointeurs sur  les éléments sSockTab
 * \param[in] baseport: numéro du port de base à partir duquel les sockets doivent être bindées
 * \param[in] nb_socks: nombre de sockets à créer
 * \return 0 si OK, -1 si erreur
 */
int F_Socket_CreateAll(sSockTab *** pSocks, short int baseport, char nb_socks);

/*!
 * \fn int F_SocketCli_Create(sSockTab ** pSocket, char * dst_ip, int dst_port, int reply_to)
 * \brief Créer et initialise une socket UDP cliente
 * \param[in,out] pSocket: adresse du pointeur sur élément sSockTab, contenant toutes les infos liées à la socket
 * \param[in] dst_ip: adresse ip du serveur auquel la socket doit se connecter
 * \param[in] dst_port: numéro du port du serveur auquel la socket doit se connecter
 * \param[in] reply_to: valeur du timeout de réception de la socket cliente (en secondes)
 * \return 0 si OK, -1 si erreur
 */
int F_SocketCli_Create(sSockTab ** pSocket, char * dst_ip, int dst_port, struct timeval reply_to);

/*!
 * \fn int F_SocketCli_CreateAll(sSockTab *** pSocks, char * remote_ip, short int remote_baseport, int r_timeout, char nb_socks)
 * \brief Créer et initialise toutes les nb_socks sockets UDP clientes pointées par le tableau de socket pSocks
 * \param[in,out] pSocks: adresse du tableau de pointeurs sur  les éléments sSockTab
 * \param[in] remote_ip: adresse IP du serveur distant à contacter
 * \param[in] remote_baseport: numéro du port de base du serveur distant à contacter
 * \param[in] r_timeout: valeur du timeout de réception de la socket cliente (en secondes)
 * \param[in] nb_socks: nombre de sockets à créer
 * \return 0 si OK, -1 si erreur
 */
int F_SocketCli_CreateAll(sSockTab *** pSocks, char * remote_ip, short int remote_baseport, struct timeval r_timeout, char nb_socks);

/*!
 * \fn int F_SocketCli_Reset(sSockTab * pSocket)
 * \brief Réinitialise la socket cliente après un timeout ou une erreur en réception
 * \param[in,out] pSocket: adresse de l'élement \a sSockTab
 */
int F_SocketCli_Reset(sSockTab * pSocket);

/*!
 * \fn int F_Socket_Delete(sSockTab ** pSocket)
 * \brief Libération de la mémoire allouée à un élément sSockTab
 * \param[in,out] pSocket: adresse du pointeur sur un élément sSockTab
 * \return 0
 *
 * En sortie de la fonction, le pointeur pSocks n'est plus alloué et vaut NULL
 */
int F_Socket_Delete(sSockTab ** pSocket);

/*!
 * \fn int F_Socket_DeleteAll(sSockTab *** pSockets, int nb_socks)
 * \brief Libération de la mémoire de toutes les nb_socks sockets pointée par le tableau de sockets pSockets
 * \param[in,out] pSockets: adresse du tableau de pointeur sur les sockets de type sSockTab
 * \param[in] nb_socks: nombre de sockets à libérer.
 * \return 0
 *
 * En sortie de la fonction, le pointeur pSockets n'est plus alloué et vaut NULL
 */
int F_Socket_DeleteAll(sSockTab *** pSockets, int nb_socks);

/*!
 * \fn int F_Socket_Lire(sSockTab * pSocket)
 * \brief Routine de lecture des données reçues sur une socket
 * \param[in] pSocket: pointeur sur un objet sSockTab
 * \return 0 si OK, -1 si erreur
 * 
 * La fonction utilise l'appel à recvfrom() pour recevoir les données de la socket distante attachée. 
 */
int F_Socket_Lire(sSockTab * pSocket);
int F_Socket_Lire_Raw(sSockTab * pSocket, char * buf, int * len);

/*!
 * \fn int F_Socket_Ecrire(sSockTab * pSocket, const char * msg)
 * \brief Routine d'écriture des données à envoyer sur une socket
 * \param[in] pSocket: pointeur sur un objet sSockTab
 * \param[in] msg: données à envoyer sous forme de chaine de caractère
 * \param[in] len: taille des données à envoyer, en octets. Doit être strictement supérieur à zéro.
 * \return 0 si OK, -1 si erreur
 * 
 * La fonction utilise l'appel à sendto() pour envoyer les données à la socket distante. 
 */
int F_Socket_Ecrire(sSockTab * pSocket, const char * msg, unsigned int len);

/*!
 * \fn void F_Socket_ResetBuf(sSockTab * pSocket)
 * \brief Re-initialisation des buffers d'envoi et de réception de la socket définie par pSocket
 * \param[in,out] pSocket: pointeur sur un élément sSockTab
 *
 * En sortie de la fonction, les membres pSocket->snd_buf et pSocket->rcv_buf sont mis à zéro
 */
void F_Socket_ResetBuf(sSockTab * pSocket);

/*!
 * \fn void F_Socket_ResetSndBuf(sSockTab * pSocket)
 * \brief Re-initialisation du buffer d'envoi de la socket définie par pSocket
 * \param[in,out] pSocket: pointeur sur un élément sSockTab
 *
 * En sortie de la fonction, pSocket->snd_buf est mis à zéro
 */
void F_Socket_ResetSndBuf(sSockTab * pSocket);

/*!
 * \fn void F_Socket_ResetRcvBuf(sSockTab * pSocket)
 * \brief Re-initialisation du buffer de réception de la socket définie par pSocket
 * \param[in,out] pSocket: pointeur sur un élément sSockTab
 *
 * En sortie de la fonction, pSocket->rcv_buf est mis à zéro
 */
void F_Socket_ResetRcvBuf(sSockTab * pSocket);

/*!
 * \fn void F_Socket_PrintInfos(sSockTab * pSocket)
 * \brief Affiche les informations concernant la socket passée en paramètre
 * \param[in,out] pSocket: pointeur sur un élément sSockTab
 */
void F_Socket_PrintInfos(sSockTab * pSocket);

#endif /* _SOCKS_MGT_H_ */
