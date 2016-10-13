/*!
 * \file code5t.c
 * \brief Gestion des codes 5 tons
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 20/06/2014
 *
 * Gestion des codes 5 tons
 * 
 * La gestion des codes 5-tons est basée sur la structure sCode5t
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "tlog.h"
#include "code5t.h"


int F_Code5T_Init(sCode5T ** ppCode)
{
    *ppCode = (sCode5T *)malloc(sizeof (sCode5T));
    memset(*ppCode, 0, sizeof (sCode5T));
    (*ppCode)->type = -1;
    (*ppCode)->id = -1;
    (*ppCode)->action = -1;
    (*ppCode)->value_size = -1;
    (*ppCode)->code_val[0] = -1;
    (*ppCode)->cstr_len = -1;
    (*ppCode)->cstr = (unsigned char *)malloc(CODE_MAX_LEN * sizeof (unsigned char));
    memset((*ppCode)->cstr, 0, CODE_MAX_LEN * sizeof (unsigned char));
    
    (*ppCode)->code_bibl = (unsigned char *)malloc((CODE_BIBL_LEN + 1) * sizeof (unsigned char));
    memset((*ppCode)->code_bibl, 0, (CODE_BIBL_LEN + 1) * sizeof (unsigned char));
    (*ppCode)->code_bibl_raw = (unsigned char *)malloc((CODE_BIBL_LEN + 1) * sizeof (unsigned char));
    memset((*ppCode)->code_bibl_raw, 0, (CODE_BIBL_LEN + 1) * sizeof (unsigned char));
    
    (*ppCode)->circuit_bibl = -1;
    
    (*ppCode)->pto_status = -1;
    (*ppCode)->sens = -1;
    /* printf("INIT *ppCode %x\n", (unsigned long)*ppCode); */
    return EXIT_SUCCESS;
}

int F_Code5T_InitAll(sCode5T *** pppCodes, int nb_elements)
{
    int i = -1;
    if (nb_elements <= 0)
        return EXIT_FAILURE;
    *pppCodes = (sCode5T **)malloc(nb_elements * sizeof (sCode5T *));
    for (i = 0; i < nb_elements; i++) {
        F_Code5T_Init(*pppCodes + i);
        /* printf("INIT APRES: %x\n", (unsigned long)*(*pppCodes + i)); */
    }
    return EXIT_SUCCESS;
}

int F_Code5T_Free(sCode5T ** ppCode)
{
    //printf("FREE *ppCode %x\n", (unsigned long)*ppCode);
    if (*ppCode != NULL) {
        free((*ppCode)->cstr);
        free((*ppCode)->code_bibl);
        free(*ppCode);
    }
        
    
    *ppCode = NULL;
    //printf("FREE *ppCode %x\n", (unsigned long)*ppCode);
    return EXIT_SUCCESS;
}

