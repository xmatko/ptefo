/*!
 * \file process_audio.c
 * \brief Traitement et transfert du flux audio
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 03/06/2014
 *
 * Test d'un serveur UDP
 *
 * S:\\be\\Sncf\\PTE FO IFOTEC\\E_Développement matériel\\Spécifications\\Diffussion\n
 * S:\\be\\Sncf\\PTE FO IFOTEC\\5tons.pdf\n
 * S:\\be\\Sncf\\PTE FO IFOTEC\\D_Développement logiciel\\Programmation\\Prog Poste De Tete\n
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "zaptel/trunk/zaptel.h"
#include "zaptel/trunk/pto4M.h"

#include "common.h"
#include "tlog.h"
#include "socks_mgt.h"
#include "code5t.h"
#include "code5t_proc.h"
#include "heartbeat.h"
#include "config.h"
#include "process_audio.h"

static int fd          = -1;

typedef struct pto4M_audio sPTO4M_Audio;

static char rcv_msg[BUF_SIZE * sizeof (char)] = {0};
static char snd_msg[BUF_SIZE * sizeof (char)] = {0};
sPTO4M_Audio d_samples_rd_space;
sPTO4M_Audio d_samples_wr_space;
sPTO4M_Audio * d_samples_rd = &d_samples_rd_space;
sPTO4M_Audio * d_samples_wr = &d_samples_wr_space;
static short d_tmp[ZT_CHUNKSIZE] = {0};
static unsigned int rcv_len = 0;
static unsigned int snd_len = 0;

typedef union short_char
{
    short w;
    char c[sizeof (short)];
} t_short_char;

//static char (* zline)[MAX_STR];

//static int traitement_audio_loopback(sMain * pMain);
int traitement_audio(sMain * pMain);
//static void timer_expire(sigval_t arg);

/*!
 * \fn static void cleanup_handler_UDPServer_Audio(void * arg)
 * \brief Cleanup handler pour le thread F_Th_UDPServer_Audio
 * \param arg: pointeur sur structure sThArgProcAudio
 */
static void cleanup_handler_Process_Audio(void * arg)
{
    SLOGT(LOG_INFO, "F_Th_Process_Audio clean-up handler\n");
    if (arg == NULL) arg = NULL;     /* Only to avoid warning [-Wunused-parameter] */
    int i = 0;
    sAudioStatus * pAudioSt = (sAudioStatus *)arg;
    
    /*
    free(rcv_msg);
    free(snd_msg);
    free(d_samples_rd);
    free(d_samples_wr);
    free(d_tmp);
    */
    
    for (i = 0; i < pAudioSt->nb_circuits; i++) {
        F_ProcAudio_Set_ActiveAudio(pAudioSt, i + 1, CC_AUDIO_STATUS_INACTIVE);

        /*
        if (close(pAudioSt->zfd[i]) < 0) {
            SLOGT(LOG_ERR, "Erreur de fermeture %s (%s)\n", pAudioSt->zline[i], strerror(errno));
        } else {
            SLOGT(LOG_NOTICE, "cleanup_handler_Process_Audio: %s closed\n", pAudioSt->zline[i]);
        }
        */
    }
    /* Close /dev/pto_sample */
    close(fd);

}

