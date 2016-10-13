////////////////////////////////////////////////////////////////////////////////
//  SYSTEME      : NUMCOM 3000
//  SOUS SYSTEME : UCC.32             LOGICIEL      : Asterisk
//  FICHIER      : $RCSfile: Siema_NC_CORE_Asterisk_Utils.c,v $
//  EMPLACEMENT  : $Source: Siema_NC_CORE_Asterisk_Utils.c,v $
//
//  AUTEUR       : Fabien DESSEREE     DATE CREATION : 23/06/2008 (entête)
//  CONTENU      :
////////////////////////////////////////////////////////////////////////////////
//  REVISION     : $Revision: 1.0 $
//  DATE         : $Date: 2008/06/23 13:09:20 $
//  PAR          : $Locker: integrateur $
//  HISTORIQUE   : $Log: Siema_NC_CORE_Asterisk_Utils.c,v $
//  HISTORIQUE   : Revision 1.3  2007/11/23 13:09:20  desseree
////////////////////////////////////////////////////////////////////////////////


#include "Siema_NC_CORE_Asterisk_Utils.h"
#include "asterisk_logger.h"

#ifdef ASTERISK
#include "asterisk.h"
#include "asterisk/utils.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#ifndef ASTERISK
#include <pthread.h>
#include <poll.h>
#include <signal.h>
#endif // !ASTERISK //

// client TCP //
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>   // gethostbyename //
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#define closesocket close
#define SOCKADDR struct sockaddr
#define SOCKADDR_IN struct sockaddr_in




////////////////////////////////////////////
// fonctions d'affichage pour le debogage //
////////////////////////////////////////////
#ifndef ASTERISK
#include "asterisk_logger.h"

/* MACROS Debug */
extern int _siema_asterisk_debug;
int _siema_asterisk_debug = 1;

char verbose_text_color[20]= color_reset;
char log_text_color[20] = color_reset;
char last_verbose_text_color[20]= color_reset;
char last_log_text_color[20] = color_reset;
int SurvMsgAlloc = 0;
char msginconnu[32];

/* Some defines */
#define SIAMA_AST_DEBUG(a) if (_siema_asterisk_debug) { a; fflush(stdout); }
#define SIAMA_AST_DEBUG2(a) if (_siema_asterisk_debug >= 2) { a; fflush(stdout); }

// change la couleur du text //
void siema_ast_text_color(char *color_verbose, char *color_log)
{
    sprintf(last_verbose_text_color,"%s",verbose_text_color);
    sprintf(last_log_text_color,"%s",log_text_color);
    sprintf(verbose_text_color,"%s",color_verbose);
    sprintf(log_text_color,"%s",color_log);
}

// reinitialise la couleur a l'avant derniere couleur utilisee //
void siema_ast_last_text_color()
{
    sprintf(verbose_text_color,"%s",last_verbose_text_color);
    sprintf(log_text_color,"%s",last_log_text_color);
}

void ast_log(int level, const char *file, int line, const char *function, const char *fmt, ...)
{
    va_list vars;
    va_start(vars,fmt);

    char * lvl_str = (char *)malloc(16 * sizeof (char));
    memset(lvl_str, 0, 16 * sizeof (char));
    
    switch (level) {
        case LOG_DEBUG:
            sprintf(lvl_str, "DEBUG");
            break;
        case LOG_EVENT:
            sprintf(lvl_str, "DEBUG");
            break;
        case LOG_NOTICE:
            sprintf(lvl_str, "DEBUG");
            break;
        case LOG_WARNING:
            sprintf(lvl_str, "DEBUG");
            break;
        case LOG_ERROR:
            sprintf(lvl_str, "DEBUG");
            break;
        /*case LOG_VERBOSE:
            sprintf(lvl_str, "DEBUG");
            break;
        case LOG_DTMF:
            sprintf(lvl_str, "DEBUG");
            break;
            */
        default:
            sprintf(lvl_str, "UNKNOWN");
    }
    printf("%c%s",ESC, log_text_color);  // applique la couleur de texte //
    //SIAMA_AST_DEBUG(printf("LOG: lev:%d file:%s  line:%d func: %s  ", level, file, line, function));
    SIAMA_AST_DEBUG(printf("LOG: lev:%s file:%s  line:%d func: %s  ", lvl_str, file, line, function));
    
    SIAMA_AST_DEBUG(vprintf(fmt, vars));
    printf("%c%s",ESC,color_reset); // reinitialise la couleur de texte //
    fflush(stdout);
    free(lvl_str);
    va_end(vars);
}

void ast_verbose(const char *fmt, ...)
{
    va_list vars;
    va_start(vars,fmt);

    printf("%c%s",ESC, verbose_text_color);  // applique la couleur de texte //
    SIAMA_AST_DEBUG(vprintf(fmt, vars));
    printf("%c%s",ESC,color_reset); // reinitialise la couleur de texte //
    fflush(stdout);

    va_end(vars);
}
#endif // ASTERISK //


///////////////////////////////////////////
// fonctions communes client/serveur TCP //
///////////////////////////////////////////
int SiemaSocketSend(int pSocket, unsigned char *buf, int bufSize)
{
   // ast_log(__LOG_WARNING,"envoie de %d => '%s'\n",bufSize, buf);
    int res;
    int cpt = 0;
    struct timeval select_timeout;
    unsigned char *ptr;
    int toSend;
    fd_set wrfd;

    ptr = buf;
    toSend = bufSize;
    select_timeout.tv_sec = 1;
    select_timeout.tv_usec = 0;

    FD_ZERO(&wrfd);
    FD_SET(pSocket, &wrfd);

    do
    {
        res = select(pSocket+1, NULL, &wrfd, NULL, &select_timeout);
        if (res == -1)
        {
            ast_log(__LOG_WARNING,"Erreur Select\n");
            switch(errno)
            {
                case EINTR:
                break;

                default:
                    return res;
                break;
            }

            continue; // Si on a été interrompu par signal, on relance un select
        }

        if (res == 0) // le timeout a ete atteint sans que quelque chose ne soit recu //
        {
            // ast_log(__LOG_WARNING,"Timeout Select depasse\n");
            ++cpt;
            if( cpt == 3 )
            {
                // On a lance 3 select sans succes, on abandonne
                break;
            }
            else
            {
                continue; // Relance le select
            }
        }

        if (FD_ISSET(pSocket, &wrfd))
        {
            res = send(pSocket, (char*)ptr, toSend, 0);
            if ( res == -1 )
            {
                switch(errno)
                {
                    case EINTR:
                    break;

                    default:
                        return res;
                    break;
                }

                continue; // Si on a été interrompu par signal, on relance un select
            }
            else
            {
                if ( res == toSend )
                {
                    return bufSize;
                }
                else
                {
                    toSend -= res;
                    ptr += res;
                }
            }
        }
    } while (1);

    return -1;
}

