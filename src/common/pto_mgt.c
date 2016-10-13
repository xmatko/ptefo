/*!
 * \file pto_mgt.c
 * \brief Gestion des PTO connus du système
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 25/08/2014
 *
 * Gestion de la liste des PTO
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "tlog.h"
#include "pto_mgt.h"

static char * str_st = NULL;

const char * str_ptostatus(int status)
{
    if (str_st == NULL) {
        str_st = (char *)malloc(16 * sizeof (char));
    }

    memset(str_st, 0, 16 * sizeof (char));
    switch (status) {
        case PTO_STATUS_INIT:
            strcpy(str_st, "INIT");
            break;
        case PTO_STATUS_CALLED:
            strcpy(str_st, "CALLED");
            break;
        case PTO_STATUS_CALLING:
            strcpy(str_st, "CALLING");
            break;
        case PTO_STATUS_RINGING:
            strcpy(str_st, "RINGING");
            break;
        case PTO_STATUS_ONLINE:
            strcpy(str_st, "ONLINE");
            break;
        case PTO_STATUS_ENDCALL:
            strcpy(str_st, "ENDCALL");
            break;
        case PTO_STATUS_OFFLINE:
            strcpy(str_st, "OFFLINE");
            break;
        case PTO_STATUS_AUDIO_ON:
            strcpy(str_st, "AUDIO_ON");
            break;
        case PTO_STATUS_AUDIO_OFF:
            strcpy(str_st, "AUDIO_OFF");
            break;
        case PTO_STATUS_POWER_DEF:
            strcpy(str_st, "POWER_DEF");
            break;
        case PTO_STATUS_HS:
            strcpy(str_st, "HS");
            break;
        case PTO_STATUS_CALLBACK:
            strcpy(str_st, "CALLBACK");
            break;
        case PTO_STATUS_UNKNOWN:
        default:
            strcpy(str_st, "UNKNOWN");
    }
    return str_st;
}

int F_PTO_List_Init(sPTO_List ** ppList)
{
    sPTO_List * pNew = NULL;
    pNew = (sPTO_List *)malloc(sizeof (sPTO_List));
    if (pNew != NULL)
    {
        pNew->length = 0;
        pNew->pHead = NULL;
        pNew->pTail = NULL;
    }
    *ppList = pNew;
    return EXIT_SUCCESS;
}

int F_PTO_List_Append(sPTO_List ** ppList, int id, int circuit, ePto_Status status, ePto_CommInit commtype)
{
    /*          <---\/--->   <---\/--->
            data    /\   data    /\   data
            pNext -/  \  pNext -/  \  pNext --> NULL
   NULL <-- pPrev      - pPrev      - pPrev
    */
    
    if (*ppList != NULL) {     /* On vérifie si notre liste a été allouée */
        struct tPTO * pNew = malloc(sizeof *pNew); /* Création d'un nouveau node */
        if (pNew != NULL) {    /* On vérifie si le malloc n'a pas échoué */
            pNew->id = id;     /* On 'enregistre' notre donnée */
            pNew->circuit = circuit;
            pNew->status = status;
            pNew->comm_type = commtype;
            
            pNew->pNext = NULL;   /* On fait pointer pNext vers NULL */
            if ((*ppList)->pTail == NULL) {
                /* Cas où notre liste est vide (pointeur vers fin de liste à  NULL) */
                pNew->pPrev = NULL; /* On fait pointer pPrev vers NULL */
                (*ppList)->pHead = pNew; /* On fait pointer la tête de liste vers le nouvel élément */
                (*ppList)->pTail = pNew; /* On fait pointer la fin de liste vers le nouvel élément */
            } else {
                /* Cas où des éléments sont déjà présents dans la liste */
                (*ppList)->pTail->pNext = pNew; /* On relie le dernier élément de la liste vers notre nouvel élément (début du chaînage) */
                pNew->pPrev = (*ppList)->pTail; /* On fait pointer pPrev vers le dernier élément de la liste */
                (*ppList)->pTail = pNew; /* On fait pointer la fin de liste vers notre nouvel élément (fin du chaînage: 3 étapes) */
            }
            (*ppList)->length++; /* Incrémentation de la taille de la liste */
        }
    }
    return ((*ppList)->length - 1); 
}

