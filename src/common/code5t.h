/*!
 * \file code5t.h
 * \brief Gestion des codes 5 tons
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 20/06/2014
 *
 * Gestion des codes 5 tons
 * 
 */

#ifndef _CODE_5T_H_
#define _CODE_5T_H_

/*!
 * \def CODE_TYPE_IDX
 * \brief Index de l'information "circuit" dans une chaine de caractère 5-tons
 */
#define CODE_TYPE_IDX 0
/*!
 * \def CODE_ID_IDX
 * \brief Index de l'information "identifiant" dans une chaine de caractère 5-tons
 */
#define CODE_ID_IDX      1
/*!
 * \def CODE_VALUE_IDX
 * \brief Index de l'information "action" dans une chaine de caractère 5-tons
 */
#define CODE_VALUE_IDX  6
/*!
 * \def CODE_TYPE_LEN
 * \brief Taille de l'information "circuit" dans une chaine de caractère 5-tons
 */
#define CODE_TYPE_LEN 1
/*!
 * \def CODE_ID_LEN
 * \brief Taille de l'information "identifiant" dans une chaine de caractère 5-tons
 */
#define CODE_ID_LEN      5
/*!
 * \def CODE_VALUE_LEN
 * \brief Taille de l'information "action" dans une chaine de caractère 5-tons
 */
#define CODE_VALUE_LEN  1
/*!
 * \def CODE_VALUE_TEST_AUDIO_LEN
 * \brief Taille de l'information "resultat du test audio" dans une chaine de caractère 5-tons
 */
#define CODE_VALUE_TEST_AUDIO_LEN  4
/*!
 * \def CODE_BIBL_LEN
 * \brief Taille d'un code 5-tons au format d'échange BIBL (un "vrai" code 5-tons)
 */
#define CODE_BIBL_LEN  5

/*!
 * \def CODE_MAX_LEN
 * \brief Taille maximale du message à envoyer
 */
#define CODE_MAX_LEN CODE_TYPE_LEN + CODE_ID_LEN + CODE_VALUE_TEST_AUDIO_LEN

/*!
 * \enum Pto_Action code5t.h
 * \brief Liste des actions associées à un élement \a sCode5T
 */
enum Pto_Action {
    PTO_ACT_IDENT       = 1,    /*!< Identification: Utilisé lors de l'appel depuis un PTO */
    PTO_ACT_CALL        = 2,    /*!< Appel et connexion: Utilisé lors de l'appel vers un PTO */
    PTO_ACT_CALLEND     = 3,    /*!< Raccrochage: Utilisé pour raccrocher un appel */
    PTO_ACT_DEFALIM     = 4,    /*!< Défaut d'alimentation */
    PTO_ACT_CALLBACK    = 5,    /*!< Retour d'appel */
    PTO_ACT_OFFLINE     = 6     /*!< PTO inexistant ou hors ligne */
};

/*!
 * \enum Code_Type code5t.h
 * \brief Liste des types associés à un élement \a sCode5T
 */
enum Code_Type {
    CTP_CIRCUIT_1        = 1,   /*!< Message pour circuit 1 */
    CTP_CIRCUIT_2        = 2,   /*!< Message pour circuit 2 */
    CTP_CIRCUIT_3        = 3,   /*!< Message pour circuit 3 */
    CTP_CIRCUIT_4        = 4,   /*!< Message pour circuit 4 */
    CTP_AUDIO_TEST       = 8,   /*!< Message de resultat de test audio */
    CTP_SYSTEM           = 9    /*!< Message de type système */
};

/*!
 * \enum Code_Id code5t.h
 * \brief Liste des identifiants associés à un élement \a sCode5T
 */
enum Code_Id {
    CID_PDT_START       = 99900,     /*!< Code de fonctionnement au démarrage */
    CID_BIBL_STATUS     = 99992,    /*!< Information sur l'état d'une carte BIBL */
    CID_FREE_CIRCUIT    = 99994,    /*!< Libération de circuit */
    CID_HEARTBEAT       = 99995,    /*!< Envoi de la trame de vie */
    CID_HEARTBEAT_ACK   = 99996,    /*!< Réponse à la trame de vie */
    CID_LAST_TERM       = 99999,    /*!< Dernière borne */
    CID_TEST_ALIM_AUTO  = 99999
};

/*!
 * \enum Code_Value_5tons code5t.h
 * \brief Liste des valeurs associées à un élement \a sCode5T de type \a CTP_CIRCUIT_X
 *
 * Correspond à la valeur des caractères ASCII pouvant être utilisés pour définir un code 5-tons
 */
