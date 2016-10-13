/*!
 * \file common.c
 * \brief Routines et fonctions diverses utilisée de façon commune
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 24/06/2014
 */
 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "tlog.h"
/*!
 * \def MIN_CHAR
 * \brief Utilisé pour la fonction F_SetRandomMsg
 */
#define MIN_CHAR 32
/*!
 * \def MAX_CHAR
 * \brief Utilisé pour la fonction F_SetRandomMsg
 */
#define MAX_CHAR 126

int opt_isnum(char * str)
{
    char v = 0;
    while(*(str+v) != 0) {
        if (!isdigit( *(str+v))) {
            return 0;
        }
        v++;
    }
    return 1;
}

void die(char *s)
{
    SLOGT(LOG_CRIT, "Died: %s ([%d] %s)\n", s, errno, strerror(errno));
    F_NLog_Close();
    exit(errno);
}

int F_SetRandomMsg(char ** msg, int max_len)
{
    char * tmp = NULL;
    char c = 0;
    int len = -1;
    int i = -1;
    
    if (*msg != NULL) {
        //printf("Pointeur déjà alloué. Liberation\n");
        free(*msg);
        *msg = NULL;
    }
    srand(time(NULL));
    len = (rand() % (max_len - 2)) + 2;
    if ((len < 2) || (len >= max_len)) {
        printf("Erreur rand. Revois ta formule, Albus !\n");
        return EXIT_FAILURE;
    }
    //printf("Random = %d\n", len);
    tmp = (char *)malloc(len * sizeof (char));
    memset(tmp, 0, len * sizeof (char));
    //printf("rand msg (len = %lu) = %s\n", strlen(tmp), tmp);
    /* ASCII entre 32 (0x20 - SPACE) et 126 (0x7E - ~) */
    for (i = 0; i < len - 1 ; i++) {
        c = (rand() % (MAX_CHAR - MIN_CHAR)) + MIN_CHAR;
        if ( (c < MIN_CHAR) || (c > MAX_CHAR)) {
            printf("Erreur rand. Revois ta formule, Potter !\n");
            return EXIT_FAILURE;
        }
        //printf("%d ", c);
        *(tmp + i) = c;
    }
    //printf("\n");
    //printf("rand msg (len = %lu) = %s\n", strlen(tmp), tmp);
    *msg = tmp;
    //printf("rand msg (len = %lu) = %s\n", strlen(*msg), *msg);
    return EXIT_SUCCESS;
}
