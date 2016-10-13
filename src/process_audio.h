/*!
 * \file process_audio.h
 * \brief Traitement et transfert du flux audio
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 30/06/2014
 */

#ifndef _PROCESS_AUDIO_H_
#define _PROCESS_AUDIO_H_

#define ZAP_CHAN_BASE   101
#define SAMPLE_DEVICE   "/dev/pto_sample"

#define SAMPLE_BUF_SZ   1000
#define FS              8000
#define MAX_STR         256




/*!
 * \struct sThArgServAudio server_udp.h
 * \brief Structure d'encapsulation pour passage d'argument aux threads F_Th_UDPServer_Audio
 */
typedef struct tThArgProcAudio
{
    void  * pMain;   /*!< Pointeur sur la structure principale. Attention a caster (void *) en (sMain *) */
    int     th_num; /*!< Numero du thread */
} sThArgProcAudio;

enum eAudioStatus {
    CC_AUDIO_STATUS_INACTIVE = 0,
    CC_AUDIO_STATUS_ACTIVE,
    CC_AUDIO_STATUS_UNKNOWN
};

typedef struct tAudioStatus
{
    int   * pActiveAudio;
    int     nb_circuits;
    int   * zfd;       /*! Pointeur sur les file descriptors des canaux ZAP */
    char ** zline;
    timer_t timerid;
    pthread_mutex_t     mutex_activeaudio;
} sAudioStatus;

int F_ProcAudio_Init_ActiveAudio(sAudioStatus ** pAudioSt, int nb_circuits);
int F_ProcAudio_Set_ActiveAudio(sAudioStatus * pAudioSt, int circuit, int val);
int F_ProcAudio_Get_ActiveAudio(sAudioStatus * pAudioSt, int circuit);


/*!
 * \fn void * F_Th_Process_Audio(void * arg)
 * \brief Thread de traitement des données audio
 * \param[in,out] arg: pointeur sur une structure sThArgServAudio
 *
 * Ce thread structe en permanence l'état de la file associée à la socket recevant les données audio.
 * Dès que la file contient au moins un élément, la fonction dépile les élements et tente de décoder les données audio.
 * devant y être contenues.\n
 * Le paquet audio est ensuite transferé à Asterisk.
 */
void * F_Th_Process_Audio(void * arg);


#endif /* _PROCESS_AUDIO_H_ */