int F_PTO_List_Prepend(sPTO_List ** ppList, int id, int circuit, ePto_Status status, ePto_CommInit commtype)
{
    if (*ppList != NULL) {
        struct tPTO * pNew = malloc(sizeof *pNew); 
        if (pNew != NULL) {
            pNew->id = id;
            pNew->circuit = circuit;
            pNew->status = status;
            pNew->comm_type = commtype;
            
            pNew->pPrev = NULL; 
            if ((*ppList)->pTail == NULL) {
                /* Cas où notre liste est vide (pointeur vers fin de liste à  NULL) */
                pNew->pNext = NULL;
                (*ppList)->pHead = pNew;
                (*ppList)->pTail = pNew;
            } else {
                /* Cas où des éléments sont déjà présents dans la liste */
                (*ppList)->pHead->pPrev = pNew;
                pNew->pNext = (*ppList)->pHead;
                (*ppList)->pHead = pNew;
            }
            (*ppList)->length++; /* Incrémentation de la taille de la liste */
        }
    }
    return EXIT_SUCCESS;
}

int F_PTO_List_Insert(sPTO_List ** ppList, int position, int id, int circuit, ePto_Status status, ePto_CommInit commtype)
{
    if (*ppList != NULL)
    {
        /* On utilise un pointeur temporaire pour parcourir la liste. On le fait pointer vers le premier élément de notre list */
        struct tPTO * pTmp = (*ppList)->pHead;
        int i = 0;
        
        /* On parcours la liste à la recherche de la position d'insertion souhaitée */
        while (pTmp != NULL && i <= position) {
            
            /* On a trouvé la position d'insertion */
            if (position == i) {
                if (pTmp->pNext == NULL) {
                    /* Si il n'y a pas d'élément après la position d'insertion, on ajoute le nouvel élement en fin de liste */
                    F_PTO_List_Append(ppList, id, circuit, status, commtype);
                }
                else if (pTmp->pPrev == NULL) {
                    /* Sinon, si il n'y a pas d'élément avant la position d'insertion, on ajoute le nouvel élément en début de list */
                    F_PTO_List_Prepend(ppList, id, circuit, status, commtype);
                }
                else {
                    /* Sinon, on crée le nouvel élement et on l'insert à la position souhaitée */
                    struct tPTO * pNew = malloc(sizeof *pNew);;
                    if (pNew != NULL) {
                        pNew->id = id;
                        pNew->circuit = circuit;
                        pNew->status = status;
                        pNew->comm_type = commtype;
                        //pTmp->pNext->pPrev = pTmp;    /* Pour l'élement suivant, l'élement précédent l'élément pointé lors de la recherche */
                        pTmp->pPrev->pNext = pNew;      /* Pour l'élement précédent, l'élément suivant désormais sera notre nouvel élément */
                        pNew->pPrev = pTmp->pPrev;      /* Pour notre nouvel élément, l'élement précédent sera celui qui précédait l'élement pointé lors de la recherche de position  */
                        
                        pTmp->pPrev = pNew;            
                        pNew->pNext = pTmp;             /* Pour notre nouvel élément, l'élément suivant sera celui pointé lors de la recherche de position */
                        (*ppList)->length++;            /* Incrémentation de la taille de la liste */
                    }
                }
            } else {
                /* L'élément de liste pointé n'est toujours pas à la position d'insertion souhaitée, 
                   on passe à l'élément suivant */
                pTmp = pTmp->pNext;
            }
            i++;
        }
    }
    return position;
    
}

int F_PTO_List_Update(sPTO_List ** ppList, int position, int id, int circuit, ePto_Status status, ePto_CommInit commtype)
{
    if (*ppList != NULL)
    {
        /* On utilise un pointeur temporaire pour parcourir la liste. On le fait pointer vers le premier élément de notre liste */
        struct tPTO * pTmp = (*ppList)->pHead;
        int i = 0;
        
        /* On parcours la liste à la recherche de la position de l'élément à mettre à jour */
        while (pTmp != NULL && i <= position) {
             /* On a trouvé la position de mise à jour */
            if (position == i) {
                pTmp->id = id;
                pTmp->circuit = circuit;
                pTmp->status = status;
                pTmp->comm_type = commtype;
            } else {
                /* L'élément de liste pointé n'est toujours pas à la position de mise à jour souhaitée, 
                   on passe à l'élément suivant */
                pTmp = pTmp->pNext;
            }
            i++;
        }
    }
    return EXIT_SUCCESS;
    
}

