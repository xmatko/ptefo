/*!
 * \file common.h
 * \brief Routines et fonctions diverses utilisée de façon commune
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 24/06/2014
 */
#ifndef _COMMON_H_
#define _COMMON_H_

#include <ctype.h>

#define M_MILLION 1000000
/*!
 * \fn int opt_isnum(char * str)
 * \brief Determine si une chaine de caractère est constituée uniquement de valeur numérique
 * \param[in] str: chaine de caractère à analyser
 * \return 1 si la chaine de caractère est constituée uniquement de valeur numériques, 0 sinon
 */
int opt_isnum(char * str);

/*!
 * \fn void die(char *s)
 * \brief Sortie du programme suite à une erreur.
 * \param[in] s: message à logguer ou à afficher lors de l'appel de cette fonction
 */
void die(char *s);

/*!
 * \fn int F_SetRandomMsg(char ** msg, int max_len)
 * \brief Génère une chaine de caractère aléatoire
 * \param[out] msg: pointeur sur une chaine de caractère devant contenir le résultat de la génération
 * \param[in] max_len: longueur maximum de la chaine de caractère devant être générée
 * \return 0 si OK, -1 si erreur
 * \warning Le pointeur passé en paramètre doit être non alloué et doit valoir NULL. 
 * Dans le cas contraire, la fonction tentera de désallouer le pointeur.\n
 * En conséquence, un pointeur non alloué et non initialisé à NULL provoquera une erreur de ségmentation\n
 * D'ou l'importance d'initialisé à NULL les pointeur dès leur déclaration !
 */
int F_SetRandomMsg(char ** msg, int max_len);

#endif /* _COMMON_H_ */