enum Code_Value_5tons {
    CVL_C5T_0           = 0x30,     /*!< Indication de sonnerie d'un PTO */
    CVL_C5T_1           = 0x31,     /*!< Identification: Utilisé lors de l'appel depuis un PTO / Décrochage: Utilisé lorsque le PTO répond à un appel du pupitre */
    CVL_C5T_2           = 0x32,     /*!< Appel et connexion: Utilisé lors de l'appel vers un PTO */
    CVL_C5T_3           = 0x33,     /*!< Raccrochage: Utilisé pour raccrocher un appel */
    CVL_C5T_4           = 0x34,     /*!< Défaut d'alimentation */
    CVL_C5T_5           = 0x35,     /*!< Retour d'appel */
    CVL_C5T_B           = 0x42,     /*!< Demande de test audio */
    CVL_C5T_6           = 0x36,     /*!< PTO inexistant ou hors ligne */
    CVL_C5T_7           = 0x37,
    CVL_C5T_8           = 0x38,
    CVL_C5T_9           = 0x39,
    CVL_C5T_D           = 0x44
};

/*!
 * \enum Code_Value_Sys code5t.h
 * \brief Liste des valeurs associées à un élement \a sCode5T de type \a CTP_SYSTEM
 *
 * Correspond à la valeur des caractères ASCII pouvant être utilisés pour définir un code 5-tons
 */
enum Code_Value_Sys {
    CVL_SYS_C_ALL       = 0x30,
    CVL_SYS_C_1         = 0x31,     /*!< Identification: Utilisé lors de l'appel depuis un PTO */
    CVL_SYS_C_2         = 0x32,     /*!< Appel et connexion: Utilisé lors de l'appel vers un PTO */
    CVL_SYS_C_3         = 0x33,     /*!< Raccrochage: Utilisé pour raccrocher un appel */
    CVL_SYS_C_4         = 0x34,     /*!< Défaut d'alimentation */
    CVL_SYS_BIBL_HS     = 0x37,
    CVL_SYS_BIBL_OK     = 0x38,
    CVL_SYS_LAST_TERM   = 0x38,
    CVL_SYS_PDT_START   = 0x38
};

enum Code_SendToFrom {
    GTW_TO_PDT      = 0,
    PDT_TO_GTW      = 1,
    SND             = 0,
    RCV             = 1,
    TO_PDT          = 0,
    FROM_PDT        = 1,
    TO_NC_CORE      = 1,
    FROM_NC_CORE    = 0
};

/*!
 * \struct sCode5T code5t.h
 * \brief Structure de gestion des codes 5-tons
 */
typedef struct tCode5T
{
    int     type;                                   /*!< Type de code */
    int     id;                                     /*!< Identifiant du code (code 5-Tons PTO ou code systeme )*/
    int     action;                                 /*!< Code d'action: Appel, raccrocher */
    unsigned char code_val[CODE_VALUE_TEST_AUDIO_LEN];  /*!< Tableau de char contenant la valeur du code sur \a CODE_VALUE_TEST_AUDIO_LEN octets maximum */
    int     value_size;                             /*!< Taille de la variable contenant la valeur du code */
    unsigned char * cstr;                           /*!< Code 5-tons concatené sous forme de chaine de caractere, au format PDT */
    int     cstr_len;                               /*!< Taille de la chaine de caractère représentant le code à envoyer */
    unsigned char * code_bibl;                      /*!< Chaine de caractère contenant le "vrai" code 5-tons à échanger avec la BIBL, incluant l'offset d'action de la pyramide 5-tons */
    unsigned char * code_bibl_raw;                  /*!< Chaine de caractère contenant le code 5-tons sans l'offset d'action de la pyramide 5-tons */
    int     circuit_bibl;                           /*!< Circuit BIBL */
    int     pto_status;                             /*!< Status associé au PTO identifié par \a id (dans le cas ou il s'agit d'un code de type CTP_CIRCUIT_X) */
    int     sens;
    int (*set_num)(struct tCode5T*,int,int,int);    /*!< Pointeur sur fonction */
    int (*set_str)(struct tCode5T*,unsigned char*); /*!< Pointeur sur fonction */
}sCode5T;


/*!
 * \fn int F_Code5T_BIBL2PDT(sCode5T * pCode)
 * \brief Converti le code 5-tons du format BIBL vers le format PDT
 * \param[in,out] pCode: adresse du pointeur sur la structure \a sCode5T
 * \return 0
 *
 */
int F_Code5T_BIBL2PDT(sCode5T * pCode);
/*!
 * \fn int F_Code5T_PDT2BIBL(sCode5T * pCode)
 * \brief Converti le code 5-tons du format PDT vers le format BIBL
 * \param[in,out] pCode: pointeur sur la structure \a sCode5T
 * \return 0
 *
 */
int F_Code5T_PDT2BIBL(sCode5T * pCode);