int F_PTO_List_Remove(sPTO_List ** ppList, int position)
{
    if (*ppList != NULL) {
        struct tPTO * pTmp = (*ppList)->pHead;
        int i = 0;
        //printf("F_PTO_List_Remove: demande de suppression du pto à la position %d\n", position);
        while (pTmp != NULL && i <= position) {
            if (position == i) {
                /* Si on veut supprimer le dernier élément de la liste */
                if (pTmp->pNext == NULL) {
                    //printf("supp last\n");
                    if(pTmp->pPrev == NULL) {    /* Si le dernier est aussi le premier (1 seul élément) */
                        (*ppList)->pTail = NULL;
                        (*ppList)->pHead = NULL;
                    } else {
                        (*ppList)->pTail = pTmp->pPrev;
                        (*ppList)->pTail->pNext = NULL;
                    }
                }
                /* Si on veut supprimer le premier élément de la liste */
                else if (pTmp->pPrev == NULL) {
                    //printf("supp first\n");
                    (*ppList)->pHead = pTmp->pNext;
                    if ((*ppList)->pHead != NULL)
                        (*ppList)->pHead->pPrev = NULL;
                }
                else {
                    //printf("supp %d\n", i);
                    //printf("Element (%d): id = %d, circuit = %d, status = %d, commtype = %d\n", i-2, pTmp->pPrev->id, pTmp->pPrev->circuit, pTmp->pPrev->status, pTmp->pPrev->comm_type);
                    //printf("Element (%d): id = %d, circuit = %d, status = %d, commtype = %d\n", i-1, pTmp->id, pTmp->circuit, pTmp->status, pTmp->comm_type);
                    //printf("Element (%d): id = %d, circuit = %d, status = %d, commtype = %d\n", i, pTmp->pNext->id, pTmp->pNext->circuit, pTmp->pNext->status, pTmp->pNext->comm_type);
                    pTmp->pNext->pPrev = pTmp->pPrev;
                    pTmp->pPrev->pNext = pTmp->pNext;
                }
                free(pTmp);
                (*ppList)->length--;
            } else {
                pTmp = pTmp->pNext;
            }
            i++;
        }
        if (pTmp == NULL && i <= position) {
            printf("F_PTO_List_Remove: Position %d non existante (%d éléments dans la liste)\n", position, (int)(*ppList)->length);
        }
    }

    return EXIT_SUCCESS;
}

int F_PTO_List_FindPtoId(sPTO_List * pList, int pto_id)
{
    int found_pos = -1;
    int i = 0;
    if (pList != NULL) {
        //printf("PTO Id to find = %d\n", pto_id);
        struct tPTO * pTmp = pList->pHead;
        while ((pTmp != NULL) && (found_pos == -1)) {
            if (pTmp->id == pto_id) {
                found_pos = i;
                //printf("found_pos = %d\n", i);
            }
            pTmp = pTmp->pNext;
            i++;
        }
    }
    //printf("found_pos = %d\n", found_pos);
    return found_pos;
}

int F_PTO_List_FindCircuit(sPTO_List * pList, int circuit)
{
    int found_pos = -1;
    int i = 0;
    if (pList != NULL) {
        //printf("PTO circuit to find = %d\n", circuit);
        struct tPTO * pTmp = pList->pHead;
        while ((pTmp != NULL) && (found_pos == -1)) {
            if (pTmp->circuit == circuit) {
                found_pos = i;
                //printf("found_pos = %d\n", i);
            }
            pTmp = pTmp->pNext;
            i++;
        }
    }
    //printf("found_pos = %d\n", found_pos);
    return found_pos;
}

int F_PTO_List_FindOnlinePto_ByCircuit(sPTO_List * pList, int circuit)
{
    int nb_online_pto = 0;
    int i = 0;
    if (pList != NULL) {
        //printf("PTO circuit to find = %d\n", circuit);
        struct tPTO * pTmp = pList->pHead;
        while (pTmp != NULL)  {
            if (pTmp->circuit == circuit) {
                nb_online_pto++;
                //printf("found_pos = %d\n", i);
            }
            pTmp = pTmp->pNext;
            i++;
        }
    }
    //printf("found_pos = %d\n", found_pos);
    return nb_online_pto;
}

/* Cette fonction retourne UNE COPIE de l'élément recherché. Si des modifications sont effectuées par l'appelant sur l'élément retourné
ces modifications ne seront pas repercuté dans l'élément de la liste.
Pour se faire, il faut utiliser après coup la fonction F_PTO_List_Update() */
sPTO * F_PTO_List_GetPto(sPTO_List * pList, int position)
{   
    int i = 0;
    struct tPTO * pReturn = malloc(sizeof *pReturn);
    memset(pReturn, 0, sizeof (*pReturn));
    
    if (pList != NULL) {
        struct tPTO * pTmp = pList->pHead;
        while ((pTmp != NULL) && (i <= position)) {
            if (i == position) {
                /* On effectue une copie du pointeur temporaire dans le pointeur de retour */
                memcpy(pReturn, pTmp, sizeof(struct tPTO));
                
            } else {
                pTmp = pTmp->pNext;
            }
            i++;
            
        }
    }
    
    /* On reset les pointeurs pNext et pPrev de notre élément retour, celui-ci n'étant pas inclu dans une liste */
    pReturn->pNext = NULL;
    pReturn->pPrev = NULL;
    return pReturn;
}

