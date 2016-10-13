/*!
 * \file client_udp.h
 * \brief Gestion de la partie client UDP
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 01/07/2014
 */

#ifndef _CLIENT_UDP_H_
#define _CLIENT_UDP_H_


/*!
 * \struct sThArgCliAudio client_udp.h
 * \brief Structure d'encapsulation pour passage d'argument aux threads F_Th_UDPClient_Audio
 */
typedef struct tThArgCliAudio
{
    void  * pMain;  /*!< Pointeur sur la structure principale */
    int     th_num; /*!< Numero du thread */
} sThArgCliAudio;


/*!
 * \fn void * F_Th_UDPClient_Data(void * arg);
 * \brief Thread client UDP data
 * \param[in,out] arg: pointeur sur une structure sMain
 *
 * Ce thread structe en permanence l'état de la file associée à la socket cliente recevant les données d'appel.
 * Dès que la file contient au moins un élement, la fonction dépile l'élément et tente de décoder le code 5-tons
 * devant y être contenu.\n Ce dernier est envoyé sur la socket data au poste de tête.
 */
void * F_Th_UDPClient_Data(void * arg);

/*!
 * \fn void * F_Th_UDPClient_Audio(void * arg)
 * \brief Thread client UDP audio
 * \param[in,out] arg: pointeur sur une structure sThArgCliAudio
 *
 * Ce thread structe en permanence l'état de la file associée à la socket recevant les données audio.
 * Dès que la file contient au moins un élément, la fonction dépile les élements et tente de décoder les données audio.
 * devant y être contenues.\n Ce dernier est ensuite envoyé sur la socket audio adéquate au poste de tête.
 */
void * F_Th_UDPClient_Audio(void * arg);


#endif /* _CLIENT_UDP_H_ */