int SiemaSocketRecv(int pSocket, unsigned char *buf, int bufMaxSize, int timeOut_s)
{
    int res;
    struct timeval select_timeout;
    fd_set rdfd;

    select_timeout.tv_sec = timeOut_s;
    select_timeout.tv_usec = 0;

    FD_ZERO(&rdfd);
    FD_SET(pSocket, &rdfd);

    res = select(pSocket+1, &rdfd, NULL, NULL, &select_timeout);
    if (res == -1)
    {
        ast_log(__LOG_WARNING,"Erreur Select\n");
        return res;
    }
    if (res == 0) // le timeout a ete atteint sans que quelque chose ne soit recu //
    {
       // ast_log(__LOG_WARNING,"Timeout Select depasse\n");
        return res;
    }

    if (FD_ISSET(pSocket, &rdfd))
    {
        return recv(pSocket, (char*)buf, bufMaxSize, 0 );
    }

    return -1;
}


void SiemaSocketClose(int pSocket)
{
    if (pSocket > -1) closesocket(pSocket);
}


// TODO : on renvoi la derniere adresse IP de la liste uniquement (sauf s'il s'agit de 127.0.0.1) ....
// PB : retourne 127.0.0.1 si /etc/hosts ne contient pas "adresseIP" localhost
// -> dans ce ca utiliser la recherche de l'adresse IP avec SiemaGetMACAdr()
int SiemaGetIPLocale(char *adresse)
{
   struct hostent *host;
   struct in_addr **adr;
   char* HostName="localhost"; //
   int nb_adr = 0;

   // par defaut on retour 127.0.0.1 //
   sprintf(adresse,"127.0.0.1");
#ifndef ASTERISK
   if((host = gethostbyname(HostName)) != NULL)
#else // ASTERISK //
    struct ast_hostent ahp;
   if((host = ast_gethostbyname(HostName,&ahp)) != NULL)
#endif // ASTERISK //
   {
      // parcourt des adresses IP
      for (adr=(struct in_addr **)host->h_addr_list; *adr; adr++)
      {
#ifndef ASTERISK
         if (strcmp((char*)inet_ntoa(**adr),"127.0.0.1") != 0)
            sprintf(adresse,"%s", inet_ntoa(**adr));
#else // ASTERISK //
        if (strcmp((char*)ast_inet_ntoa(**adr),"127.0.0.1") != 0)
            sprintf(adresse,"%s", ast_inet_ntoa(**adr));
#endif // ASTERISK //

         nb_adr++;
      }
   }
   else
   {
      // echec de la resolution
      return -1;
   }
   return nb_adr;
}


// name_interface="eth0", "eth1", ...
// si adrIP!=NULL, retourne aussi l'adresse IP de cette interface
int SiemaGetMACAdr(char *name_interface, char *adrMAC, char *adrIP)
{
    struct ifreq ifr;
    int sock, j, k;
    char *p, addr[32], mask[32], mac[32];

    if (adrMAC != NULL) sprintf(adrMAC,"00:00:00:00:00:00");
    if (adrIP != NULL) adrIP[0] = '\0';

    sock=socket(AF_INET, SOCK_DGRAM,0); //PF_INET, SOCK_STREAM, 0);
    if (-1==sock)
    {
        ast_log(__LOG_WARNING,"Erreur creation Socket\n");
        return -1;
    }

    strncpy(ifr.ifr_name,name_interface,sizeof(ifr.ifr_name)-1);

    ifr.ifr_name[sizeof(ifr.ifr_name)-1]='\0';

    if (-1==ioctl(sock, SIOCGIFADDR, &ifr))
    {
        ast_log(__LOG_WARNING,"Erreur ioctl %s\n",strerror(errno));
        return -1;
    }
#ifndef ASTERISK
    p=inet_ntoa(((struct sockaddr_in *)(&ifr.ifr_addr))->sin_addr);
#else // ASTERISK //
    p=ast_inet_ntoa(((struct sockaddr_in *)(&ifr.ifr_addr))->sin_addr);
#endif // ASTERISK //
    strncpy(addr,p,sizeof(addr)-1);
    addr[sizeof(addr)-1]='\0';

    if (-1==ioctl(sock, SIOCGIFNETMASK, &ifr))
    {
        ast_log(__LOG_WARNING,"Erreur ioctl %s\n",strerror(errno));
        return -1;
    }
#ifndef ASTERISK
    p=inet_ntoa(((struct sockaddr_in *)(&ifr.ifr_netmask))->sin_addr);
#else // ASTERISK //
    p=ast_inet_ntoa(((struct sockaddr_in *)(&ifr.ifr_netmask))->sin_addr);
#endif // ASTERISK //
    strncpy(mask,p,sizeof(mask)-1);
    mask[sizeof(mask)-1]='\0';

    if (-1==ioctl(sock, SIOCGIFHWADDR, &ifr))
    {
        ast_log(__LOG_WARNING,"Erreur ioctl %s\n",strerror(errno));
        return -1;
    }
    for (j=0, k=0; j<6; j++)
    {
        k+=snprintf(mac+k, sizeof(mac)-k-1, j ? ":%02X" : "%02X",
            (int)(unsigned int)(unsigned char)ifr.ifr_hwaddr.sa_data[j]);
    }
    mac[sizeof(mac)-1]='\0';

    /*printf("\n");
    printf("name:    %s\n",ifr.ifr_name);
    printf("address: %s\n",addr);
    printf("netmask: %s\n",mask);
    printf("macaddr: %s\n",mac);
    printf("\n");*/
    if (adrMAC != NULL) sprintf(adrMAC,"%s", mac);
    if (adrIP != NULL) sprintf(adrIP,"%s", addr);

    close(sock);
    return 0;
}



