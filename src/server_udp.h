/*!
 * \file server_udp.h
 * \brief Gestion de la partie serveur UDP
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 30/06/2014
 */

#ifndef _SERVER_UDP_H_
#define _SERVER_UDP_H_

/*!
 * \fn void * F_Th_UDPServer(void * arg)
 * \brief Thread serveur UDP principal
 * \param[in,out] arg: pointeur sur une structure sMain
 *
 * Ce thread appelle périodiquement la fonction F_ServUDP_Listening.
 * C'est cette fonction qui va se mettre en écoute sur les sockets serveur et agir en fonction des données recues.
 */
void * F_Th_UDPServer(void * arg);

#endif /* _SERVER_UDP_H_ */