int F_ProcAudio_Init_ActiveAudio(sAudioStatus ** pAudioSt, int nb_circuits)
{
    int i = 0;
    *pAudioSt = (sAudioStatus *) malloc(sizeof (sAudioStatus));
    memset(*pAudioSt, 0, sizeof (sAudioStatus));
    pthread_mutex_init(&((*pAudioSt)->mutex_activeaudio), NULL);
    (*pAudioSt)->zline = NULL;
    (*pAudioSt)->zfd = NULL;
    /*
    (*pAudioSt)->zfd = (int *)malloc(NUM_CARDS * sizeof (int));
    memset((*pAudioSt)->zfd, -1, NUM_CARDS * sizeof (int));
    
    (*pAudioSt)->zline = (char **) malloc(NUM_CARDS * sizeof (char *));
    memset((*pAudioSt)->zline, 0, NUM_CARDS * sizeof (char *));
    for (i = 0; i < NUM_CARDS; i++) {
        (*pAudioSt)->zline[i] = (char *) malloc(512 * sizeof (char));
        memset((*pAudioSt)->zline[i], 0, 512 * sizeof (char));
    }
    */

    if (nb_circuits > 0) {
        (*pAudioSt)->nb_circuits = nb_circuits;
        (*pAudioSt)->pActiveAudio = (int *)malloc(nb_circuits * sizeof (int));
        
        /*
        for (i = 0; i < nb_circuits; i++) {
            F_ProcAudio_Set_ActiveAudio(*pAudioSt, i+1, CC_AUDIO_STATUS_UNKNOWN);
            sprintf((*pAudioSt)->zline[i], "/dev/zap/%d", ZAP_CHAN_BASE + i);
            
            (*pAudioSt)->zfd[i] = open((*pAudioSt)->zline[i], O_RDWR | O_NONBLOCK);
            if ((*pAudioSt)->zfd[i] <= 0) {
                SLOGT(LOG_CRIT, "Erreur d'ouverture %s (%s)\n", (*pAudioSt)->zline[i], strerror(errno));
            }
            
            SLOGT(LOG_NOTICE, "F_ProcAudio_Init_ActiveAudio: %s opened\n", (*pAudioSt)->zline[i]);
        }
        */
    }
    return EXIT_SUCCESS;
}

int F_ProcAudio_Set_ActiveAudio(sAudioStatus * pAudioSt, int circuit, int val)
{
    if (pAudioSt != NULL) {
        pthread_mutex_lock(&(pAudioSt->mutex_activeaudio));
        if (val != pAudioSt->pActiveAudio[circuit - 1]) {
            pAudioSt->pActiveAudio[circuit - 1] = val;
            if (val == CC_AUDIO_STATUS_ACTIVE) {
                SLOGT(LOG_NOTICE, "F_ProcAudio_Set_ActiveAudio: Activation de l'audio sur le circuit %d\n", circuit);
                //SLOGT(LOG_DEBUG, "Device %s\n", pAudioSt->zline[circuit - 1]);
            }
            if (val == CC_AUDIO_STATUS_INACTIVE) {
                SLOGT(LOG_NOTICE, "F_ProcAudio_Set_ActiveAudio: Désactivation de l'audio sur le circuit %d\n", circuit);
                
                /*
                if (ioctl(pAudioSt->zfd[circuit - 1], ZT_SIEMA_TD_HANG_UP, NULL) < 0) {
                    SLOGT(LOG_ERR, "F_ProcAudio_Set_ActiveAudio: Erreur ioctl ZT_SIEMA_TD_HANG_UP sur circuit %d: %s (%d)\n", circuit, strerror(errno), errno);
                } else {
                    SLOGT(LOG_INFO, "F_ProcAudio_Set_ActiveAudio: ioctl ZT_SIEMA_TD_HANG_UP sur circuit %d OK\n", circuit);
                }
                */
                if (ioctl(fd, PTO_HANG_UP, circuit + 100) < 0) {
                    SLOGT(LOG_ERR, "F_ProcAudio_Set_ActiveAudio: Erreur ioctl PTO_HANG_UP sur circuit %d: %s (%d)\n", circuit, strerror(errno), errno);
                } else {
                    SLOGT(LOG_INFO, "F_ProcAudio_Set_ActiveAudio: ioctl PTO_HANG_UP sur circuit %d OK\n", circuit);
                }
            }
        }
        pthread_mutex_unlock(&(pAudioSt->mutex_activeaudio));
    }
    else {
        SLOGT(LOG_ERR, "F_ProcAudio_Set_ActiveAudio: ERREUR Pointeur non alloué !!!\n");
    }
    return EXIT_SUCCESS;
}