int F_Code5T_GetValidity(sCode5T * pCode, char * validity);

/*!
 * \fn int F_Code5T_Init(sCode5T ** ppCode)
 * \brief Initialise une structure \a sCode5T
 * \param[in,out] ppCode: adresse du pointeur sur la structure \a sCode5T
 * \return 0
 *
 * \a F_Code5T_Init effectue les opérations suivantes:
 * \li alloue la mémoire nécessaire pour l'élement \a pCode passée en paramètre
 * \li initialise les membres de cette structure à leur valeur par défaut
 */
int F_Code5T_Init(sCode5T ** ppCode);

/*!
 * \fn int F_Code5T_InitAll(sCode5T *** pppCodes, int nb_elements)
 * \brief Initialise une série de structures \a sCode5T
 * \param[in,out] pppCodes: adresse du tableau de pointeurs sur élément \a sCode5T
 * \param[in] nb_elements: nombre d'élements \a sCode5T à initialiser
 * \return 0
 *
 * \a F_Code5T_InitAll prend en paramètre un pointeur de pointeurs sur une structure \a sCode5T par son adresse, (ce qui en fait un triple pointeur !).
 * Pour faire simple, voyons \a pCodes comme un tableau de pointeurs sur des élements \a sCode5T.\n
 * Elle initialise individuellement les \a nb_elements élements \a sCode5T pointés par le pointeur de pointeurs \a pCodes passé en paramètre. 
 * 
 */
int F_Code5T_InitAll(sCode5T *** pppCodes, int nb_elements);

/*!
 * \fn int F_Code5T_Free(sCode5T ** ppCode)
 * \brief Libére la mémoire allouée à un élement \a sCode5T
 * \param[in,out] ppCode: adresse du pointeur sur l'élément \a sCode5T
 * \return 0
 *
 * En sortie de la fonction, le pointeur \a pCode n'est plus alloué et vaut NULL
 */
int F_Code5T_Free(sCode5T ** ppCode);

/*!
 * \fn int F_Code5T_FreeAll(sCode5T *** pppCodes, int nb_elements)
 * \brief Libére la mémoire allouée pour tous les éléments \a sCode5T
 * \param[in,out] pppCodes: adresse du tableau de pointeurs sur les éléments \a sCode5T
 * \param[in] nb_elements: nombre d'élements \a sCode5T à liberer
 * \return 0
 *
 * \a F_Code5T_FreeAll prend en paramètre un pointeur de pointeurs sur une structure \a sCode5T par son adresse, (ce qui en fait un triple pointeur !).
 * Pour faire simple, voyons \a pCodes comme un tableau de pointeurs sur des élements sCode5T.\n
 * En sortie de la fonction, le pointeur \a pCodes n'est plus alloué et vaut NULL
 */
int F_Code5T_FreeAll(sCode5T *** pppCodes, int nb_elements);

/*!
 * \fn int F_Code5T_Reset(sCode5T * pCode)
 * \brief Réinitialise l'élement \a pCode aux valeurs par défaut
 * \param[in] pCode: pointeur sur l'élément \a sCode5T
 * \return 0 si OK, -1 si le pointeur passé en paramètre n'est pas alloué
 */
int F_Code5T_Reset(sCode5T * pCode);

/*!
 * \fn int F_Code5T_Infos(sCode5T * pCode)
 * \brief Affiche les informations contenues dans un élement \a sCode5T
 * \param[in] pCode: pointeur sur l'élément \a sCode5T
 * \return 0 si OK, -1 si le pointeur passé en paramètre n'est pas alloué
 */
int F_Code5T_Infos(sCode5T * pCode);

/*!
 * \fn int F_Code5T_InfosAll (sCode5T ** ppCodes, int nb_elements)
 * \brief Affiche les informations de tous les élements \a sCode5T pointés par \a pCodes
 * \param[in] ppCodes: tableau de pointeurs sur les éléments \a sCode5T
 * \param[in] nb_elements: nombre d'élements \a sCode5T à afficher
 * \return 0 si OK, -1 si le pointeur passé en paramètre n'est pas alloué
 */
int F_Code5T_InfosAll (sCode5T ** ppCodes, int nb_elements);

/*!
 * \fn int F_Code5T_StrGrab_Type(unsigned char * code_str, int * code_type)
 * \brief Extrait le type du code à partir la chaine de caractère \a code_str passée en paramètre
 * \param[in] code_str: chaine de caractère définissant le code 5-tons
 * \param[out] code_type: adresse de la variable contenant le type de code
 * \return 0 si OK, -1 si le pointeur passé en paramètre n'est pas alloué
 */
int F_Code5T_StrGrab_Type(unsigned char * code_str, int * code_type);

