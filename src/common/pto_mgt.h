/*!
 * \file pto_mgt.h
 * \brief Gestion des PTO connus du système
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 25/08/2014
  * 
 */

#ifndef _PTO_MGT_H_
#define _PTO_MGT_H_

typedef enum tPto_Status {
    PTO_STATUS_INIT         = 0,
    PTO_STATUS_CALLED       = 1,
    PTO_STATUS_CALLING      = 2,
    PTO_STATUS_RINGING      = 3,
    PTO_STATUS_ONLINE       = 4,
    PTO_STATUS_ENDCALL      = 5,
    PTO_STATUS_OFFLINE      = 6,
    PTO_STATUS_AUDIO_ON     = 7,
    PTO_STATUS_AUDIO_OFF    = 8,
    PTO_STATUS_POWER_DEF    = 9,
    PTO_STATUS_HS           = 10,
    PTO_STATUS_CALLBACK     = 11,
    PTO_STATUS_UNKNOWN      = 12
} ePto_Status;

typedef enum tPto_CommInit {
    PTO_UDP_SERVEUR = 0,
    PTO_UDP_CLIENT  = 1,
    COMM_INIT_BY_PTO = 0,
    COMM_INIT_BY_NC_CORE = 1,
} ePto_CommInit;

typedef struct tPTO
{
    int id;
    int circuit;
    ePto_Status status;
    ePto_CommInit comm_type;
    struct tPTO * pNext;
    struct tPTO * pPrev;
} sPTO;

typedef struct tPTO_List
{
    size_t length;
    struct tPTO * pTail;
    struct tPTO * pHead;
} sPTO_List;


const char * str_ptostatus(int status);

int F_PTO_List_Init(sPTO_List ** ppList);
int F_PTO_List_Delete(sPTO_List ** ppList);


int F_PTO_List_Append(sPTO_List ** ppList, int id, int circuit, ePto_Status status, ePto_CommInit commtype);
int F_PTO_List_Prepend(sPTO_List ** ppList, int id, int circuit, ePto_Status status, ePto_CommInit commtype);
int F_PTO_List_Insert(sPTO_List ** ppList, int position, int id, int circuit, ePto_Status status, ePto_CommInit commtype);
int F_PTO_List_Update(sPTO_List ** pList, int position, int id, int circuit, ePto_Status status, ePto_CommInit commtype);
int F_PTO_List_Remove(sPTO_List ** ppList, int position);

int F_PTO_List_GetNbElements(sPTO_List * pList);
int F_PTO_List_DisplayContent(sPTO_List * pList);
int F_PTO_List_FindPtoId(sPTO_List * pList, int pto_id);
int F_PTO_List_FindCircuit(sPTO_List * pList, int circuit);
int F_PTO_List_FindOnlinePto_ByCircuit(sPTO_List * pList, int circuit);

sPTO * F_PTO_List_GetPto(sPTO_List * pList, int position);

int F_PTO_Test_Usage(void);




#endif /* _PTO_MGT_H_ */
