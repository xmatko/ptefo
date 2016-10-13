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
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <signal.h>

#include "common.h"
#include "tlog.h"
#include "heartbeat.h"
#include "code5t_proc.h"
#include "main.h"


static void timer_expire(sigval_t arg);
static int traitement(sMain * pMain);

/* Teste les fonctions du heartbeat */
/*
unsigned char hbtest = 0;
SLOGT(LOG_INFO, "Valeur des compteurs heartbeat: %u / %u / %u / %u\n",  pMain->pHB->counter,
                                                                        pMain->pHB->counter_last,
                                                                        pMain->pHB->counter_rcv);

SLOGT(LOG_INFO, "Get HB Value: %u / %u\n",  F_HB_GetValue(pMain->pHB), F_HB_GetValueRcv(pMain->pHB));

hbtest = F_HB_Inc(pMain->pHB);
SLOGT(LOG_INFO, "Inc HB = %u\n", hbtest);

SLOGT(LOG_INFO, "Get HB Value: %u / %u\n",  F_HB_GetValue(pMain->pHB), F_HB_GetValueRcv(pMain->pHB));
SLOGT(LOG_INFO, "Heartbeat status: %d\n", F_HB_GetStatus(pMain->pHB));

F_HB_ReceiveValue(pMain->pHB, 1);

SLOGT(LOG_INFO, "Heartbeat status: %d\n", F_HB_GetStatus(pMain->pHB));

hbtest = F_HB_Inc(pMain->pHB);
SLOGT(LOG_INFO, "Inc HB = %u\n", hbtest);
SLOGT(LOG_INFO, "Heartbeat status: %d\n", F_HB_GetStatus(pMain->pHB));

hbtest = F_HB_Inc(pMain->pHB);
SLOGT(LOG_INFO, "Inc HB = %u\n", hbtest);
SLOGT(LOG_INFO, "Heartbeat status: %d\n", F_HB_GetStatus(pMain->pHB));
*/

/*!
 * \fn static void hb_cleanup_handler(void * arg)
 * \brief Cleanup handler pour le thread F_Th_HB_Mgt
 * \param arg: pointeur sur structure sMain
 */
static void hb_cleanup_handler(void * arg)
{
    sMain * pMain = (sMain *)arg;
    SLOGT(LOG_DEBUG, "Th_HB_Mgt clean-up handler (%d)\n", pMain->dumb);
}


/*!
 * \fn void * Th_HB_Mgt(void * arg)
 * \brief Gestion du signal de vie
 * \param arg: pointeur sur structure sMain
 */
void * F_HB_Mgt(void * arg)
{
    sMain * pMain = (sMain *)arg;
    int ret = 0;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_cleanup_push(hb_cleanup_handler, (void *)pMain);
    SLOGT(LOG_DEBUG, "F_HB_Mgt: Demarrage thread\n");
    /* On récupere la config du heartbeat */
    pMain->pHB->timeout = pMain->pConfig->hb_timeout;
    /* Démarrage de la boucle du thread */
    //while(1) {
    //    while ((ret = F_Get_InhibStatus(pMain)) == 0) {
    //        F_HB_Inc(pMain->pHB);
    //        pthread_testcancel();
    //        sleep(pMain->pHB->timeout);
    //        F_HB_UpdateStatus(pMain->pHB);
    //    }
    //    pthread_testcancel();
    //    usleep(DEFAULT_THREAD_SLEEP_US);
    //} /* Fin while(1) */
    
    
    
    struct sigevent sev;
    struct itimerspec its;

    /* Create the timer */
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_value.sival_ptr = pMain;
    sev.sigev_notify_function = timer_expire;
    sev.sigev_notify_attributes = NULL;

    if (timer_create(CLOCK_MONOTONIC, &sev, &(pMain->pHB->hb_timer)) == -1)   die("timer_create");

    /* Armement du timer */
    its.it_value.tv_sec = pMain->pHB->timeout;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;

    timer_settime(pMain->pHB->hb_timer, 0, &its, NULL);
    
    /* Début de la boucle du thread */
    while (1) {
        sleep(1);
        pthread_testcancel();
    } /* fin while (1) */
    
     /* Fin de la boucle du thread */
    SLOGT(LOG_DEBUG, "F_Th_UDPClient: Fin thread\n");
    pthread_cleanup_pop(0);
    pthread_exit(0);
    
}



static void timer_expire(sigval_t arg)
{
    int gor = -1;
    sMain * pMain = (sMain *)arg.sival_ptr;
    gor = timer_getoverrun(pMain->pHB->hb_timer);
    if (gor > 0) {
        SLOGT(LOG_WARNING, "HB Timer overrun %d\n", gor);
    }
    traitement(pMain);

}

static int traitement(sMain * pMain)
{
    SLOGT(LOG_NOTICE, "HB Status: %d\n", F_HB_GetStatus(pMain->pHB));
    F_HB_UpdateStatus(pMain->pHB);
    if (F_Get_InhibStatus(pMain) == 0) {
        F_HB_Inc(pMain->pHB);
    }
    return EXIT_SUCCESS;
}

