/*!
 * \file gtw_core.h
 * \brief Traitements principaux de la passerelle
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 05/09/2014
 *
 */

#ifndef _GTW_CORE_H_
#define _GTW_CORE_H_
#include "main.h"

enum eFifoList {
    FIFO_NC_CORE_SERV = 0,
    FIFO_NC_CORE_CLI,
    FIFO_PDT_SERV,
    FIFO_PDT_CLI,
    FIFO_NUM_MAX
};

enum eTransfertPermission {
    TP_REFUSED,
    TP_ALLOWED
};

int TestFifoList(sMain * pMain);
void * F_Th_GtwCore(void * arg);

/* Fonction principale du traitement du routage des messages */
int F_GtwCore_Traite_Msg(sMain * pMain, sCode5T * pCodeRcv, int src_fifo_num, sCode5T ** ppCodeAns, int * dst_fifo_num);

/* Fonction de traitement du routage des messages en fonction de leur type */
int F_GtwCore_Traite_Msg_TypePTO(sMain * pMain, sCode5T * pCodeRcv, int src_fifo_num, sCode5T ** ppCodeAns, int * dst_fifo_num);
int F_GtwCore_Traite_Msg_TypeSystem(sMain * pMain, sCode5T * pCodeRcv, int src_fifo_num, sCode5T ** ppCodeAns, int * dst_fifo_num);

/* Fonction de traitements du routage des messages de type PTO */
//int F_GtwCore_Traite_Msg_ActionPTO(sMain * pMain, int pto_idx, sCode5T * pCodeRcv, int src_fifo_num, sCode5T ** ppCodeAns, int * dst_fifo_num);
int F_GtwCore_Traite_Msg_ActionPTO_Serveur(sMain * pMain, int pto_idx, sCode5T * pCodeRcv, int src_fifo_num, sCode5T ** ppCodeAns, int * dst_fifo_num);
int F_GtwCore_Traite_Msg_ActionPTO_Client(sMain * pMain, int pto_idx, sCode5T * pCodeRcv, int src_fifo_num, sCode5T ** ppCodeAns, int * dst_fifo_num);
int F_GtwCore_Traite_Msg_ActionPTO_Global(sMain * pMain, int pto_idx, sCode5T * pCodeRcv, int src_fifo_num, sCode5T ** ppCodeAns, int * dst_fifo_num);

/* Suppression des PTO inactifs dans la liste maintenue */
int F_GtwCode_RemovePTOInactives(sMain * pMain);

#endif /* _GTW_CORE_H_ */
