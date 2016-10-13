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
 * Ce thread structe en permanence l'�tat de la file associ�e � la socket cliente recevant les donn�es d'appel.
 * D�s que la file contient au moins un �lement, la fonction d�pile l'�l�ment et tente de d�coder le code 5-tons
 * devant y �tre contenu.\n Ce dernier est envoy� sur la socket data au poste de t�te.
 */
void * F_Th_UDPClient_Data(void * arg);

/*!
 * \fn void * F_Th_UDPClient_Audio(void * arg)
 * \brief Thread client UDP audio
 * \param[in,out] arg: pointeur sur une structure sThArgCliAudio
 *
 * Ce thread structe en permanence l'�tat de la file associ�e � la socket recevant les donn�es audio.
 * D�s que la file contient au moins un �l�ment, la fonction d�pile les �lements et tente de d�coder les donn�es audio.
 * devant y �tre contenues.\n Ce dernier est ensuite envoy� sur la socket audio ad�quate au poste de t�te.
 */
void * F_Th_UDPClient_Audio(void * arg);


#endif /* _CLIENT_UDP_H_ */