// cette fonction de recuperation d'adresse MAC ne fonctionne qu'en
// root à cause de SOCK_RAW.
/*#include <netinet/if_ether.h>
int SiemaGetMACAdr2(char *adrMAC)
{
    int sock; //,sockfd, n, cnt;
    int n;
    char buffer[2048];
    unsigned char *ethhead; //,*iphead

    if ( (sock=socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP)))<0)
    {
        ast_log(__LOG_WARNING,"Erreur creation Socket : %s\n",strerror(errno));
        return -1;
    }

    if ((n = recvfrom(sock,buffer,2048,0,NULL,NULL)) == -1)
    {
        ast_log(__LOG_WARNING,"Erreur recvfrom : %s\n",strerror(errno));
        close(sock);
        return -1;
    }

    ethhead = (unsigned char *)buffer;

    if (ethhead != NULL)
    {
        // iphead = (unsigned char *)(buffer+14); // Skip Ethernet header
        // printf("\n--------------------------------------"
        //    "\nMAC  ""%02x:%02x:%02x:%02x:%02x:%02x\n",ethhead[6],ethhead[7],ethhead[8], ethhead[9],ethhead[10],ethhead[11]);

        sprintf(adrMAC,"%02x:%02x:%02x:%02x:%02x:%02x",ethhead[0],ethhead[1],ethhead[2], ethhead[3],ethhead[4],ethhead[5]);

       // if (*iphead==0x45) // Double check for IPv4 and no options present
       // {
       //     printf("IP destino  (server): %d.%d.%d.%d\n",
       //         iphead[12],iphead[13],
       //         iphead[14],iphead[15]);
       //     printf("IP origen   (CAL30x): %d.%d.%d.%d\n",
       //        iphead[16],iphead[17],
       //         iphead[18],iphead[19]);
       //     printf("Protocolo   (UDP=11): %02x Hex\n",iphead[9]);
       // }
    }
    close(sock);
    return 0;
}*/

