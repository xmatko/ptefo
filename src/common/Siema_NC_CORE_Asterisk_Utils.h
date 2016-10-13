////////////////////////////////////////////////////////////////////////////////
//  SYSTEME      : NUMCOM 3000
//  SOUS SYSTEME : UCC.32             LOGICIEL      : Asterisk
//  FICHIER      : $RCSfile: Siema_NC_CORE_Asterisk_Utils.hv $
//  EMPLACEMENT  : $Source: Siema_NC_CORE_Asterisk_Utils.h,v $
//
//  AUTEUR       : Fabien DESSEREE     DATE CREATION : 23/06/2008 (entête)
//  CONTENU      :
////////////////////////////////////////////////////////////////////////////////
//  REVISION     : $Revision: 1.0 $
//  DATE         : $Date: 2008/06/23 13:09:20 $
//  PAR          : $Locker: integrateur $
//  HISTORIQUE   : $Log: Siema_NC_CORE_Asterisk_Utils.h,v $
//  HISTORIQUE   : Revision 1.3  2007/11/23 13:09:20  desseree
////////////////////////////////////////////////////////////////////////////////


#ifndef _SIEMA_UTILS_H_
#define _SIEMA_UTILS_H_

#include <netdb.h>


///////////
// types //
///////////
typedef union int_char
{
    int i;
    char c[sizeof(int)];
}t_int_char;

typedef union short_char
{
    short w;
    char c[sizeof(short)];
}t_short_char;

///////////////////////////
// gestion des  Messages //
///////////////////////////
typedef struct chaine *p_chaine;
typedef struct chaine
{
    short longueur;
    char *chaine;
    p_chaine next;
}t_chaine;

typedef struct message_3num_nchaine *p_message_3num_nchaine;
typedef struct message_3num_nchaine
{
    int longueur;
    short type;
    short sous_type;
    int num1;
    int num2;
    int num3;
    short nb_chaines;
    t_chaine *liste_chaine;
}t_message_3num_nchaine;

//////////////////////
// type de messages //
//////////////////////
#define TYPE_3NUM_NCHAINE 4

////////////////////////////
// sous-types de messages //
////////////////////////////
#include "headers-communs/trunk/MSGConst.h"
/*
#define MSG_APPEL_ENTRANT       3000
#define MSG_APPELER_REL         3001
#define MSG_FIN_REL             3002
#define MSG_ACK                 3003
#define MSG_ECHEC               3004

#define MSG_APPEL_INTER_PUPITRE 3011
#define MSG_APPEL_SORTANT       3012

#define MSG_NC_CORE_PILOTER_SORTIE      3016
#define MSG_MASK_ENTREES        3017
#define MSG_ASK_ETAT_ENTREE     3018
#define MSG_ETAT_ENTREES        3019


#define MSG_NC_CORE_PING        3020
#define MSG_NC_CORE_ACK         3021
*/


//////////////////////////
// gestion des messages //
//////////////////////////

// cree un message vide //
// ATTENTION : le message retourne doit etre libere par appel a SiemaDeleteMessage()!
t_message_3num_nchaine *SiemaNewMessage(void);
// supprime un message et toutes ses chaines associees //
int SiemaDeleteMessage(t_message_3num_nchaine *msg);
// ajoute une chaine au message (et met a jour la taille du message) //
int SiemaAjoutChaineMessage(t_message_3num_nchaine *msg, const char* chaine, short longueur_chaine);
char *SiemaMSGtoStr(int numMSG); // 2009/03/02 : retourne le nom d'un message en fonction de son numero
void SiemaAfficherMessage(t_message_3num_nchaine *msg);
// Serialise un message pour pouvoir le transmettre sur IP
int SiemaSerialiserMessage(t_message_3num_nchaine *msg, unsigned char *chaine, int tailleMaxChaine);
// Deserialise un message et retourne la structure completee du message //
// ATTENTION : le message retourne doit etre libere par appelle a SiemaDeleteMessage()!
t_message_3num_nchaine *SiemaDeserialiserMessage(unsigned char *chaine, int tailleChaine);
// fin gestion des messages //


/////////
// TCP //
/////////
typedef void* t_CallbackConnexionTCP (void *data);
struct siema_tcp_server_instance
{
    //FILE *f;
    int fd;
    struct sockaddr_in requestor;
    //ast_http_callback callback;
    char ipHoteDist[256];
};

int SiemaSocketSend(int pSocket, unsigned char *buf, int bufSize);
int SiemaSocketRecv(int pSocket, unsigned char *buf, int bufMaxSize, int timeOut_s);
void SiemaSocketClose(int pSocket);

int SiemaGetIPLocale(char *adresse);
int SiemaGetMACAdr(char *name_interface, char *adrMAC, char *adrIP);
//int SiemaGetMACAdr2(char *adrMAC);
int SiemaGetIfName(char *ifName, int sizeIfNameMax);

#ifndef ASTERISK
// serveur //
int DemarrerServeurTCP(int port, t_CallbackConnexionTCP *callbackConnexion, int inThread);
void ArreterServeurTCP();
// exemple de callback de connexion client pour le serveur TCP //
//  void *callbackConnexionClient(void *data)
//  {
//      unsigned char tcpbuffer[2048];
//      struct siema_tcp_server_instance *ser = data;
//      int size;
//
//      size = SiemaSocketRecv(ser->fd, tcpbuffer, TCPBUFFER_SIZE, 10); // 10 s de timeout
//      size = SiemaSocketSend(ser->fd, tcpbuffer, size);
//
//      if (ser->fd > -1) closesocket(ser->fd); // ferme le socket
//      free(ser);  // libere la memoire!!!
//  }
//
#endif // !ASTERISK //

// client //
int ClientConnect(char* strAdress,int nPortNumber, int *m_pSocket, int timeout);


#ifndef ASTERISK
/////////////////////////
// gestion des threads //
/////////////////////////
#define STACK_SIZE 48 * 1024 // taille de la pile des threads
#define SIEMA_PTHREADT_NULL (pthread_t) -1

int siema_pthread_create_stack(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *),
                 void *data, size_t stacksize, const char *file, const char *caller,
                 int line, const char *start_fn);
#define siema_pthread_create(a, b, c, d) siema_pthread_create_stack(a, b, c, d,            \
                                    0,                \
                                     __FILE__, __FUNCTION__,        \
                                     __LINE__, #c)

#define siema_pthread_create_background(a, b, c, d) siema_pthread_create_stack(a, b, c, d,            \
                                       STACK_SIZE,    \
                                       __FILE__, __FUNCTION__,    \
                                       __LINE__, #c)

#endif // !ASTERISK //

////////////////
// Status CPU //
////////////////
int getIdleCPU(void);


#endif // _SIEMA_UTILS_H_ //