int F_ProcAudio_Get_ActiveAudio(sAudioStatus * pAudioSt, int circuit)
{
    int is_active = -1;
    if (pAudioSt != NULL) {
        pthread_mutex_lock(&(pAudioSt->mutex_activeaudio));
        is_active = pAudioSt->pActiveAudio[circuit - 1];
        pthread_mutex_unlock(&(pAudioSt->mutex_activeaudio));
    }
    return is_active;
}

void * F_Th_Process_Audio(void * arg)
{
    int len         = -1;

    sMain * pMain = (sMain *)arg;
    if (pMain->pAudioCircuit == NULL) pthread_exit(0);

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_cleanup_push(cleanup_handler_Process_Audio, pMain->pAudioCircuit);
    SLOGT(LOG_DEBUG, "F_Th_Process_Audio: Démarrage thread\n");

    /* On ouvre le périphériphe audio */
    fd = open(SAMPLE_DEVICE, O_RDWR);
    if (fd == -1) {
        couleur(C_RED); SLOGT(LOG_ERR, "F_Th_Process_Audio: open error on %s...([%d] %s)\n", SAMPLE_DEVICE, errno, strerror(errno)); couleur_r;
        pthread_exit(0);
    } else {
        couleur(C_GREEN); SLOGT(LOG_NOTICE, "F_Th_Process_Audio: %s opened\n", SAMPLE_DEVICE); couleur_r;
    }

    /* Début de la boucle du thread */
    while (1) {
        //traitement_audio(pMain);
        sleep(1);
        pthread_testcancel();
    } /* fin while (1) */

    pthread_cleanup_pop(0);
    pthread_exit(0);
}