// retourne les nom des interfaces reseau disponibles //
// issu de : http://www.koders.com/c/fidC95CE720A0854F30ECE21A275762DCD242205E7E.aspx //
// ifName : recoit la copie des noms d'interfaces reseau trouvees separees par une virgule ','
// sizeIfNameMax : taille du buffer ifName
// retour : le nombre d'interfaces trouvees
int SiemaGetIfName(char *ifName, int sizeIfNameMax)
{
    int fd;
    int i;
    struct ifconf    ifc;
    struct ifreq    ibuf[16],
            ifr,
            *ifrp,
            *ifend;

    memset(ifName, '\0', sizeIfNameMax);

    if ( (fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        ast_log(__LOG_WARNING,"Erreur creation Socket : %s\n",strerror(errno));
        return -1;
    }

    memset(ibuf, 0, sizeof (struct ifreq)*16);
    ifc.ifc_len = (int)sizeof (ibuf);
    ifc.ifc_buf = (caddr_t) ibuf;

    /* gets interfaces list */
    if ( (ioctl(fd, SIOCGIFCONF, (char*)&ifc) == -1) || ((size_t)ifc.ifc_len < sizeof(struct ifreq)))
    {
        ast_log(__LOG_WARNING,"Erreur ioctl : %s\n",strerror(errno));
        close(fd);
        return -1;
    }

    /* ifrp points to buffer and ifend points to buffer's end */
    ifrp = ibuf;
    ifend = (struct ifreq*) ((char*)ibuf + ifc.ifc_len);

    i = 0;
    for (; ifrp < ifend; ifrp++)
    {
        //strlcpy(ifr.ifr_name, c, sizeof(ifr.ifr_name));
        strncpy(ifr.ifr_name,ifrp->ifr_name,sizeof(ifr.ifr_name)-1);
        if ((int)(strlen(ifName)+strlen(ifr.ifr_name)) <= sizeIfNameMax)
        {
            sprintf(ifName,"%s%s,",ifName, ifr.ifr_name);
            i++;
        }
        else break; // buffer insufisant pour recevoir d'autres noms d'interface
    }
    ifName[strlen(ifName) - 1] = '\0';

    close(fd);
    return i;
}


// fin foctions communues //

////////////////
// client TCP //
////////////////
// timeout en s
int ClientConnect(char* strAdress,int nPortNumber, int *m_pSocket, int timeout)
{
    int err = 0;
    char* adrIP = strAdress;
#ifdef ASTERISK
    struct ast_hostent* remoteHost = NULL;
#else // ASTERISK //
    struct hostent* remoteHost = NULL;
#endif // ASTERISK //
    int pSocket;
    fd_set myset;
    long arg;
    struct timeval tv;
    socklen_t lon;
    int valopt;

    *m_pSocket = -1;


    // If the user input is an alpha name for the host, use gethostbyname()
    // If not, get host by addr (assume IPv4)
    if (((strAdress[0] >= 'A') && (strAdress[0] <= 'Z')) || ((strAdress[0] >= 'a') && (strAdress[0] <= 'z')))/* host address is a name */
    {
        //printf("is alpha\n");
        // if hostname terminated with newline '\n', remove and zero-terminate
        if (strAdress[strlen(strAdress)-1] == '\n')
            strAdress[strlen(strAdress)-1] = '\0';

#ifdef ASTERISK
        ast_gethostbyname(strAdress, remoteHost);
#else   // ASTERISK //
        remoteHost = gethostbyname(strAdress);
#endif  // ASTERISK //

        if (remoteHost != NULL)
        {
#ifdef ASTERISK
            adrIP = ast_inet_ntoa (*((struct in_addr*)remoteHost->hp.h_addr_list));
#else  // ASTERISK //
            adrIP = inet_ntoa (*(struct in_addr *)*remoteHost->h_addr_list);
#endif // ASTERISK //
        }
        else return -1;
    }

    SOCKADDR_IN service;
    // initialisation du socket //

    service.sin_addr.s_addr = inet_addr(adrIP);
    service.sin_family = AF_INET;
    service.sin_port = htons(nPortNumber);
    pSocket = socket(AF_INET,SOCK_STREAM,0);
    if (pSocket < 0 )
    {
        ast_log(__LOG_WARNING,"Erreur creation Socket\n");
        err = -1;
        goto end;
    }
    *m_pSocket = pSocket;

    // Set non-blocking
    arg = fcntl(pSocket, F_GETFL, NULL);
    arg |= O_NONBLOCK;
    fcntl(pSocket, F_SETFL, arg);

    // tentative de connexion //
    err = connect(pSocket,(struct sockaddr*)&service,sizeof(service));
    if (err < 0)
    {
     if (errno == EINPROGRESS)
     {
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        FD_ZERO(&myset);
        FD_SET(pSocket, &myset);
        if (select(pSocket+1, NULL, &myset, NULL, &tv) > 0)
        {
           lon = sizeof(int);
           getsockopt(pSocket, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon);
           if (valopt)
           {
                ast_verbose("Serveur indisponible\n"); //- %d - %s\n",valopt, strerror(valopt));
                err = -1;
                goto end;
           }
           else err = 0; // tout est OK
        }
        else
        {
           ast_verbose("Timeout de connexion depasse: %d - %s\n",errno, strerror(errno));
           err = -2;
           goto end;
        }
     }
     else
     {
        ast_verbose("Erreur Connect %d - %s\n",errno, strerror(errno));
        err = -1;
        goto end;
     }
    }
    else err = 0;

end:
    if (err != 0) //ferme le socket en cas d'erreur //
    {
        SiemaSocketClose(*m_pSocket);
        *m_pSocket = -1;
    }

    // Set to blocking mode again...
    if (pSocket >= 0)
    {
        arg = fcntl(pSocket, F_GETFL, NULL);
        arg &= (~O_NONBLOCK);
        fcntl(pSocket, F_SETFL, arg);
    }


    return err;
}




// fin client TCP //


/////////////////
// Serveur TCP //
/////////////////
#ifndef ASTERISK
static int fin_serveurTCP = 0;
static int getArgs_serveurTCP;
static pthread_t master = SIEMA_PTHREADT_NULL;
struct ServeurTCPfunc_args
{
    int tcpfd;
    t_CallbackConnexionTCP *callbackConnexion;
};



int siema_wait_for_input(int fd, int ms)
{
    struct pollfd pfd[1];
    memset(pfd, 0, sizeof(pfd));
    pfd[0].fd = fd;
    pfd[0].events = POLLIN|POLLPRI;
    return poll(pfd, 1, ms);
}


static void *ServeurTCPfunc(void *args)
{
    int fd;
    struct sockaddr_in sin;
    socklen_t sinlen;
    struct siema_tcp_server_instance *ser;
    pthread_t launched;
    pthread_attr_t attr;
    char ipHoteDist[512];
    int res;
    struct ServeurTCPfunc_args *params =(struct ServeurTCPfunc_args*) args;
    int tcpfd = params->tcpfd;
    t_CallbackConnexionTCP *callbackConnexion = params->callbackConnexion;


    // libere le process appelant qui attend la lecture des arguments //
    // TODO : MUTEX!!!
    getArgs_serveurTCP = 1;

    for (;;)
    {
        int flags;

        if (fin_serveurTCP == 1) break;

        usleep(10000); // petite pause de 10ms

        res = siema_wait_for_input(tcpfd, 10000); // timeout de 10s
        if (res == 0) continue; // timeout atteint
        if (res < 0) {ast_log(__LOG_WARNING,"Erreur Select (Poll) : fin du serveur TCP");break;}
        sinlen = sizeof(sin);
        fd = accept(tcpfd, (struct sockaddr *)&sin, &sinlen);
        if (fd < 0)
        {
            if ((errno != EAGAIN) && (errno != EINTR))
                ast_log(__LOG_WARNING,"Accept failed: %s\n", strerror(errno));
            continue;
        }
        ser = (struct siema_tcp_server_instance*) calloc(1, sizeof(*ser));
        if (!ser)
        {
            ast_log(__LOG_WARNING,"No memory for new session: %s\n", strerror(errno));
            close(fd);
            continue;
        }
        flags = fcntl(fd, F_GETFL);
        fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
        ser->fd = fd;
        memcpy(&ser->requestor, &sin, sizeof(ser->requestor));

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        // SIEMA : FDS : recupere le nom du pair distant //
        ipHoteDist[0] = '\0';
        sprintf(ipHoteDist,"%s",inet_ntoa(sin.sin_addr));
        //if (remote_s > 0) getpeername(remote_s, &nomHoteDist, sizeof(nomHoteDist));
        //ast_log(__LOG_WARNING,"SIEMA : FDS : DEBUG : nom hote distant : %s\n",ipHoteDist);
        sprintf(ser->ipHoteDist,"%s",ipHoteDist);

        if (siema_pthread_create_background(&launched, &attr, callbackConnexion, ser))
        {
            ast_log(__LOG_WARNING,"Unable to launch helper thread: %s\n", strerror(errno));
            if (ser->fd > -1) closesocket(ser->fd);
            free(ser);
        }
        pthread_attr_destroy(&attr);
    }

    if (tcpfd >= 0)
    {
        // ferme le socket //
        close(tcpfd);
    }

    fin_serveurTCP = 0;
   ast_log(__LOG_WARNING,"Sortie ServeurTCPfunc : %s\n", strerror(errno));
    return NULL;
}


// prend en argument le port d'ecoute et la callback qui gère chaque connexion indépendamment //
// dans la callback on peut lire sur le socket, ecrire, ....
int DemarrerServeurTCP(int port, t_CallbackConnexionTCP *callbackConnexion, int inThread)
{
    int flags;
    int x = 1;
    int tcpfd = -1;

    struct sockaddr_in sin;

    fin_serveurTCP = 0;

    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
       sin.sin_addr.s_addr = INADDR_ANY;

    /* Shutdown a running server if there is one */
    if (master != SIEMA_PTHREADT_NULL)
    {
        pthread_cancel(master);
        pthread_kill(master, SIGURG);
        pthread_join(master, NULL);
    }


    /* If there's no new server, stop here */
    if (!sin.sin_family)
        return -1;


    tcpfd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpfd < 0)
    {
        ast_log(__LOG_WARNING,"Unable to allocate socket: %s\n", strerror(errno));
        return -1;
    }

    setsockopt(tcpfd, SOL_SOCKET, SO_REUSEADDR, &x, sizeof(x));
    if (bind(tcpfd, (struct sockaddr *)&sin, sizeof(sin)))
    {
        ast_log(__LOG_WARNING,"Unable to bind tcp server to %s:%d: %s\n",
            inet_ntoa(sin.sin_addr), ntohs(sin.sin_port),
            strerror(errno));
        close(tcpfd);
        tcpfd = -1;
        return -1;
    }
    if (listen(tcpfd, 10))
    {
        ast_log(__LOG_WARNING,"Unable to listen!\n");
        close(tcpfd);
        tcpfd = -1;
        return -1;
    }
    flags = fcntl(tcpfd, F_GETFL);
    fcntl(tcpfd, F_SETFL, flags | O_NONBLOCK);

    ast_verbose("Demarrage du serveur TCP sur le port %d\n",port);

    if (inThread == 0)
    {
        struct ServeurTCPfunc_args args;
        args.tcpfd = tcpfd;
        args.callbackConnexion = callbackConnexion;
        ServeurTCPfunc(&args); // lance le seveur

    }
    else
    {
        struct ServeurTCPfunc_args args;
        args.tcpfd = tcpfd;
        args.callbackConnexion = callbackConnexion;

        pthread_t launched;
        pthread_attr_t attr;

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        getArgs_serveurTCP = 0;
        if (siema_pthread_create_background(&launched, &attr, ServeurTCPfunc, &args))
        {
            ast_log(__LOG_WARNING,"Unable to launch helper thread: %s\n", strerror(errno));
        }
        pthread_attr_destroy(&attr);

        // TODO : mettre un mutex sur getArgs_serveurTCP
        while (getArgs_serveurTCP == 0) sleep(1); // attent que les argument de 'args' aient ete recopies par le thread
    }
    return 0;
}


