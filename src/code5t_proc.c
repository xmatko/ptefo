/*!
 * \file code5t_proc.c
 * \brief Traitements des codes 5 tons
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 20/08/2014
 *
 * Gestion du traitement des codes 5 tons
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "tlog.h"
#include "code5t_proc.h"


int F_Get_InhibStatus(sMain * pMain)
{
    //SLOGT(LOG_INFO, "F_Get_InhibStatus: pMain->inhib_traitements %d\n", pMain->inhib_traitements);
    return pMain->inhib_traitements;
}

int F_Set_InhibStatus(sMain * pMain, int status)
{
    if ( ((pMain->inhib_traitements == 0) && (status == 1)) ||
         ((pMain->inhib_traitements == 1) && (status == 0)) ) {
        //SLOGT(LOG_INFO, "F_Set_InhibStatus: %d, %d\n", pMain->inhib_traitements, status);
        //SLOGT(LOG_INFO, "F_Set_InhibStatus %d\n", status);
        pMain->inhib_traitements = status;
    }
    return EXIT_SUCCESS;
}

int F_Get_PDTStatus(sMain * pMain)
{
    return pMain->status_pdt;
}

int F_Set_PDTStatus(sMain * pMain, unsigned int status)
{
    if (pMain->status_pdt != status)
        pMain->status_pdt = status;
    return EXIT_SUCCESS;
}

int F_Get_PDTIdent(sMain * pMain)
{
    return pMain->ident_pdt;
}

int F_Set_PDTIdent(sMain * pMain, unsigned int ident)
{
    if (pMain->ident_pdt != ident) {
        pMain->ident_pdt = ident;
    }
    return EXIT_SUCCESS;
}

int F_Get_SystemStatus(sMain * pMain)
{
    SLOGT(LOG_INFO, "Inhibition des traitements: %d\n", pMain->inhib_traitements);
    SLOGT(LOG_INFO, "Identification du poste de tete: %u\n", pMain->ident_pdt);
    SLOGT(LOG_INFO, "Status du poste de tete: %u\n", pMain->status_pdt);
    return EXIT_SUCCESS;
}




//int F_Process_RcvData(sMain * pMain, sCode5T * pCodeRcv, sCode5T *** pppCodeAns, int * nb_ans)
int F_Process_RcvData(sMain * pMain, sCode5T * pCodeRcv, sCode5T ** ppCodeAns, int * nb_ans)
{
    unsigned char action5t = 0xFF;
    unsigned char syscode_val = 0xFF;
    char is_valid = -1;
    
    F_Code5T_GetValidity(pCodeRcv, &is_valid);
    if (is_valid != 1) {
        SLOGT(LOG_INFO, "MESSAGE INVALIDE! Pas de traitement (type %d, id %d, taille valeur %d)\n", pCodeRcv->type, pCodeRcv->id, pCodeRcv->value_size);
        return EXIT_FAILURE;
    }
    SLOGT(LOG_INFO, "Traitement d'un message\n");
    NLOG_INFO(F_Code5T_Infos(pCodeRcv));
    
    /* En fonction du code reçu, un ou plusieurs codes peuvent être envoyés en retour.
    Aucune allocation mémoire n'est réalisée ici. L'appellant passe en paramètre un tableau de pointeur
    déjà alloué à une taille max fixée à l'avance. Chaque pointeur pointé devra également être alloué. Cela peut-être réalisé via la fonction
    F_Code5T_InitAll().
    L'appellant aura également la charge de libérer l'ensemble des pointeurs pointés alloués ici via la fonction F_Code5T_FreeAll() une fois
    qu'il estime qu'il n'en n'a plus besoin.
    Entre temps, il peut appeller cette fonction plusieurs fois. On se chargera ici de réinitialiser les valeurs par défaut des pointeurs pointés
    Avantage: un espace mémoire alloué fixé à l'avance, qui peut utiliser plus de mémoire que nécessaire. L'allocation et la libération se font une seule
    fois par l'appellant.
    Inconvénient: L'allocation et la libération de la mémoire est à la charge de l'appellant.   
    */
    
    /* Code pour la deuxième méthode */
    if (nb_ans != NULL) {
        *nb_ans = 0;
        int nb_codes_ans = 2;
        int i = 0;
        unsigned char code_val_ans = 0;
        for (i = 0; i < nb_codes_ans; i++) {
            code_val_ans = i;
            F_Code5T_Reset(*(ppCodeAns + i));
            F_Code5T_Set_Num(*(ppCodeAns + i), GTW_TO_PDT, CTP_SYSTEM, CID_LAST_TERM, &code_val_ans, CODE_VALUE_LEN);
        }
        *nb_ans = nb_codes_ans;
    }
    /* Traitement en fonction du code :

     Cas possibles gérés par la partie serveur UDP (Communication initiée par le PDT)
        - Un PTO appelle
        - Un PTO raccroche
        - PDT envoie resultats test audio automatique
        - PDT envoie defaut d'alimentation
        - PDT envoie code de fonctionnement au démarrage
     Format du message reçu:    X YYYYY Z
     Si X >= 1 et X <= 4:
        Code 5-tons à destination du circuit X
        YYYYY = Identifiant du PTO
        Switch Z:
            case 1: PTO_ACT_IDENT       => PTO Appelle
            case 3: PTO_ACT_IDENT       => PTO raccroche
            case 4: PTO_ACT_IDENT       => Defaut d'alim PTO
            case 5: PTO_ACT_IDENT       => Retour d'appel
            case 6: PTO_ACT_IDENT       => PTO inexistant
            case 0: PTO_ACT_RING        => Indication sonnerie PTO

     Si X = 9
        Code systeme
        
        Switch YYYYY 
            case 99900 et Z = 8 ==> Fonctionnement au démarrage du poste de tête en maitre

     Si X = 8
        Retour test audio
        
    Un PTO raccroche
    PDT envoie resultats test audio automatique
    PDT envoie defaut d'alimentation
    PDT envoie code de fonctionnement au démarrage
     */
    switch(pCodeRcv->type) {
        case CTP_CIRCUIT_1:
        case CTP_CIRCUIT_2:
        case CTP_CIRCUIT_3:
        case CTP_CIRCUIT_4:
            action5t = pCodeRcv->code_val[0];
            //SLOGT(LOG_INFO, "Message de type circuit %d avec PTO identifiant %d\n", pCodeRcv->type, pCodeRcv->id);
            switch (action5t) {
                case CVL_C5T_0:
                    /* Indication de sonnerie d'un PTO */
                    /*  (GTW <- PDT) */
                    SLOGT(LOG_INFO, "\tPTO %d indique qu'il sonne sur circuit %d...\n", pCodeRcv->id, pCodeRcv->type);
                    break;
                case CVL_C5T_1:
                    /* Un PTO appelle: envoi du message à nc_core pour relai au pupitre via BIBL */
                    /*  (GTW <- PDT) */
                    SLOGT(LOG_INFO, "\tPTO %d appelle sur circuit %d...\n", pCodeRcv->id, pCodeRcv->type);
                    break;
                case CVL_C5T_2:
                    /* Un PTO est appellé (GTW -> PDT) */
                    SLOGT(LOG_INFO, "\tAppelle PTO %d sur circuit %d...\n", pCodeRcv->id, pCodeRcv->type);
                    break;
                case CVL_C5T_3:
                    /* Un PTO raccroche (GTW <- PDT) */
                    SLOGT(LOG_INFO, "\tPTO %d raccroche sur circuit %d...\n", pCodeRcv->id, pCodeRcv->type);
                    break;
                case CVL_C5T_4:
                    /* Un PTO déclare un défaut d'alimentation (GTW <- PDT) */
                    SLOGT(LOG_INFO, "\tPTO %d sur circuit %d déclare un défaut d'alimentation (%u) !\n", pCodeRcv->id, pCodeRcv->type, action5t);
                    break;
                case CVL_C5T_5:
                    /* Un PTO envoi un retour d'appel (GTW <- PDT) */
                    SLOGT(LOG_INFO, "\tPTO %d sur circuit %d envoie un retour d'appel (%u) !\n", pCodeRcv->id, pCodeRcv->type, action5t);
                    break;
                case CVL_C5T_6:
                    /* Un PTO n'est pas en ligne (GTW <- PDT) */
                    SLOGT(LOG_INFO, "\tPTO %d sur circuit %d n'est pas en ligne (%u)\n", pCodeRcv->id, pCodeRcv->type, action5t);
                    /* FIXME: Transfert du code 5-tons de raccrochage à la BIBL */
                    break;
                case CVL_C5T_B:
                    /* Un pupitre effectue une demande de test audio [GTW -> PDT] */
                    SLOGT(LOG_INFO, "\tPupitre demande test audio de PTO %d sur circuit %d (%u)\n", pCodeRcv->id, pCodeRcv->type, action5t);
                default:
                    SLOGT(LOG_ERR, "\tPTO %d sur circuit %d envoi un code 5-tons non reconnu (%u)\n", pCodeRcv->id, pCodeRcv->type, action5t);
            } /* Fin switch (action5t) */
            break;
        
        case CTP_AUDIO_TEST:
            /* [GTW <- PDT] */
            SLOGT(LOG_INFO, "PTO %d Message de type résultat de test audio (%d) du PTO\n", pCodeRcv->type, pCodeRcv->id);
            SLOGT(LOG_INFO, "\tCircuit %c: Fréquence 0x%02x 0x%02x, Niveau 0x%02x\n", pCodeRcv->code_val[0],
                                                                                      pCodeRcv->code_val[1],
                                                                                      pCodeRcv->code_val[2],
                                                                                      pCodeRcv->code_val[3]);
            break;
            
        case CTP_SYSTEM:
            //SLOGT(LOG_INFO, "Message de type system (%d)\n", pCodeRcv->type);
            syscode_val = pCodeRcv->code_val[0];
            switch(pCodeRcv->id) {
                case CID_FREE_CIRCUIT:
                    if (syscode_val == 0) {
                        SLOGT(LOG_INFO, "\tLibération de tous les circuit (%u)\n", syscode_val);
                    } else {
                        SLOGT(LOG_INFO, "\tLibération du circuit no %u\n", syscode_val);
                    }
                    break;
                case CID_HEARTBEAT:
                    SLOGT(LOG_INFO, "\tEnvoi de la trame de vie (0x%02X)\n", syscode_val);
                    break;
                case CID_HEARTBEAT_ACK:
                    SLOGT(LOG_INFO, "\tRéponse à la trame de vie (reçu 0x%02X)\n", syscode_val);
                    if (F_HB_GetStatus(pMain->pHB) != HB_DEAD) {
                        /* Mise à jour du compteur de vie reçu */
                        F_HB_ReceiveValue(pMain->pHB, syscode_val);
                        F_HB_UpdateStatus(pMain->pHB);
                    } else {
                        SLOGT(LOG_INFO, "\tRejetée car status heartbeat = HB_DEAD\n");
                    }
                    break;
                case CID_LAST_TERM:
                    if(syscode_val == CVL_C5T_8) {
                        SLOGT(LOG_INFO, "\tDernière borne\n");
                        /* Mise à jour de l'identification du poste de tête */
                        F_Set_PDTIdent(pMain, LASTTERM);
                        /* OBSOLETE: On force le maintient du heartbeat si on à une réponse identifiant la dernière borne 
                            Obsolete car désormais, le PDT envoie la trame "Derniere borne" mais répond également à la trame de vie
                        */
                        //F_HB_ReceiveValue(pMain->pHB, F_HB_GetValuePrev(pMain->pHB));
                    } else {
                        SLOGT(LOG_INFO, "\tDernière borne MESSAGE CORROMPU\n");
                    }

                    break;
                case CID_BIBL_STATUS:
                    SLOGT(LOG_INFO, "\tCarte BIBL HS ou manquante\n");
                    break;
                case CID_PDT_START:
                    SLOGT(LOG_INFO, "\tCode de fonctionnement au démarrage\n");
                    /* On réinitialise le flag d'inhibition des traitements */
                    F_Set_PDTIdent(pMain, MASTER);
                    F_Set_PDTStatus(pMain, PDT_STATUS_OK);
                    
                    /* On nettoie les messages contenus dans la file d'envoi */
                    if (F_SockQueue_GetNbElem(pMain->pSocketsCli[SOCK_DATA]->snd_queue) > 0)
                        F_SockQueue_Clear(&(pMain->pSocketsCli[SOCK_DATA]->snd_queue));
                    
                    F_HB_Reset(pMain->pHB);
                    SLOGT(LOG_INFO, "\tReset de la trame de vie\n");
                    if (F_Get_InhibStatus(pMain) == 1) {
                        SLOGT(LOG_INFO, "\tLevée de l'inhibition des traitements\n");
                        F_Set_InhibStatus(pMain, 0);
                    }
                    break;
                default:
                    SLOGT(LOG_ERR, "\tIdentifiant de message non reconnu...\n");
                    break;
            }
            break;
    }
    
    return EXIT_SUCCESS;
}