/*!
 * \fn int F_Code5T_StrGrab_Id(unsigned char * code_str, int * code_id)
 * \brief Extrait l'identifiant du code à partir la chaine de caractère \a code_str passée en paramètre
 * \param[in] code_str: chaine de caractère définissant le code 5-tons
 * \param[out] code_id: adresse de la variable contenant l'identifiant du code
 * \return 0 si OK, -1 si le pointeur passé en paramètre n'est pas alloué
 */
int F_Code5T_StrGrab_Id(unsigned char * code_str, int * code_id);

/*!
 * \fn int F_Code5T_StrGrab_Value(unsigned char * code_str, unsigned char * code_value, int value_size)
 * \brief Extrait la valeur du code à partir la chaine de caractère \a code_str passée en paramètre
 * \param[in] code_str: chaine de caractère définissant le code 5-tons
 * \param[out] code_value: adresse de la variable contenant la valeur du code
 * \param[out] value_size: nombre d'octets sur lequel est encodée la valeur du code
 * \return 0 si OK, -1 si le pointeur passé en paramètre n'est pas alloué
 */
int F_Code5T_StrGrab_Value(unsigned char * code_str, unsigned char * code_value, int value_size);

/*!
 * \fn int F_Code5T_MkNum2Str(sCode5T * pCode)
 * \brief Créer la chaine de caracètre 5-tons à partir des informations numéraires le composant
 * \param[in,out] pCode: pointeur sur un élément sCode5T
 * \return 0 si OK, -1 si le pointeur passé en paramètre n'est pas alloué
 */
int F_Code5T_MkNum2Str(sCode5T * pCode);

/*!
 * \fn int F_Code5T_Set_Type(sCode5T * pCode, int code_type)
 * \brief Afecte un type de code à l'élement sCode5T passé en paramètre
 * \param[in,out] pCode: pointeur sur un élément sCode5T
 * \param[in] code_type: type du code à affecter
 * \return 0 si OK, -1 si le pointeur passé en paramètre n'est pas alloué
 */
int F_Code5T_Set_Type(sCode5T * pCode, int code_type);

/*!
 * \fn int F_Code5T_Set_Id(sCode5T * pCode, int code_id)
 * \brief Affecte un identifiant de code à l'élement sCode5T passé en paramètre
 * \param[in,out] pCode: pointeur sur un élément sCode5T
 * \param[in] code_id: identifiant du code à affecter
 * \return 0 si OK, -1 si le pointeur passé en paramètre n'est pas alloué
 */
int F_Code5T_Set_Id(sCode5T * pCode, int code_id);

/*!
 * \fn int F_Code5T_Set_Value(sCode5T * pCode, unsigned char * code_value, int value_size)
 * \brief Affecte une valeur à l'élement sCode5T passé en paramètre
 * \param[in,out] pCode: pointeur sur un élément sCode5T
 * \param[in] code_value: Adresse de la variable contenant la valeur à affecter
 * \param[in] value_size: taille de la variable contenant la valeur à affecter, en octets
 * \return 0 si OK, -1 si le pointeur passé en paramètre n'est pas alloué
 */
int F_Code5T_Set_Value(sCode5T * pCode, unsigned char * code_value, int value_size);

int F_Code5T_Set_Direction(sCode5T * pCode, int sens);

/*!
 * \fn int F_Code5T_Set_Num(sCode5T * pCode, int code_type, int code_id, unsigned char * code_value, int value_size)
 * \brief Défini les informations d'un code 5-tons à partir des valeurs numériques le composant
 * \param[in,out] pCode: pointeur sur un élément sCode5T
 * \param[in] code_type: type du code à affecter
 * \param[in] code_id: identifiant du code à affecter
 * \param[in] code_value: Adresse de la variable contenant la valeur à affecter
 * \param[in] value_size: taille de la variable contenant la valeur à affecter, en octets
 * \return 0 si OK, -1 si le pointeur passé en paramètre n'est pas alloué
 */
int F_Code5T_Set_Num(sCode5T * pCode, int sens, int code_type, int code_id, unsigned char * code_value, int value_size);

/*!
 * \fn int F_Code5T_Set_Str(sCode5T * pCode, unsigned char * code_str)
 * \brief Défini les informations d'un code 5-tons à partir d'une chaine de caractère
 * \param[in,out] pCode: pointeur sur un élément sCode5T
 * \param[in] code_str: Chaine de caractère de définition
 * \return 0 si OK, -1 si le pointeur passé en paramètre n'est pas alloué
 */
int F_Code5T_Set_Str(sCode5T * pCode, unsigned char * code_str, int sens);

int F_Code5T_Set_BiblCode(sCode5T * pCode, unsigned char * str_biblcode, int circuit, int sens);

int F_Code5T_TestUsage(void);
#endif /* _CODE_5T_H_ */