int F_HB_Init(sHeartbeat ** pHB)
{
    *pHB = (sHeartbeat*) malloc(sizeof (sHeartbeat));
    memset(*pHB, 0, sizeof (sHeartbeat));
    (*pHB)->counter = 0;
    (*pHB)->counter_last = 0;
    (*pHB)->counter_rcv = 0;
    (*pHB)->status = HB_INIT;
    (*pHB)->status_prev = HB_INIT;
    (*pHB)->timeout = -1;
    pthread_mutex_init(&((*pHB)->mutex_status), NULL);
    pthread_mutex_init(&((*pHB)->mutex_counters), NULL);
    return EXIT_SUCCESS;
}

int F_HB_Reset(sHeartbeat * pHB)
{
    if (pHB == NULL)
        return EXIT_FAILURE;

    pthread_mutex_lock(&(pHB->mutex_status));
    pthread_mutex_lock(&(pHB->mutex_counters));
    pHB->counter = 0;
    pHB->counter_last = 0;
    pHB->counter_rcv = 0;
    pHB->status = HB_INIT;
    pHB->status_prev = HB_INIT;
    pthread_mutex_unlock(&(pHB->mutex_counters));
    pthread_mutex_unlock(&(pHB->mutex_status));
    return EXIT_SUCCESS;
}

unsigned char F_HB_Inc(sHeartbeat * pHB)
{
    unsigned char cnt = 0;
    pthread_mutex_lock(&(pHB->mutex_counters));
    if(pHB->counter == 0xFF)
        pHB->counter = 0;
    else
        (pHB->counter)++;
    cnt = pHB->counter;
    pthread_mutex_unlock(&(pHB->mutex_counters));
    return cnt;
}

int F_HB_UpdatePrev(sHeartbeat * pHB)
{
    pthread_mutex_lock(&(pHB->mutex_counters));
    pHB->counter_last = pHB->counter;
    pthread_mutex_unlock(&(pHB->mutex_counters));
    return EXIT_SUCCESS;
}

unsigned char F_HB_GetValue(sHeartbeat * pHB)
{
    unsigned char cnt = 0;
    pthread_mutex_lock(&(pHB->mutex_counters));
    cnt = pHB->counter;
    pthread_mutex_unlock(&(pHB->mutex_counters));
    return cnt;
}

unsigned char F_HB_GetValuePrev(sHeartbeat * pHB)
{
    unsigned char cnt = 0;
    pthread_mutex_lock(&(pHB->mutex_counters));
    cnt = pHB->counter_last;
    pthread_mutex_unlock(&(pHB->mutex_counters));
    return cnt;
}

unsigned char F_HB_GetValueRcv(sHeartbeat * pHB)
{
    unsigned char cnt = 0;
    pthread_mutex_lock(&(pHB->mutex_counters));
    cnt = pHB->counter_rcv;
    pthread_mutex_unlock(&(pHB->mutex_counters));
    return cnt;
}

int F_HB_ReceiveValue(sHeartbeat * pHB, unsigned char val)
{
    pthread_mutex_lock(&(pHB->mutex_counters));
    pHB->counter_rcv = val;
    pthread_mutex_unlock(&(pHB->mutex_counters));
    return EXIT_SUCCESS;
}

int F_HB_UpdateStatus(sHeartbeat * pHB)
{
    int status = HB_UNKNOWN;
    pthread_mutex_lock(&(pHB->mutex_counters));
    pthread_mutex_lock(&(pHB->mutex_status));
    pHB->status_prev = pHB->status;
    pHB->status = HB_UNKNOWN;
    if (pHB->counter == pHB->counter_rcv) {
        //SLOGT(LOG_INFO, "Trame de vie identique snd/rcv (%u).\n", pHB->counter_last);
        pHB->status = HB_ALIVE;
    }
    else if ((pHB->counter == pHB->counter_rcv + 1) || ((pHB->counter == 0) && (pHB->counter_rcv == 0xFF))) {
        //SLOGT(LOG_INFO, "Trame de vie en attente d'acquittement (snd %u / rcv %u).\n", pHB->counter, pHB->counter_rcv);
        pHB->status = HB_WAIT_ACK;
    }
    else if ((pHB->counter == pHB->counter_rcv + 2) || ((pHB->counter == 0) && (pHB->counter_rcv == 0xFE)) || ((pHB->counter == 1) && (pHB->counter_rcv == 0xFF))) {
        //SLOGT(LOG_INFO, "Trame de vie en attente d'acquittement (snd %u / rcv %u).\n", pHB->counter, pHB->counter_rcv);
        pHB->status = HB_WAIT_ACK_LAST_CHANCE;
    }    
    else {
        pHB->status = HB_DEAD;
    }
    status = pHB->status;
    pthread_mutex_unlock(&(pHB->mutex_status));
    pthread_mutex_unlock(&(pHB->mutex_counters));
    return status;
    
}

int F_HB_GetStatus(sHeartbeat * pHB)
{
    int status = HB_UNKNOWN;
    pthread_mutex_lock(&(pHB->mutex_status));
    status = pHB->status;
    pthread_mutex_unlock(&(pHB->mutex_status));
    return status;
}

int F_HB_GetStatusPrev(sHeartbeat * pHB)
{
    int status_prev = HB_UNKNOWN;
    pthread_mutex_lock(&(pHB->mutex_status));
    status_prev = pHB->status_prev;
    pthread_mutex_unlock(&(pHB->mutex_status));
    return status_prev;
}