int F_Code5T_FreeAll(sCode5T *** pppCodes, int nb_elements)
{
    int i = -1;
    
    /* Equivalences avec: sCode5T ** pTmp = *pCodes;
            pTmp == *pCodes;
           *pTmp == **pCodes;
           *(pTmp + 1) == pTmp[1] == *(*pCodes + 1)
           (pTmp + 1) == &(pTmp[1]) == (*pCodes + 1)
    */
    /* Debug pointeurs
    printf("FREEALL pppCodes %x\n", (unsigned long)*(pppCodes));
    printf("FREEALL pTmp %x\n", (unsigned long)(pTmp));
    printf("FREEALL pTmp[0] %x\n", (unsigned long)*(pTmp));
    printf("FREEALL pTmp[0] %x\n", (unsigned long)**pppCodes);
    printf("FREEALL pTmp[1] %x\n", (unsigned long)*(pTmp + 1));
    printf("FREEALL pTmp[1] %x\n", (unsigned long)pTmp[1]);
    printf("FREEALL pTmp[1] %x\n", (unsigned long)(*(*pppCodes + 1)));
    printf("FREEALL pTmp[2] %x\n", (unsigned long)*(pTmp + 2));
    printf("FREEALL pTmp[1] %x\n", (unsigned long)pTmp[2]);
    printf("FREEALL pTmp[2] %x\n", (unsigned long)(*(*pppCodes + 2)));
    */

    for (i = 0; i < nb_elements; i++) {
        //F_Code5T_Free(&(pTmp[i]));  //(pTmp + i)  <=> &(pTmp[i])
        //F_Code5T_Free(pTmp + i);
        F_Code5T_Free(*pppCodes + i);
    }

    if (*pppCodes != NULL) {
        //printf("*pppCodes %x\n", (unsigned long)*pppCodes);
        free(*pppCodes);
        *pppCodes = NULL;
        //printf("*pppCodes %x\n", (unsigned long)*pppCodes);
    } else {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int F_Code5T_Reset(sCode5T * pCode)
{
    pCode->type = -1;
    pCode->id = -1;
    pCode->action = -1;
    pCode->value_size = -1;
    memset(pCode->code_val, 0, CODE_VALUE_TEST_AUDIO_LEN * sizeof (unsigned char));
    pCode->cstr_len = -1;
    memset(pCode->cstr, 0, CODE_MAX_LEN * sizeof (unsigned char));
    memset(pCode->code_bibl, 0, (CODE_BIBL_LEN + 1) * sizeof (unsigned char));
    pCode->circuit_bibl = -1;
    pCode->sens = -1;
    return EXIT_SUCCESS;
}

int F_Code5T_GetValidity(sCode5T * pCode, char * validity)
{
    /* Premier check */
    /*
    if ((pCode->type == -1) || (pCode->id == -1) || (pCode->value_size == -1)) {
        //printf("Message 5Tons invalide! type %d, id %d, taille valeur %d\n", pCode->type, pCode->id, pCode->value_size);
        *validity = 0;
    } else {
        *validity = 1;
    }
    */
    
    *validity = 0;
    /* Deuxieme check */
    if ( /* Check sur le type: */
         ( ((pCode->type >= CTP_CIRCUIT_1) && (pCode->type <= CTP_CIRCUIT_4)) ||
            (pCode->type == CTP_AUDIO_TEST) || (pCode->type == CTP_SYSTEM))
        &&
         /* Check sur l'id */
         ((pCode->id >= 10000) && (pCode->id <= 99999))
        &&
         /* Check sur la valeur */
         (pCode->value_size != -1)
        &&
         /* Check sur le sens */
         (pCode->sens != -1)
       )
        *validity = 1;
    return EXIT_SUCCESS;
}

int F_Code5T_Infos(sCode5T * pCode)
{
    int i = -1;
    unsigned char c =-1;
    char is_valid = -1;
    if (pCode == NULL)
        return EXIT_FAILURE;
    
    F_Code5T_GetValidity(pCode, &is_valid);
    if (is_valid != 1) {
        SLOGT(LOG_ERR, "Infos sur le message: Invalide ! (type %d, id %d, taille valeur %d)\n", pCode->type, pCode->id, pCode->value_size);
        return EXIT_FAILURE;
    }
    
    //printf("Infos sur le message code 5Tons: type %d, id %d, valeur ", pCode->type, pCode->id);
    printf("Message ");
    if (pCode->sens == GTW_TO_PDT) {
        printf("[GTW -> PDT]");
    } else {
        printf("[GTW <- PDT]");
    }
    printf(" : %d | %d | ", pCode->type, pCode->id);
    if (pCode->value_size == -1)
        printf("-1 ");
    for (i = 0; i < pCode->value_size; i++) {
        c = *(pCode->code_val + i);
        if ((!isalnum(c)) || (pCode->id == CID_HEARTBEAT) || (pCode->id == CID_HEARTBEAT_ACK) ){
            printf("0x%02x ", c);
        } else {
            printf("%c ", c);
        }
    }
    printf("\n");
    
    //printf("Chaine 5-tons format PDT  \"%s\"\n", pCode->cstr);
    
    //if (pCode->circuit_bibl != -1)
    //    printf("\tChaine au 5-tons format BIBL \"%s\" sur circuit %d\n", pCode->code_bibl, pCode->circuit_bibl);
    
    return EXIT_SUCCESS;
}

int F_Code5T_InfosAll (sCode5T ** ppCodes, int nb_elements)
{
    int i = -1;
    for (i = 0; i < nb_elements; i++) {
        F_Code5T_Infos(ppCodes[i]);
    }

    return EXIT_SUCCESS;
}

int F_Code5T_Set_Str(sCode5T * pCode, unsigned char * code_str, int sens)
{
    int value_size = -1;
    if (pCode == NULL)
        return EXIT_FAILURE;


    F_Code5T_Set_Direction(pCode, sens);
    
    /* Décodage du type de code */
    F_Code5T_StrGrab_Type(code_str, &(pCode->type));
    
    /* Décodage de l'identifiant du code */
    F_Code5T_StrGrab_Id(code_str, &(pCode->id));
    
    /* Décodage de la valeur associée */
    if (pCode->type == CTP_AUDIO_TEST)
        value_size = CODE_VALUE_TEST_AUDIO_LEN;
    else
        value_size = CODE_VALUE_LEN;
    pCode->value_size = value_size;
    
    F_Code5T_StrGrab_Value(code_str, pCode->code_val, value_size);
    /* Concatenation des données sous forme de chaine d'octets */
    F_Code5T_MkNum2Str(pCode);
    return EXIT_SUCCESS;
}

int F_Code5T_Set_Num(sCode5T * pCode, int sens, int code_type, int code_id, unsigned char * code_value, int value_size)
{
    if (pCode == NULL)
        return EXIT_FAILURE;
    F_Code5T_Reset(pCode);
    F_Code5T_Set_Direction(pCode, sens);
    F_Code5T_Set_Type(pCode, code_type);
    F_Code5T_Set_Id(pCode, code_id);
    F_Code5T_Set_Value(pCode, code_value, value_size);
    /* Concatenation des données sous forme de chaine d'octets */
    F_Code5T_MkNum2Str(pCode);
    return EXIT_SUCCESS;
}

int F_Code5T_Set_Direction(sCode5T * pCode, int sens)
{
    if (pCode == NULL)
        return EXIT_FAILURE;
        
    pCode->sens = sens;
    return EXIT_SUCCESS;
}

int F_Code5T_Set_Type(sCode5T * pCode, int code_type)
{
    if (pCode == NULL)
        return EXIT_FAILURE;
        
    pCode->type = code_type;
    return EXIT_SUCCESS;
}

int F_Code5T_Set_Id(sCode5T * pCode, int code_id)
{
    if (pCode == NULL)
        return EXIT_FAILURE;

    pCode->id = code_id;
    return EXIT_SUCCESS;
}

int F_Code5T_Set_Value(sCode5T * pCode, unsigned char * code_value, int value_size)
{
    int i = -1;
    
    if (pCode == NULL)
        return EXIT_FAILURE;
        
    pCode->value_size = value_size;

    memset(pCode->code_val, 0, CODE_VALUE_TEST_AUDIO_LEN * sizeof (unsigned char));
    for (i = 0; i < pCode->value_size; i++)
        *(pCode->code_val + i) = *(code_value + i);
    return EXIT_SUCCESS;
}

int F_Code5T_MkNum2Str(sCode5T * pCode)
{
    int i = -1;
    char is_valid = -1;
    
    if (pCode == NULL)
        return EXIT_FAILURE;

    F_Code5T_GetValidity(pCode, &is_valid);
    if (is_valid != 1)
        return EXIT_FAILURE;

    /* Initialisation de la chaine de caractère */
    pCode->cstr_len = CODE_TYPE_LEN + CODE_ID_LEN + pCode->value_size;
    memset(pCode->cstr, 0, CODE_MAX_LEN * sizeof (unsigned char));

    /* Conversion du type */
    snprintf((char*)(pCode->cstr + CODE_TYPE_IDX), CODE_TYPE_LEN+1, "%u", pCode->type);
    
    /* Conversion de l'identifiant */
    if (pCode->id != 0) {
        snprintf((char*)(pCode->cstr + CODE_ID_IDX),   CODE_ID_LEN+1,   "%u", pCode->id);
    } else {
        /* Cas spécial: pCode->id = 0 doit donner "00000"*/
        for (i = 0; i < CODE_ID_LEN; i++) {
            snprintf((char*)(pCode->cstr + CODE_ID_IDX + i), 2, "%u", 0);
        }
    }
    /* Conversion de la valeur */
    memcpy((pCode->cstr + CODE_VALUE_IDX), pCode->code_val, pCode->value_size);
    return EXIT_SUCCESS;
}

int F_Code5T_StrGrab_Type(unsigned char * code_str, int * code_type)
{
    unsigned char * s_type = NULL;
    
    if (code_str == NULL)
        return EXIT_FAILURE;

    *code_type = -1;
    
    s_type = (unsigned char *)malloc((CODE_TYPE_LEN+1) * sizeof (unsigned char));
    memset(s_type, 0, CODE_TYPE_LEN + 1);
    memcpy(s_type, (code_str + CODE_TYPE_IDX), CODE_TYPE_LEN);
        
    if (opt_isnum((char *)s_type) == 1)
        *code_type = atoi((char *)s_type);
    free(s_type);
    return EXIT_SUCCESS;
}

int F_Code5T_StrGrab_Id(unsigned char * code_str, int * code_id)
{
    unsigned char * s_cid = NULL;
    
    if (code_str == NULL)
        return EXIT_FAILURE;

    *code_id = -1;
    
    s_cid = (unsigned char *)malloc((CODE_ID_LEN + 1) * sizeof (unsigned char));
    memset(s_cid, 0, CODE_ID_LEN + 1);
    memcpy(s_cid, (code_str + CODE_ID_IDX), CODE_ID_LEN);
        
    if (opt_isnum((char *)s_cid) == 1)
        *code_id = atoi((char *)s_cid);
    free(s_cid);
    return EXIT_SUCCESS;
}

int F_Code5T_StrGrab_Value(unsigned char * code_str, unsigned char * code_value, int value_size)
{
    unsigned char * s_cvalue = NULL;
    int i = 0;
    if (code_str == NULL)
        return EXIT_FAILURE;
    if (value_size <= 0)
        value_size = CODE_VALUE_LEN;     /* Taille par défaut d'une valeur contenue dans un code */
    
    s_cvalue = (unsigned char *)malloc((value_size + 1) * sizeof (unsigned char));
    memset(s_cvalue, 0, value_size + 1);
    memcpy(s_cvalue, (code_str + CODE_VALUE_IDX), value_size);

    for (i = 0; i < value_size; i++)
        *(code_value + i) = (unsigned char)*(s_cvalue + i);
    free(s_cvalue);
    return EXIT_SUCCESS;
}
 
int F_Code5T_Set_BiblCode(sCode5T * pCode, unsigned char * str_biblcode, int circuit, int sens)
{
    int len = -1;

    if (pCode == NULL)
        return EXIT_FAILURE;
    F_Code5T_Reset(pCode);

    /* Vérifie la longueur de la chaine de caractère contenant le code 5-tons BIBL */
    if ((len = strlen((char *)str_biblcode)) != CODE_BIBL_LEN) {
        SLOGT(LOG_ERR, "F_Code5T_Set_BiblCode: erreur dans la longueur de la chaine de caractère du code 5-tons BIBL (%d)\n", len);
        return EXIT_FAILURE;
    }
    memset(pCode->code_bibl, 0, CODE_BIBL_LEN * sizeof (unsigned char));
    memcpy(pCode->code_bibl, str_biblcode, CODE_BIBL_LEN * sizeof (unsigned char));
    
    /* Vérifie le numéro de circuit */
    if ((circuit < CTP_CIRCUIT_1) || (circuit > CTP_CIRCUIT_4)) {
        SLOGT(LOG_ERR, "F_Code5T_Set_BiblCode: erreur dans le numéro de circuit fourni (%d)\n", circuit);
        return EXIT_FAILURE;
    }
    pCode->circuit_bibl = circuit;
    F_Code5T_Set_Direction(pCode, sens);
    F_Code5T_BIBL2PDT(pCode);
    return EXIT_SUCCESS;
}

int F_Code5T_BIBL2PDT(sCode5T * pCode)
{
    int code_id = -1;
    unsigned char action5t = 0;
    unsigned char * s_cid = NULL;
    
    if (pCode == NULL)
        return EXIT_FAILURE;

    /* Décodage de l'identifiant PTO */
    s_cid = (unsigned char *)malloc((CODE_BIBL_LEN) * sizeof (unsigned char));
    memset(s_cid, 0, CODE_BIBL_LEN);
    memcpy(s_cid, pCode->code_bibl, CODE_BIBL_LEN - 1); /* On copie les 4 premiers octets */
    if (opt_isnum((char *)s_cid) == 1)
        code_id = atoi((char *)s_cid);
    free(s_cid);
    if (code_id < 0) {
        SLOGT(LOG_ERR, "F_Code5T_BIBL2PDT: erreur dans le code_id décodé (%d)\n", code_id);
        return EXIT_FAILURE;
    }
    
    if (code_id != 0) {
        /* Cas standard:
            Le code 5-tons BIBL "50102" sur circuit N, donne "N 50101 2" au format PDT */
        F_Code5T_Set_Type(pCode, pCode->circuit_bibl);
        code_id = (code_id * 10) + 1; /* Pour obtenir un identifiant de 5 chiffres se terminant par 1 */
        F_Code5T_Set_Id(pCode, code_id);
        
        /* Décodage de la valeur de l'action 5-tons */
        memcpy(&action5t, (pCode->code_bibl + CODE_BIBL_LEN - 1), CODE_VALUE_LEN);  /* Le dernier octet de la chaine str_biblcode */
        F_Code5T_Set_Value(pCode, &action5t, CODE_VALUE_LEN);
    }
    else {
        /* Cas spécial: raccrochage 
            Le code 5-tons BIBL "00000" sur circuit N, donne "9 99994 N" au format PDT */
        F_Code5T_Set_Type(pCode, CTP_SYSTEM);
        code_id = CID_FREE_CIRCUIT;
        F_Code5T_Set_Id(pCode, code_id);
        action5t = CVL_C5T_0 + pCode->circuit_bibl;  /* Pour obtenir la valeur ASCII du circuit */
        F_Code5T_Set_Value(pCode, &action5t, CODE_VALUE_LEN);
    }
    
    /* Concatenation des données sous forme de chaine d'octets */
    F_Code5T_MkNum2Str(pCode);

    return EXIT_SUCCESS;
}

int F_Code5T_PDT2BIBL(sCode5T * pCode)
{
    unsigned char action5t = 0;
    
    if (pCode == NULL)
        return EXIT_FAILURE;

    if ((pCode->type == -1) || (pCode->id == -1) || (pCode->value_size == -1))
        return EXIT_FAILURE;

    memset(pCode->code_bibl, 0, (CODE_BIBL_LEN + 1) * sizeof (unsigned char));
    snprintf((char*)(pCode->code_bibl_raw), (CODE_BIBL_LEN + 1) * sizeof (unsigned char), "%u", pCode->id);
    
    memset(pCode->code_bibl, 0, CODE_BIBL_LEN * sizeof (unsigned char));
    snprintf((char*)(pCode->code_bibl), CODE_BIBL_LEN * sizeof (unsigned char), "%u", pCode->id);
    if (pCode->value_size == CODE_VALUE_LEN) {
        action5t = *(pCode->code_val);
        /* Cas spécial: PTO pas en ligne */
        if (action5t == CVL_C5T_6)
            action5t = CVL_C5T_3;
            
        memcpy((pCode->code_bibl + CODE_BIBL_LEN - 1), &action5t, pCode->value_size);
        pCode->action = (int)action5t;

    } else {
        SLOGT(LOG_ERR, "Erreur longueur valeur action 5-tons (%d)\n", pCode->value_size);
        return EXIT_FAILURE;
    }
    pCode->circuit_bibl = pCode->type;
    
    return EXIT_SUCCESS;
}

int F_Code5T_TestUsage(void)
{
    sCode5T *  pCodeTest = NULL;
    F_Code5T_Init(&pCodeTest);
    
    /* Affectation d'un code par sa chaine de caractère au format PDT */
    const unsigned char strtest[] = "1501012";
    F_Code5T_Set_Str(pCodeTest, (unsigned char *)strtest, RCV);
    F_Code5T_PDT2BIBL(pCodeTest);
    F_Code5T_Infos(pCodeTest);
    
    /* Affectation d'un code par sa chaine de caractère BIBL */
    F_Code5T_Reset(pCodeTest);
    F_Code5T_Set_BiblCode(pCodeTest, (unsigned char *)"50102", 4, FROM_NC_CORE);
    F_Code5T_Infos(pCodeTest);
    
    /* Affectation d'un code par sa chaine de caractère BIBL - Cas spécial*/
    F_Code5T_Reset(pCodeTest);
    F_Code5T_Set_BiblCode(pCodeTest, (unsigned char *)"00000", 3, TO_NC_CORE);
    F_Code5T_Infos(pCodeTest);
    
    /* Affectation d'un code par sa chaine de caractère au format PDT */
    F_Code5T_Reset(pCodeTest);
    F_Code5T_Set_Str(pCodeTest, (unsigned char *)"2501013", SND);
    F_Code5T_PDT2BIBL(pCodeTest);
    F_Code5T_Infos(pCodeTest);
    
    /* Affectation d'un code par sa chaine de caractère au format PDT */
    F_Code5T_Reset(pCodeTest);
    F_Code5T_Set_Str(pCodeTest, (unsigned char *)"4501016", GTW_TO_PDT);
    F_Code5T_PDT2BIBL(pCodeTest);
    F_Code5T_Infos(pCodeTest);
    
    /* Affectation d'un code par sa chaine de caractère au format BIBL */
    F_Code5T_Reset(pCodeTest);
    F_Code5T_Set_BiblCode(pCodeTest, (unsigned char *)"5010B", 4, TO_NC_CORE);
    F_Code5T_Infos(pCodeTest);
    
    /* Affectation d'un code par des caracteristiques numérique */
    unsigned char val = 2;
    F_Code5T_Reset(pCodeTest);
    F_Code5T_Set_Num(pCodeTest, TO_PDT, CTP_SYSTEM, CID_FREE_CIRCUIT, &val, CODE_VALUE_LEN);
    F_Code5T_PDT2BIBL(pCodeTest);
    F_Code5T_Infos(pCodeTest);
    
    F_Code5T_Free(&pCodeTest);
    return EXIT_SUCCESS;
}