int traitement_audio(sMain * pMain)
{
    int circuit     = -1;
    int nb_circuits = pMain->pAudioCircuit->nb_circuits;
    int msg_id      = -1;
    int is_active   = -1;
    int nb_elem     = -1;
    int len         = -1;
    int index       =  0;
    int ret         =  0;
    
    t_short_char * iw;
    struct timespec gts1, gts2, gts3, gts4;
    sSockTab * pSockServ = NULL;
    sSockTab * pSockCli = NULL;
    
    /* Variables contenant les samples lus et écrit sur le module pto4M */
    short d_sample = 0;
    
    
    /*
        On lit dans le device pto_sample les 4 canaux
        Pour chaque circuit audio actif du point de vue PTO:
            On écrit les données audio lues de pto_sample dans la fifo d'envoi cliente correspondante au circuit
            On lit les données audio à partir de la fifo de reception serveur correspondante au circuit
        On écrit dans le device pto_sample les données lue pour les 4 canaux
    */
        
    /* Lecture sur le device /dev/pto_sample */
    len = read(fd, d_samples_rd, sizeof (*d_samples_rd));
    if ( len == -1 ) {
        couleur(C_RED); SLOGT(LOG_ERR, "F_Th_Process_Audio: read error...\n"); couleur_r;
        //pthread_exit(0);
        return (0);
    }
    len =  write(fd, d_samples_rd, sizeof (*d_samples_rd));
    if (len == -1) {
        couleur(C_RED); SLOGT(LOG_ERR, "F_Th_Process_Audio: write error...\n"); couleur_r;
        //pthread_exit(0);
        return (0);
    }

    clock_gettime(CLOCK_MONOTONIC, &gts1);
    /* On parcours chaque circuits audio */
    
    for (circuit = 1; circuit <= nb_circuits; circuit++) {
        /* Si l'audio est active pour ce circuit */
        is_active = F_ProcAudio_Get_ActiveAudio(pMain->pAudioCircuit, circuit);
        pSockServ = pMain->pSockets[circuit];
        pSockCli = pMain->pSocketsCli[circuit];

        if (is_active == CC_AUDIO_STATUS_ACTIVE) {
            /* Lecture des données reçues sur le device pto_sample
              et ecriture dans la FIFO d'emission de la socket cliente
             */

            /* On écrit les données audio lue depuis le device /dev/pto_sample dans la fifo d'emission cliente associée au circuit */
            switch (circuit) {
                case CTP_CIRCUIT_1:
                    //SLOGT(LOG_DEBUG, "%d\n", (short)d_samples_rd->chan_1[0]);
                    //memcpy(d_tmp, d_samples_rd->chan_1, ZT_CHUNKSIZE * sizeof (int));
                    //memcpy(snd_msg, ((char *)d_samples_rd->chan_1), ZT_CHUNKSIZE * 2);
                    //F_Socket_Ecrire(pSockCli, ((char *)d_samples_rd->chan_1), ZT_CHUNKSIZE * 2);
                    F_Socket_Ecrire(pSockCli, ((char *)rcv_msg), ZT_CHUNKSIZE * 2);
                    break;
                case CTP_CIRCUIT_2:
                    //SLOGT(LOG_DEBUG, "%d\n", (short)d_samples_rd->chan_2[0]);
                    //memcpy(d_tmp, d_samples_rd->chan_2, ZT_CHUNKSIZE * sizeof (int));
                    memcpy(snd_msg, ((char *)d_samples_rd->chan_2), ZT_CHUNKSIZE * 2);
                    break;
                case CTP_CIRCUIT_3:
                    //SLOGT(LOG_DEBUG, "%d\n", (short)d_samples_rd->chan_2[0]);
                    //memcpy(d_tmp, d_samples_rd->chan_3, ZT_CHUNKSIZE * sizeof (int));
                    memcpy(snd_msg, ((char *)d_samples_rd->chan_3), ZT_CHUNKSIZE * 2);
                    break;
                case CTP_CIRCUIT_4:
                    //SLOGT(LOG_DEBUG, "%d\n", (short)d_samples_rd->chan_2[0]);
                    //memcpy(d_tmp, d_samples_rd->chan_4, ZT_CHUNKSIZE * sizeof (int));
                    memcpy(snd_msg, ((char *)d_samples_rd->chan_4), ZT_CHUNKSIZE * 2);
                    break;
                default:
                    SLOGT(LOG_ERR, "ERROR\n");
            } /* fin switch (circuit) */
            
            /* On sérialise les samples audio pour le buffer d'envoi de la socket */
            index = 0;
            //SLOGT(LOG_DEBUG, "Read on pto_sample, chan %d: ", circuit);
            /*
            while (index < ZT_CHUNKSIZE) {
                //SLOGT(LOG_DEBUG, "%04x ", (short)d_tmp[index]);
                ((short *)snd_msg)[index] = htons((short)d_tmp[index]);
                index++;
            }
            */
            //printf("\n");

            snd_len = ZT_CHUNKSIZE * sizeof (short);
            #if _OLD_SCHED_
            F_SockQueue_Add(&(pSockCli->snd_queue), snd_msg, snd_len);

            F_SockQueue_Remove(&(pSockServ->rcv_queue), &msg_id, rcv_msg, &rcv_len);
            #endif _OLD_SCHED_
            
            /* Ajout des données lues à la fifo de réception */
            // if ( F_SockQueue_Add(&(pSockets[i]->rcv_queue), pSockets[i]->rcv_buf, pSockets[i]->rcv_len) < 0)

            
            /* On vérifie que la trame reçu correspond à un échantillon 16-bits (2 octets) */
            
                /* On désérialise les octets reçus en tableaux de shorts (correspondant aux échantillons audio) */
                /* On utilise une union pour déserialiser les deux octets reçus et les convertir en short. En fonction de l'architecture,
                la valeur short sera lue en little ou big endian.
                Par exemple: Pour la trame UDP reçue: 0x37 0x3E
                La valeur short sera lue,
                    Pour une archi PowerPC (Big-endian):  iw->w = 0x373E
                    Pour une archi x86 (Little-endian):   iw->w = 0x3E37
                    
                Pour que la valeur soit lue de façon identique quelque soit l'archi (0x373E), on reconverti la valeur en network byte order
                */

                //memset(d_tmp, 0, ZT_CHUNKSIZE * sizeof (int));
                /*
                index = 0;
                while (index < (int)rcv_len / 2) {
                    d_tmp[index] = (int)htons(((short *)rcv_msg)[index * 2]);
                    ++index;
                }
                */
                switch (circuit) {
                    case CTP_CIRCUIT_1:
                        //memcpy(d_samples_wr->chan_1, d_tmp, ZT_CHUNKSIZE * sizeof (int));
                        //memcpy((char *)d_samples_wr->chan_1, pSockServ->rcv_buf, ZT_CHUNKSIZE * 2);
                        //F_Socket_Lire_Raw(pSockServ, (char *)d_samples_wr->chan_1, &len);
                        F_Socket_Lire_Raw(pSockServ, (char *)rcv_msg, &rcv_len);
                        break;
                    case CTP_CIRCUIT_2:
                        //memcpy(d_samples_wr->chan_2, d_tmp, ZT_CHUNKSIZE * sizeof (int));
                        memcpy((char *)d_samples_wr->chan_2, pSockServ->rcv_buf, ZT_CHUNKSIZE * 2);
                        break;
                    case CTP_CIRCUIT_3:
                        //memcpy(d_samples_wr->chan_3, d_tmp, ZT_CHUNKSIZE * sizeof (int));
                        memcpy((char *)d_samples_wr->chan_3, pSockServ->rcv_buf, ZT_CHUNKSIZE * 2);
                        break;
                    case CTP_CIRCUIT_4:
                        //memcpy(d_samples_wr->chan_4, d_tmp, ZT_CHUNKSIZE * sizeof (int));
                        memcpy((char *)d_samples_wr->chan_4, pSockServ->rcv_buf, ZT_CHUNKSIZE * 2);
                        break;
                    default:
                        SLOGT(LOG_ERR, "ERROR\n");
                } /* fin switch (circuit) */

        } /* Fin if is_active */
        else {
            /* Si l'audio est inactive sur le circuit, on nettoie les FIFO d'emission / réception des données audio */
            if (F_SockQueue_GetNbElem(pSockServ->rcv_queue) > 50 ) {
                SLOGT(LOG_INFO, "Drop pSockServ->rcv_queue\n");
                F_SockQueue_Clear(&(pSockServ->rcv_queue));
            }
            if (F_SockQueue_GetNbElem(pSockCli->snd_queue) > 50 ) {
                F_SockQueue_Clear(&(pSockCli->snd_queue));
                SLOGT(LOG_INFO, "Drop pSockCli->snd_queue\n");
            }
        }
    } /* Fin for */
    clock_gettime(CLOCK_MONOTONIC, &gts2);

    gts3.tv_sec = gts2.tv_sec - gts1.tv_sec;
    gts3.tv_nsec = gts2.tv_nsec - gts1.tv_nsec;
    
    //if (gts3.tv_nsec / 1000000 > 10) {
    SLOGT(LOG_NOTICE, "Traitement process audio: %d.%03d -> %d.%03d (%d.%03d)\n",
            gts1.tv_sec, gts1.tv_nsec / 1000000,
             gts2.tv_sec, gts2.tv_nsec / 1000000,
              gts3.tv_sec, gts3.tv_nsec / 1000000);
              

    //SLOGT(LOG_DEBUG, "F_Th_Process_Audio circuit %d: Message audio à écrire sur le device:\n", 1);
    //SLOGT(LOG_INFO, "ECRITURE SUR PTO_SAMPLE\n");
    //NLOG_INFO(log_hexprint((unsigned char *)d_samples_wr->chan_1, ZT_CHUNKSIZE * sizeof (int)));
    return EXIT_SUCCESS;
}