void ArreterServeurTCP()
{
    getArgs_serveurTCP = 1;
    fin_serveurTCP = 1;
    while (fin_serveurTCP == 1) sleep(1); // attent qla fin du serveur
}
#endif // !ASTERISK //
// fin serveur TCP //

//////////////////////////
// gestion des messages //
//////////////////////////
t_message_3num_nchaine *SiemaNewMessage(void);
// cree un message vide //
// ATTENTION : le message retourne doit etre libere par appel a SiemaDeleteMessage()!
t_message_3num_nchaine *SiemaNewMessage(void)
{
     t_message_3num_nchaine *msg;

     msg = (t_message_3num_nchaine *) malloc(sizeof( t_message_3num_nchaine));
     memset(msg,0,sizeof(t_message_3num_nchaine));
     msg->longueur = 22; // ATTENTION : a cause des alignement 32bits, sizeof(t_message_3num_nchaine) - sizeof(t_chaine *) ne vaut pas 22 !!;
     msg->type = TYPE_3NUM_NCHAINE;
//     ast_verbose("malloc  add = 0x%lX\n",msg);
     return msg;
}

// supprime un message et toutes ses chaines associees //
int SiemaDeleteMessage(t_message_3num_nchaine *msg)
{
    int n;
    t_chaine *ch_tmp, *ch_tmp2;
//     ast_verbose("dealloc  add = 0x%lX\n",msg);

    if (msg == NULL) return 0;
    ch_tmp= msg->liste_chaine;
    n = 0;
    while ((ch_tmp != NULL) && (n < msg->nb_chaines))
    {
        n++;
        free(ch_tmp->chaine);
        ch_tmp2 = ch_tmp->next;
        free(ch_tmp);
        ch_tmp = ch_tmp2;
        }
        free(msg);

    return 0;
}

// ajoute une chaine au message (et met a jour la taille du message) //
int SiemaAjoutChaineMessage(t_message_3num_nchaine *msg, const char* chaine, short longueur_chaine)
{
    t_chaine *ch_tmp;
    int i;

    if ((msg == NULL) || (chaine == NULL)) return -1;

    if (msg->liste_chaine == NULL) // cas du premier message de la liste //
    {
        msg->liste_chaine = (t_chaine*) malloc(sizeof(t_chaine));
        ch_tmp = msg->liste_chaine;
    }
    else // ajout d'une chaine en queue de liste
    {
        ch_tmp= msg->liste_chaine;
        while (ch_tmp->next != NULL)
            {
              ch_tmp = ch_tmp->next;
            }
            ch_tmp->next = (t_chaine*) malloc(sizeof(t_chaine));
            ch_tmp = ch_tmp->next;
        }

        if (ch_tmp == NULL) return -1;

        ch_tmp->longueur = longueur_chaine;
        ch_tmp->chaine = (char*) malloc(longueur_chaine+1);
        ch_tmp->next = NULL;
        // NB : pour plus de securite pour la manipulation des chaines, ajoute un \0 à la fin systematiquement //
        ch_tmp->chaine[longueur_chaine] = '\0';

        for (i = 0; i < ch_tmp->longueur; i++)
        ch_tmp->chaine[i] = chaine[i];

        msg->longueur = msg->longueur + (sizeof(ch_tmp->longueur) + ch_tmp->longueur);
      msg->nb_chaines++;

      return 0;
}

