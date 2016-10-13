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

#ifndef _CODE_5T_PROC_H_
#define _CODE_5T_PROC_H_
#include "main.h"

/*!
 * \def NB_CODES_REPONSES_MAX
 * \brief Nombre maximum de codes devant être renvoyés en réponse à un code reçu donné.
 *
 * Ce nombre fixé à l'avance permet à l'appellant d'allouer un espace mémoire suffisant
 * pour le pointeur de pointeurs de codes réponse utilisé dans la fonction \a F_Process_RcvData()
 */
#define NB_CODES_REPONSES_MAX  10


//int F_Cli_CallPTO(int pto_id, int circuit);
/*!
 * \fn int F_Get_InhibStatus(sMain * pMain)
 * \brief Retourne l'état d'inhibition des traitements
 * \param[in] pMain: pointeur sur la structure \a sMain principale
 * \return Status de l'inhibition des traitements\n1: Traitements inhibés, 0: Traitements pris en compte
 *
 */
int F_Get_InhibStatus(sMain * pMain);
/*!
 * \fn int F_Set_InhibStatus(sMain * pMain, int status)
 * \brief Positionne l'état d'inhibition des traitements
 * \param[in] pMain: pointeur sur la structure \a sMain principale
 * \param[in] status: état d'inhibition à positionner
 * \return EXIT_SUCCESS
 *
 */
int F_Set_InhibStatus(sMain * pMain, int status);

int F_Set_PDTStatus(sMain * pMain, unsigned int status);
int F_Get_PDTStatus(sMain * pMain);
int F_Set_PDTIdent(sMain * pMain, unsigned int ident);
int F_Get_PDTIdent(sMain * pMain);
int F_Get_SystemStatus(sMain * pMain);

//int F_Process_RcvData(sMain * pMain, sCode5T * pCodeRcv, sCode5T *** pppCodeAns, int * nb_ans);
/*!
 * \fn int F_Process_RcvData(sMain * pMain, sCode5T * pCodeRcv, sCode5T ** ppCodeAns, int * nb_ans)
 * \brief Routine de traitement en fonction du code 5-tons reçu.
 * \param[in] pMain: pointeur sur la structure \a sMain principale
 * \param[in] pCodeRcv: pointeur sur l'élement \a sCode5T reçu
 * \param[out] ppCodeAns: Pointeur de pointeurs sur les élements \a sCode5T devant être renvoyé en réponse
 * \param[out] nb_ans: Adresse de la variable contenant le nombre d'élements \a sCode5T à devant être renvoyés en réponse
 * \return EXIT_SUCCESS
 *
 *     Cas possibles gérés par la partie serveur UDP (Communication initiée par le PDT)
 *      - Un PTO appelle
 *      - Un PTO raccroche
 *      - PDT envoie resultats test audio automatique
 *      - PDT envoie defaut d'alimentation
 *      - PDT envoie code de fonctionnement au démarrage
 *   Format du message reçu:    X YYYYY Z
 *   Si X >= 1 et X <= 4:
 *      Code 5-tons à destination du circuit X
 *      YYYYY = Identifiant du PTO
 *      Switch Z:
 *          case 1: PTO_ACT_IDENT       => PTO Appelle
 *          case 3: PTO_ACT_IDENT       => PTO raccroche
 *          case 4: PTO_ACT_IDENT       => Defaut d'alim PTO
 *          case 5: PTO_ACT_IDENT       => Retour d'appel
 *          case 6: PTO_ACT_IDENT       => PTO inexistant
 *          case 0: PTO_ACT_RING        => Indication sonnerie PTO
 *
 *   Si X = 9
 *      Code systeme
 *      
 *      Switch YYYYY 
 *          case 99900 et Z = 8 ==> Fonctionnement au démarrage du poste de tête en maitre
 *
 *   Si X = 8
 *      Retour test audio
 *      
 *  Un PTO raccroche
 *  PDT envoie resultats test audio automatique
 *  PDT envoie defaut d'alimentation
 *  PDT envoie code de fonctionnement au démarrage
 *
 *
 *
 * En fonction du code reçu, un ou plusieurs codes peuvent être envoyés en retour.\n
 *    Deux méthodes sont étudiés ici:\n
 *    Dans la première, l'appellant passe en paramètre l'adresse d'un tableau de pointeurs sur des élements sCode5T (triple pointeur)\n
 *    ainsi que l'adresse d'une variable qui indiquera le nombre de code à renvoyer\n
 *    Ce pointeur de pointeur ainsi que les pointeurs pointés NE DOIVENT PAS être alloué. \n
 *    En effet, le cas échéant, c'est ici que l'on va se charger d'allouer la mémoire nécessaire aux codes reponses en fonction que leur nombre.\n
 *    Dans ce cas, l'appellant doit s'assurer que la mémoire précédement allouée aux pointeurs passés en paramètre est libérée.\n
 *    Avantage: un espace mémoire alloué aux nombre juste d'octets nécessaire.\n
 *    Inconvénient: l'appellant effectuer lui-même la libération de la mémoire précédement allouée avant de rappeller cette fonction.\n
 *    \n
 *    Dans la deuxième méthode, aucune allocation mémoire n'est réalisée ici. L'appellant passe en paramètre un tableau de pointeur\n
 *    déjà alloué à une taille max fixée à l'avance. Chaque pointeur pointé devra également être alloué. Cela peut-être réalisé via la fonction\n
 *    F_Code5T_InitAll().\n
 *    L'appellant aura également la charge de libérer l'ensemble des pointeurs pointés alloués ici via la fonction F_Code5T_FreeAll() une fois\n
 *    qu'il estime qu'il n'en n'a plus besoin.\n
 *    Entre temps, il peut appeller cette fonction plusieurs fois. On se chargera ici de réinitialiser les valeurs par défaut des pointeurs pointés\n
 *    Avantage: un espace mémoire alloué fixé à l'avance, qui peut utiliser plus de mémoire que nécessaire. L'allocation et la libération se font une seule
 *    fois par l'appellant.\n
 *    Inconvénient: L'allocation et la libération de la mémoire est à la charge de l'appellant.\n
 *    \n
 *    ==> Méthode choisie: 2\n
 *
 */
int F_Process_RcvData(sMain * pMain, sCode5T * pCodeRcv, sCode5T ** ppCodeAns, int * nb_ans);

#endif /* _CODE_5T_PROC_H_ */