int F_PTO_List_Delete(sPTO_List ** ppList)
{
    if (*ppList != NULL) {
        struct tPTO * pTmp = (*ppList)->pHead;
        while (pTmp != NULL) {
            struct tPTO * pDel = pTmp;
            pTmp = pTmp->pNext;
            free(pDel);
        }
        free(*ppList);
        *ppList = NULL;
    }
    return EXIT_SUCCESS;
}

int F_PTO_List_GetNbElements(sPTO_List * pList)
{
    unsigned int i = 0;
    if (pList != NULL) {
        struct tPTO * pTmp = pList->pHead;
        while (pTmp != NULL) {
             pTmp = pTmp->pNext;
             i++;
        }
    }
    if (i != pList->length) {
        printf("F_PTO_List_GetNbElements: found %u / %u\n", i,  (unsigned int)pList->length);
    }
    return (int)i;
}

int F_PTO_List_DisplayContent(sPTO_List * pList)
{
    int i = 0;
    int nb_elem = 0;
    if (pList != NULL) {
        nb_elem = F_PTO_List_GetNbElements(pList);
        printf("Nombre d'éléments dans la liste = %d\n", nb_elem);
        
        struct tPTO * pTmp = pList->pHead;
        while (pTmp != NULL) {
            printf("Element (%d): id = %d, circuit = %d, status = %d, commtype = %d\n", i, pTmp->id, pTmp->circuit, pTmp->status, pTmp->comm_type);
            fflush(stdout);
            i++;
            pTmp = pTmp->pNext;
            
        }
    }
    return EXIT_SUCCESS;
}

int F_PTO_Test_Usage(void)
{
    int elem = -1;
    sPTO_List * ptos = NULL;
    sPTO * pto = NULL;
    F_PTO_List_Init(&ptos);
    
    F_PTO_List_Append(&ptos, 50101, 1, PTO_STATUS_UNKNOWN, PTO_UDP_SERVEUR);
    F_PTO_List_Append(&ptos, 50231, 4, PTO_STATUS_UNKNOWN, PTO_UDP_SERVEUR);
    F_PTO_List_Append(&ptos, 43751, 2, PTO_STATUS_UNKNOWN, PTO_UDP_CLIENT);
    F_PTO_List_Append(&ptos, 99999, 2, PTO_STATUS_UNKNOWN, PTO_UDP_CLIENT);
    F_PTO_List_Prepend(&ptos, 33218, 3, PTO_STATUS_UNKNOWN, PTO_UDP_SERVEUR);
    F_PTO_List_Insert(&ptos, 3, 12345, 4, PTO_STATUS_UNKNOWN, PTO_UDP_CLIENT);
    F_PTO_List_Insert(&ptos, 2, 99887, 1, PTO_STATUS_UNKNOWN, PTO_UDP_SERVEUR);
    F_PTO_List_DisplayContent(ptos);
    printf("\n");
    F_PTO_List_Remove(&ptos, 2);
    F_PTO_List_Remove(&ptos, 3);
    F_PTO_List_DisplayContent(ptos);
    printf("\n");
    
    /* Suppression d'un élément de la liste inexistant */
    F_PTO_List_Remove(&ptos, 19);
    printf("\n");

    /* Récupération d'un PTO */
    pto = F_PTO_List_GetPto(ptos, 2);
    printf("PTO idx 2: id = %d, circuit = %d, status = %d, commtype = %d\n", pto->id, pto->circuit, pto->status, pto->comm_type);
    printf("\n");
    pto->id = 88888;
    pto->status = PTO_STATUS_OFFLINE;

    /* Mise à jour d'un PTO de la liste */
    F_PTO_List_Update(&ptos, 2, pto->id, pto->circuit, pto->status, pto->comm_type);
    
    
    /* Récupération du PTO modifié */
    free(pto);  // Le pointeur est alloué dans la fonction GetPto. Si on réutilise le même pointeur pour un nouvel appel, il faut le libérer avant...
    pto = F_PTO_List_GetPto(ptos, 2);
    printf("PTO idx 2: id = %d, circuit = %d, status = %d, commtype = %d\n", pto->id, pto->circuit, pto->status, pto->comm_type);
    printf("\n");
    
    /* Recherche un PTO dans la liste par son identifiant */
    elem = F_PTO_List_FindPtoId(ptos, 504231);
    printf("Found id at position %d\n", elem);
    printf("\n");
    
    
    F_PTO_List_Delete(&ptos);
    return EXIT_SUCCESS;
    
}