//#ifdef ASTERISK
char *SiemaMSGtoStr(int numMSG) // 2009/03/02 : retourne le nom d'un message en fonction de son numero
{
     switch (numMSG)
    {
        case MSG_APPEL_ENTRANT  : return "MSG_APPEL_ENTRANT";
        case MSG_APPELER_REL : return "MSG_APPELER_REL";
        case MSG_FIN_REL : return "MSG_FIN_REL";
        case MSG_ACK : return "MSG_ACK";
        case MSG_ECHEC : return "MSG_ECHEC";
        case MSG_NC_CORE_PING : return "MSG_NC_CORE_PING";
        case MSG_NC_CORE_ACK : return "MSG_NC_CORE_ACK";
        case MSG_NC_CORE_PILOTER_SORTIE : return "MSG_NC_CORE_PILOTER_SORTIE";
        case MSG_MASK_ENTREES : return "MSG_MASK_ENTREES";
        case MSG_ASK_ETAT_ENTREE : return "MSG_ASK_ETAT_ENTREE";
        case MSG_ETAT_ENTREES : return "MSG_ETAT_ENTREES";
        case MSG_APPEL_INTER_PUPITRE : return "MSG_APPEL_INTER_PUPITRE";
        case MSG_APPEL_SORTANT : return "MSG_APPEL_SORTANT";
        case MSG_DIAL_EN_COURS : return "MSG_DIAL_EN_COURS";
        case MSG_REDIRECT_REL : return "MSG_REDIRECT_REL";
        case MSG_TRANSFERT_REL : return "MSG_TRANSFERT_REL";
        case MSG_RENVOI_SORTANT_GARE : return "MSG_RENVOI_SORTANT_GARE";
        case MSG_RENVOI_ENTRANT_GARE : return "MSG_RENVOI_ENTRANT_GARE";
//        case MSG_OUVERTURE_GARE : return "MSG_OUVERTURE_GARE";
//        case MSG_FERMETURE_GARE : return "MSG_FERMETURE_GARE";
//        case MSG_GARE_FERMEE : return "MSG_GARE_FERMEE";
//        case MSG_GARE_OUVERTE : return "MSG_GARE_OUVERTE";
        case MSG_APPEL_REL_EXTERNE: return "MSG_APPEL_REL_EXTERNE";
//        case MSG_REQ_ETAT_GARE : return "MSG_REQ_ETAT_GARE";
        case MSG_REGUL_CONF_SORTANT: return "MSG_REGUL_CONF_SORTANT";
        case MSG_REGUL_CONF_ENTRANT: return "MSG_REGUL_CONF_ENTRANT";
        case MSG_REGUL_CONF_PS_ENTRANT: return "MSG_REGUL_CONF_PS_ENTRANT";
        case MSG_APPEL_ACCEPTE: return "MSG_APPEL_ACCEPTE";
        case MSG_NC_CORE_NACK:  return "MSG_NC_CORE_NACK";
        case MSG_NC_CORE_WAIT:  return "MSG_NC_CORE_WAIT";
        case MSG_RECEPTION_5T : return "MSG_RECEPTION_5T";
        case MSG_RECEPTION_WE : return "MSG_RECEPTION_WE";
        case MSG_EMETTRE_5T:    return "MSG_EMETTRE_5T";
        case MSG_REC_PTO_5T:    return "MSG_REC_PTO_5T";
        case MSG_EMETTRE_WE:    return "MSG_EMETTRE_WE";
        case MSG_RECEPTION_GSMR: return "MSG_RECEPTION_GSMR";
        case MSG_NC_CORE_STOP_WATCHDOG : return "MSG_NC_CORE_STOP_WATCHDOG";
        case MSG_GSMR_GET_NIVEAU: return "MSG_GSMR_GET_NIVEAU";
        case MSG_GSMR_MISE_EN_GARDE : return "MSG_GSMR_MISE_EN_GARDE";
        case MSG_GSMR_GET_APPELS_EN_GARDE : return "MSG_GSMR_GET_APPELS_EN_GARDE";
        case MSG_GSMR_REPRISE_GARDE : return "MSG_GSMR_REPRISE_GARDE";
        case MSG_GSMR_SORTIE_GARDE : return "MSG_GSMR_SORTIE_GARDE";
        case MSG_GSMR_IDENT_APPEL_SORTANT : return "MSG_GSMR_IDENT_APPEL_SORTANT";
        case MSG_GSMR_MET_EN_CONFERENCE: return "MSG_GSMR_MET_EN_CONFERENCE";
        case MSG_GSMR_GET_APPELS_EN_CONF: return "MSG_GSMR_GET_APPELS_EN_CONF";
        case MSG_GSMR_EJECT_CONFERENCE: return "MSG_GSMR_EJECT_CONFERENCE";
        case MSG_AVIS_PRISE_EN_TIER : return "MSG_AVIS_PRISE_EN_TIER";        // Signal la prise en tier
        case MSG_PRISE_EN_ECOUTE : return "MSG_PRISE_EN_ECOUTE";
        case MSG_AVIS_PRISE_EN_ECOUTE : return "MSG_AVIS_PRISE_EN_ECOUTE";        // Signal la mise en ecoute
        case MSG_REPRISE_GARDE_PUPITRE : return "MSG_REPRISE_GARDE_PUPITRE";     // Message pour la reprise de garde sur un pupitre
        case MSG_PRISE_EN_TIER : return "MSG_PRISE_EN_TIER";
        case MSG_REP_APPEL_ENTRANT : return "MSG_REP_APPEL_ENTRANT";          // sur reponse d'un pupitre
        case MSG_REP_APPEL_SORTANT : return "MSG_REP_APPEL_SORTANT";          // sur decroché dans canal ZAP appelé
        case MSG_NC_CORE_START_WATCHDOG : return "MSG_NC_CORE_START_WATCHDOG";
        case MSG_FEED_WATCHDOG : return "MSG_FEED_WATCHDOG";
        case MSG_READ_GA_STATUS : return "MSG_READ_GA_STATUS";              // lecture du satus des gestionaires d'acecs
        case MSG_GA_STATUS : return "MSG_GA_STATUS";                        // reponse ou lecture spontanée de status des gestionaires d'acces
       default : sprintf(msginconnu,"%d",numMSG); return msginconnu;

    }

    return "";
}

void SiemaAfficherMessage(t_message_3num_nchaine *msg)
{
    int i,j;
    t_chaine *ch_tmp;

    if (msg == NULL) return;

    ast_verbose(VERBOSE_PREFIX_2 "MSG: (time=%d)\n",(int)time(NULL));
    ast_verbose(VERBOSE_PREFIX_3 "longueur=%d * Type=%d * Sous-type=", msg->longueur, msg->type);
    ast_verbose(SiemaMSGtoStr(msg->sous_type));
    ast_verbose("\n");
    ast_verbose(VERBOSE_PREFIX_3 "num1=%d num2=%d num3=%d\n", msg->num1, msg->num2, msg->num3);
    ast_verbose(VERBOSE_PREFIX_3 "nb_chaines=%d\n", msg->nb_chaines);
    ch_tmp = msg->liste_chaine;
    for (i = 0; i < msg->nb_chaines; i++)
    {
        ast_verbose(VERBOSE_PREFIX_3 "  <%d>",ch_tmp->longueur);
        for (j = 0; j < ch_tmp->longueur; j++)
            ast_verbose("%c", ch_tmp->chaine[j]);
        ast_verbose("\n");
        ch_tmp = ch_tmp->next;
    }
}


