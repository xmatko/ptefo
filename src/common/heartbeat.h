/*!
 * \file code5t_proc.h
 * \brief Traitements des codes 5 tons
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1é
 * \date 20/08/2014
 *
 * Gestion du traitement des codes 5 tons
 * 
 */

#ifndef _HB_MGT_H_
#define _HB_MGT_H_

/*!
 * \def DEFAULT_HB_TIMEOUT
 * \brief Timeout par défaut du signal de vie, en secondes
 */
#define DEFAULT_HB_TIMEOUT  60

/*!
 * \enum HB_Status heartbeat.h
 * \brief Etats possible du heartbeat
 *
 */
enum HB_Status {
    HB_INIT,
    HB_ALIVE,
    HB_WAIT_ACK,
    HB_WAIT_ACK_LAST_CHANCE,
    HB_DEAD,
    HB_UNKNOWN
};

/*!
 * \struct sHeartbeat heartbeat.h
 * \brief Structure de gestion du heartbeat
 *
 * La structure sHeartbeat est composée de 4 compteurs: 2 compteurs d'envoi et 2 compteurs de réception 
 * servant à la comparaison des trames envoyées et reçues
 */
typedef struct tHeartbeat
{
    unsigned char   counter;             /*!< Valeur courante du compteur de vie */
    unsigned char   counter_last;        /*!< Dernière valeur connue du compteur de vie */
    unsigned char   counter_rcv;         /*!< Valeur du compteur de vie reçue */
    unsigned char   counter_rcv_last;    /*!< Dernière valeur connue du compteur de vie reçue */
    int             status;              /*!< Status courant du compteur de vie */
    int             status_prev;         /*!< Précedent status du compteur de vie */
    int             timeout;
    pthread_mutex_t mutex_status;
    pthread_mutex_t mutex_counters;
    timer_t         hb_timer;
} sHeartbeat;

void * F_HB_Mgt(void * arg);


/*!
 * \fn int F_HB_Init(sHeartbeat ** pHB)
 * \brief Initialise une structure sHeartbeat
 * \param[in,out] pHB: adresse du pointeur sur la structure \a sHeartbeat à initialiser
 *
 * La fonction \a F_HB_Init alloue la mémoire nécessaire pour contenir la structure \a sHeartbeat et initialise les compteurs
 * aux valeur par défaut.
 */
int F_HB_Init(sHeartbeat ** pHB);

int F_HB_Reset(sHeartbeat * pHB);

/*!
 * \fn unsigned char F_HB_Inc(sHeartbeat * pHB)
 * \brief Incrémente le compteur de vie
 * \param[in] pHB: pointeur sur la structure \a sHeartbeat à incrémenter
 * \return valeur du compteur de vie
 *
 */
unsigned char F_HB_Inc(sHeartbeat * pHB);


/*!
 * \fn unsigned char F_HB_GetValue(sHeartbeat * pHB)
 * \brief Retourne la valeur courante du compteur de vie
 * \param[in] pHB: pointeur sur la structure \a sHeartbeat à incrémenter
 * \return valeur courante du compteur de vie
 *
 */
unsigned char F_HB_GetValue(sHeartbeat * pHB);

/*!
 * \fn unsigned char F_HB_GetValuePrev(sHeartbeat * pHB)
 * \brief Retourne la valeur précédentedu compteur de vie
 * \param[in] pHB: pointeur sur la structure \a sHeartbeat à incrémenter
 * \return valeur précédente du compteur de vie
 *
 */
unsigned char F_HB_GetValuePrev(sHeartbeat * pHB);

/*!
 * \fn int F_HB_UpdatePrev(sHeartbeat * pHB)
 * \brief Met à jour la valeur précédente à la valeur courante.
 * \param[in] pHB: pointeur sur la structure \a sHeartbeat à incrémenter
 * \return EXIT_SUCCESS;
 *
 */
int F_HB_UpdatePrev(sHeartbeat * pHB);

/*!
 * \fn unsigned char F_HB_GetValueRcv(sHeartbeat * pHB)
 * \brief Retourne la derniere valeur reçue en retour du compteur de vie
 * \param[in] pHB: pointeur sur la structure \a sHeartbeat à incrémenter
 * \return dernière valeur connue du compteur de vie reçu
 *
 */
unsigned char F_HB_GetValueRcv(sHeartbeat * pHB);

/*!
 * \fn int F_HB_ReceiveValue(sHeartbeat * pHB, unsigned char val)
 * \brief Met à jour la valeur du compteur de vie reçu
 * \param[in] pHB: pointeur sur la structure \a sHeartbeat à incrémenter
 * \param [in] val: valeur du compteur de vie reçu
 * \return EXIT_SUCCESS
 *
 */
int F_HB_ReceiveValue(sHeartbeat * pHB, unsigned char val);

/*!
 * \fn int F_HB_UpdateStatus(sHeartbeat * pHB)
 * \brief Determine et met à jour l'état du compteur de vie
 * \param[in] pHB: pointeur sur la structure \a sHeartbeat à incrémenter
 * \return Etat du compteur de vie parmi les valeurs de l'enum \a HB_Status
 *
 * L'état est determiné par comparaison des valeurs des compteurs d'envoi et de réception.
 * Tant que les valeurs sont différente de 1 unité, l'état est \a HB_WAIT_ACK
 * Le status est passé à \a HB_DEAD si les compteurs sont différents de 2 unités ou plus.
 *
 */
int F_HB_UpdateStatus(sHeartbeat * pHB);

int F_HB_GetStatus(sHeartbeat * pHB);
int F_HB_GetStatusPrev(sHeartbeat * pHB);


#endif /* _HB_MGT_H_ */