// Serialise un message pour pouvoir le transmettre sur IP
// retourne la taille de la chaine cree dans tmp
int SiemaSerialiserMessage(t_message_3num_nchaine *msg, unsigned char *chaine, int tailleMaxChaine)
{
    if ((chaine == NULL) || (msg == NULL)) return -1;

    t_int_char *ic;
    t_short_char *iw;
    int index, i, n;
    t_chaine *ch_tmp;

    index = 0;
    ic = (t_int_char*) &chaine[index];
    ic->i = htonl(msg->longueur);
    index += sizeof(msg->longueur);
    iw = (t_short_char*)&chaine[index];
    iw->w = htons(msg->type);
    index += sizeof(msg->type);
    iw = (t_short_char*)&chaine[index];
    iw->w = htons(msg->sous_type);
    index += sizeof(msg->sous_type);
    ic = (t_int_char*)&chaine[index];
    ic->i = htonl(msg->num1);
    index += sizeof(msg->num1);
    ic = (t_int_char*)&chaine[index];
    ic->i = htonl(msg->num2);
    index += sizeof(msg->num2);
    ic = (t_int_char*)&chaine[index];
    ic->i = htonl(msg->num3);
    index += sizeof(msg->num3);
    iw = (t_short_char*)&chaine[index];
    iw->w = htons(msg->nb_chaines);
    index += sizeof(msg->nb_chaines);

    ch_tmp= msg->liste_chaine;

    n = 0;
    while ((ch_tmp != NULL) && (n < msg->nb_chaines))
    {
        n++;

        if ((int)(index + ch_tmp->longueur + sizeof(ch_tmp->longueur)) > tailleMaxChaine) return -1;

        iw = (t_short_char*)&chaine[index];
        iw->w = htons(ch_tmp->longueur);
        index += sizeof(ch_tmp->longueur);

        for (i = 0; i < ch_tmp->longueur; i++)
        {
            chaine[index++] = ch_tmp->chaine[i];
        }
        ch_tmp = ch_tmp->next;
    }

    return index;
}

// Deserialise un message et retourne la structure completee du message //
// ATTENTION : le message retourne doit etre libere par appelle a SiemaDeleteMessage()!
t_message_3num_nchaine *SiemaDeserialiserMessage(unsigned char *chaine, int tailleChaine)
{
    t_int_char *ic;
    t_short_char *iw;
    int index, n;
    t_message_3num_nchaine *msg;
    short nb_chaines_tmp;
    msg = NULL;

    if (chaine == NULL) return NULL;
    if (tailleChaine <= 0) return NULL;
    
    msg = SiemaNewMessage();

    index = 0;
    ic = (t_int_char*) &chaine[index];
    // on ne touche pas à msg->longueur, il calculer à la construction du msg... msg->longueur = ntohl(ic->i);
    index += sizeof(msg->longueur);
    iw = (t_short_char*)&chaine[index];
    msg->type = htons(iw->w);
    index += sizeof(msg->type);
    iw = (t_short_char*)&chaine[index];
    msg->sous_type = htons(iw->w);
    index += sizeof(msg->sous_type);
    ic = (t_int_char*)&chaine[index];
    msg->num1 = htonl(ic->i);
    index += sizeof(msg->num1);
    ic = (t_int_char*)&chaine[index];
    msg->num2 = htonl(ic->i);
    index += sizeof(msg->num2);
    ic = (t_int_char*)&chaine[index];
    msg->num3= htonl(ic->i);
    index += sizeof(msg->num3);
    iw = (t_short_char*)&chaine[index];
    nb_chaines_tmp = htons(iw->w);
    index += sizeof(msg->nb_chaines);
    n = 0;
    while (n < nb_chaines_tmp)
    {
        short sh_tmp = 0;
        iw = (t_short_char*)&chaine[index];
        sh_tmp = htons(iw->w );
        index += sizeof(sh_tmp);
        SiemaAjoutChaineMessage(msg, (char*)&chaine[index], sh_tmp);
        index += sh_tmp;
        n++;
    }
    return msg;
}

// fin gestion des messages //


/////////////////////////
// gestion des threads //
/////////////////////////
#ifndef ASTERISK
int siema_pthread_create_stack(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *),
                 void *data, size_t stacksize, const char *file, const char *caller,
                 int line, const char *start_fn)
{
    if (file == NULL)
        return -1;
    if (caller == NULL)
        return -1;
    if (line <= 0)
        return -1;
    if (start_fn == NULL)
        return -1;
        
    if (!attr) {
        attr = (pthread_attr_t*) malloc(sizeof(*attr));
        pthread_attr_init(attr);
    }

#ifdef __linux__
    /* On Linux, pthread_attr_init() defaults to PTHREAD_EXPLICIT_SCHED,
       which is kind of useless. Change this here to
       PTHREAD_INHERIT_SCHED; that way the -p option to set realtime
       priority will propagate down to new threads by default.
       This does mean that callers cannot set a different priority using
       PTHREAD_EXPLICIT_SCHED in the attr argument; instead they must set
       the priority afterwards with pthread_setschedparam(). */
    if ((errno = pthread_attr_setinheritsched(attr, PTHREAD_INHERIT_SCHED)))
        ast_log(__LOG_WARNING,"pthread_attr_setinheritsched: %s\n", strerror(errno));
#endif

    if (!stacksize)
        stacksize = STACK_SIZE;

     if ((errno = pthread_attr_setstacksize(attr, stacksize ? stacksize : STACK_SIZE)))
        ast_log(__LOG_WARNING,"pthread_attr_setstacksize: %s\n", strerror(errno));

    return pthread_create(thread, attr, start_routine, data); /* We're in siema_pthread_create, so it's okay */
}
#endif // !ASTERISK //

// fin threads //





// TODO : ?????
// SIEMA : FDS : ajout d'une fonction de PING pour savoir quand la liaison avec l'IAE est perdue //
/*int chknet(char * ip)
{
    struct sockaddr_in name;
    //struct hostent * hent;
    int sock;
    int retour = 0;
    int timeout = 1000;
    int rc;

    // creation de la socket en icmp //
    if(!(sock = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP))) return (-1);

      rc = setsockopt(sockraw, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout));
      if (rc < 0)
      {
            //perror("ping: recv timeout");
            return -1;
      }
     rc = setsockopt(sockraw, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeout, sizeof(timeout));
      if(rc < 0)
      {
            //perror("ping: send timeout");
            return 1;
      }

    memset(&name,0,sizeof(struct sockaddr_in));
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = inet_addr(ip);
    if(connect(sock,(struct sockaddr *) &name,sizeof(struct sockaddr))==0) retour = 1;
    else retour = 0;
    shutdown(sock,SHUT_RDWR);
    close(sock);
    printf("SIEMA : FDS : chknet ping %s=%d\n",ip,retour);
    return(retour);
}*/



// récupère l'etat du CPU //

// provient de top.c du projet busybox
// gestion processeur SMP supprimée (mutliprocesseur)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct jiffy_counts_t {
    unsigned long long usr,nic,sys,idle,iowait,irq,softirq,steal;
    unsigned long long total;
    unsigned long long busy;
} jiffy_counts_t;

static jiffy_counts_t cur_jif, prev_jif;
static int  threadCPUStatelaunched = 0;

/* NOINLINE so that complier doesn't unfold the call
 * causing multiple copies of the arithmatic instrns
 */
static int read_cpu_jiffy()
{
    char linebuf[4096];
    jiffy_counts_t *p_jif;

    static const char fmt[] = "cpu %llu %llu %llu %llu %llu %llu %llu %llu";

    int ret;

    FILE* fp = fopen("/proc/stat","r+");
    if (fp != NULL)
    {
        prev_jif = cur_jif;
        p_jif = &cur_jif;

        if (!fgets(linebuf, 4096, fp) || linebuf[0] != 'c' /* not "cpu" */)
            return 0;
        ret = sscanf(linebuf, fmt,
                &p_jif->usr, &p_jif->nic, &p_jif->sys, &p_jif->idle,
                &p_jif->iowait, &p_jif->irq, &p_jif->softirq,
                &p_jif->steal);
        if (ret > 4) {
            p_jif->total = p_jif->usr + p_jif->nic + p_jif->sys + p_jif->idle
                + p_jif->iowait + p_jif->irq + p_jif->softirq + p_jif->steal;
            /* procps 2.x does not count iowait as busy time */
            p_jif->busy = p_jif->total - p_jif->idle - p_jif->iowait;
        }
        else ast_log(__LOG_WARNING, "CPU STATE : can't read /proc/stat");
        fclose(fp);
    }

    return ret;
}

void *threadCPUState(void *arg)
{
//    int i = 0;
    if (arg != NULL)
        return NULL;
    while (1) {
        read_cpu_jiffy();
        sleep(1);
 /*       if (i++ > 5)
        {
            #define  CALC_TOT_DIFF  ((unsigned)(p_jif->total - p_prev_jif->total) ? : 1)


#define CALC_STAT(xxx) unsigned xxx = 100 * (unsigned)(p_jif->xxx - p_prev_jif->xxx) / total_diff
#define SHOW_STAT(xxx) xxx
#define FMT "%4u%% "

    {
        unsigned total_diff;
    jiffy_counts_t *p_jif, *p_prev_jif;;

        p_jif = &cur_jif;
        p_prev_jif = &prev_jif;
        total_diff = CALC_TOT_DIFF;


        { // Need a block: CALC_STAT are declarations //
            CALC_STAT(idle);
            ast_log(__LOG_WARNING,"CPU :"FMT"\n",idle);
            i = 0;
        }
    }
#undef SHOW_STAT
#undef CALC_STAT
#undef FMT

        }*/
    }
    return NULL;
}

//
// Démarrage le thread d'analyse d'état CPU (indispensable car il y a un calcul différentiel entre l'état précédent et l'actuel)
//
int startThreadCPUState()
{
    pthread_t launched;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (siema_pthread_create_background(&launched, &attr, threadCPUState,  NULL))
    {
       ast_log(__LOG_WARNING,"Unable to launch helper thread 'threadCPUState': %s\n", strerror(errno));
    }
    else threadCPUStatelaunched = 1;
    pthread_attr_destroy(&attr);

    return 0;
}

// affiche les informations de consommation temps CPU
//static void display_cpus()
//{
//    /*
//     * xxx% = (cur_jif.xxx - prev_jif.xxx) / (cur_jif.total - prev_jif.total) * 100%
//     */
//    unsigned total_diff;
//    jiffy_counts_t *p_jif, *p_prev_jif;;
//
//    // au premier appel, lance le thread qui analyse ne permanance l'etat CPU //
//    if (!threadCPUStatelaunched)
//    {
//        startThreadCPUState();
//    }
//
//    /* using (unsigned) casts to make operations cheaper */
//#define  CALC_TOT_DIFF  ((unsigned)(p_jif->total - p_prev_jif->total) ? : 1)
//
//
//#define CALC_STAT(xxx) unsigned xxx = 100 * (unsigned)(p_jif->xxx - p_prev_jif->xxx) / total_diff
//#define SHOW_STAT(xxx) xxx
//#define FMT "%4u%% "
//
//    {
//        p_jif = &cur_jif;
//        p_prev_jif = &prev_jif;
//        total_diff = CALC_TOT_DIFF;
//
//
//        { /* Need a block: CALC_STAT are declarations */
//            CALC_STAT(usr);
//            CALC_STAT(sys);
//            CALC_STAT(nic);
//            CALC_STAT(idle);
//            CALC_STAT(iowait);
//            CALC_STAT(irq);
//            CALC_STAT(softirq);
//            /*CALC_STAT(steal);*/
//
//            //snprintf(scrbuf, 80,
//            ast_log(__LOG_WARNING,"CPU:"FMT"usr"FMT"sys"FMT"nic"FMT"idle"FMT"io"FMT"irq"FMT"sirq",
//                SHOW_STAT(usr), SHOW_STAT(sys), SHOW_STAT(nic), SHOW_STAT(idle),
//                SHOW_STAT(iowait), SHOW_STAT(irq), SHOW_STAT(softirq)
//            );
//
//        }
//    }
//#undef SHOW_STAT
//#undef CALC_STAT
//#undef FMT
//}

// retourne le pourcentage de idle du cpu
// => la seule fonction utile pour l'utilisateur
int getIdleCPU()
{
    unsigned total_diff;
    jiffy_counts_t *p_jif, *p_prev_jif;

    // au premier appel, lance le thread qui analyse ne permanance l'etat CPU //
    if (!threadCPUStatelaunched)
    {
        startThreadCPUState();
    }

    /* using (unsigned) casts to make operations cheaper */
#define  CALC_TOT_DIFF  ((unsigned)(p_jif->total - p_prev_jif->total) ? (unsigned)(p_jif->total - p_prev_jif->total) : 1)


#define CALC_STAT(xxx) unsigned xxx = 100 * (unsigned)(p_jif->xxx - p_prev_jif->xxx) / total_diff
#define SHOW_STAT(xxx) xxx
#define FMT "%4u%% "

    {
        p_jif = &cur_jif;
        p_prev_jif = &prev_jif;
        total_diff = CALC_TOT_DIFF;


        { /* Need a block: CALC_STAT are declarations */
            CALC_STAT(idle);
            return idle;
        }
    }
#undef SHOW_STAT
#undef CALC_STAT
#undef FMT

}
