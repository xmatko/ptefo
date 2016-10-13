/*!
 * \file tlog.c
 * \brief Systeme éde log des informations du programme
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 24/06/2014
 */
 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "tlog.h"

static char flag_d_syslog = 0;
static char flag_d_stdout = 0;
static char v_lvl_max = DEFAULT_VERB_LVL;    /* Default log level */


void F_NLog_Init(const char * progname, int dest_flags, int verbose_lvl)
{
    if (dest_flags && NLOG_DEST_SYSLOG) {
        flag_d_syslog = 1;
        openlog(progname, LOG_PID|LOG_CONS, LOG_USER);
    }
    if (dest_flags && NLOG_DEST_STDOUT)
        flag_d_stdout = 1;
    
    F_NLog_SetLogLevel(verbose_lvl);
}

void F_NLog_Close(void)
{
    if (flag_d_syslog)
        closelog();
}

void F_NLog_SetLogLevel(int a) {
    if ((a >= LOG_EMERG) && (a <= LOG_DEBUG))
        v_lvl_max = a;
    //printf("Verbose level set to %d (asked %d)\n", v_lvl_max, a);
    if (flag_d_syslog)
        setlogmask(LOG_UPTO(v_lvl_max));
}

int F_NLog_GetLogLevel(void) {
    return v_lvl_max;
}

void F_NLog_IncreaseLogLevel(void) {
    int a = v_lvl_max;
    if (a < LOG_DEBUG) {
        a++;
        F_NLog_SetLogLevel(a);
    } else {
        printf("Verbose level already set to max (%d)\n", LOG_DEBUG);
    }
}

void F_NLog_DecreaseLogLevel(void) {
    int a = v_lvl_max;
    if (a > LOG_EMERG) {
        a--;
        F_NLog_SetLogLevel(a);
    } else {
        printf("Verbose level already set to min (%d)\n", LOG_EMERG);
    }
}

void F_NLog_Print(int lvl, const char * msg, ...)
{
    char msg_buffer[MAX_LOG_MSG_SIZE+1] = { 0 };
    va_list args;
    va_start(args, msg);
    vsnprintf(msg_buffer, MAX_LOG_MSG_SIZE, msg, args);
    
    if (flag_d_syslog)
        syslog(lvl, msg_buffer);
        
    if ((flag_d_stdout) && (lvl <= v_lvl_max)){
        printf(msg_buffer); fflush(stdout);
    }

    va_end(args);
}


void log_hexprint(const unsigned char * msg, int size)
{
    int i = 0;
    int bufsize = 0;
    unsigned char * buf_ptr = NULL;
    unsigned char * buf_str = NULL;

    if (size == -1)
        size = strlen((char *)msg);

    bufsize = 4 * (size + 1);
    //SLOGT(LOG_INFO, "buf_ptr is %d bytes long\n", bufsize);
    buf_ptr = (unsigned char *)malloc(bufsize * sizeof (unsigned char));
    //printf("Contenu de buf_ptr = 0x%08lx\n", (unsigned long)buf_ptr);
    memset((unsigned char *)buf_ptr, 0, bufsize * sizeof (unsigned char));
    
    buf_str = buf_ptr;
    //printf("Contenu de buf_str = 0x%08lx\n", (unsigned long)buf_str);
    
    for (i = 0; i < size; i++) {
        //printf("%d ", *(p_msg + i));
        if ((i != 0) & (i % 32 == 0)) {
            buf_ptr += sprintf((char *)buf_ptr, "\n");
        } else if ((i != 0) & (i % 8 == 0)) {
            buf_ptr += sprintf((char *)buf_ptr, " ");
        }
        buf_ptr += sprintf((char *)buf_ptr, "%02X",  *(msg + i));
        if (i < size) {
            buf_ptr += sprintf((char *)buf_ptr, " ");
        }
    }
    //sprintf(buf_ptr, "\n");
    *(buf_ptr + 1) = '\0';
    printf("%s\n", buf_str); fflush(stdout);
    //printf("Contenu de buf_str = 0x%08lx\n", (unsigned long)buf_str);
    //printf("Contenu de buf_ptr = 0x%08lx\n", (unsigned long)buf_ptr);
    free(buf_str);
    //printf("Contenu de buf_str = 0x%08lx\n", (unsigned long)buf_str);
    //printf("Contenu de buf_ptr = 0x%08lx\n", (unsigned long)buf_ptr);
}


void F_NLog_TestUsage(void)
{
    int test_num = 0;
    /*
    ast_log(_LOG_ERR, "Unable to listen!\n");
    ast_log(_LOG_WARNING, "Unable to listen!\n");
    ast_verbose("\n");
    ast_verbose(VERBOSE_PREFIX_3 "num1=%d num2=%d num3=%d\n", 1, 2, 3);
    ast_verbose(VERBOSE_PREFIX_3 "nb_chaines=%d\n", 8);
    */

    F_NLog_Init("Testprog", NLOG_DEST_STDOUT | NLOG_DEST_SYSLOG, LOG_INFO);
    test_num = 1;
    
    //F_NLog_Print(LOG_EMERG, "Test num %d de log niveau LOG_EMERG %d\n", test_num, LOG_EMERG);
    F_NLog_Print(LOG_ALERT, "Test num %d de log niveau LOG_ALERT %d\n", test_num, LOG_ALERT);
    F_NLog_Print(LOG_CRIT, "Test num %d de log niveau LOG_CRIT %d\n", test_num, LOG_CRIT);
    F_NLog_Print(LOG_ERR, "Test num %d de log niveau LOG_ERR %d\n", test_num, LOG_ERR);
    F_NLog_Print(LOG_WARNING, "Test num %d de log niveau LOG_WARNING %d\n", test_num, LOG_WARNING);
    F_NLog_Print(LOG_NOTICE, "Test num %d de log niveau LOG_NOTICE %d\n", test_num, LOG_NOTICE);
    F_NLog_Print(LOG_INFO, "Test num %d de log niveau LOG_INFO %d\n", test_num, LOG_INFO);
    F_NLog_Print(LOG_DEBUG, "Test num %d de log niveau LOG_DEBUG %d\n", test_num, LOG_DEBUG);
    
    F_NLog_DecreaseLogLevel();
    F_NLog_DecreaseLogLevel();
    test_num = 2;
    //F_NLog_Print(LOG_EMERG, "Test num %d de log niveau LOG_EMERG %d\n", test_num, LOG_EMERG);
    F_NLog_Print(LOG_ALERT, "Test num %d de log niveau LOG_ALERT %d\n", test_num, LOG_ALERT);
    F_NLog_Print(LOG_CRIT, "Test num %d de log niveau LOG_CRIT %d\n", test_num, LOG_CRIT);
    F_NLog_Print(LOG_ERR, "Test num %d de log niveau LOG_ERR %d\n", test_num, LOG_ERR);
    F_NLog_Print(LOG_WARNING, "Test num %d de log niveau LOG_WARNING %d\n", test_num, LOG_WARNING);
    F_NLog_Print(LOG_NOTICE, "Test num %d de log niveau LOG_NOTICE %d\n", test_num, LOG_NOTICE);
    F_NLog_Print(LOG_INFO, "Test num %d de log niveau LOG_INFO %d\n", test_num, LOG_INFO);
    F_NLog_Print(LOG_DEBUG, "Test num %d de log niveau LOG_DEBUG %d\n", test_num, LOG_DEBUG);
    
    F_NLog_SetLogLevel(LOG_DEBUG);
    test_num = 3;
    //F_NLog_Print(LOG_EMERG, "Test num %d de log niveau LOG_EMERG %d\n", test_num, LOG_EMERG);
    F_NLog_Print(LOG_ALERT, "Test num %d de log niveau LOG_ALERT %d\n", test_num, LOG_ALERT);
    F_NLog_Print(LOG_CRIT, "Test num %d de log niveau LOG_CRIT %d\n", test_num, LOG_CRIT);
    F_NLog_Print(LOG_ERR, "Test num %d de log niveau LOG_ERR %d\n", test_num, LOG_ERR);
    F_NLog_Print(LOG_WARNING, "Test num %d de log niveau LOG_WARNING %d\n", test_num, LOG_WARNING);
    F_NLog_Print(LOG_NOTICE, "Test num %d de log niveau LOG_NOTICE %d\n", test_num, LOG_NOTICE);
    F_NLog_Print(LOG_INFO, "Test num %d de log niveau LOG_INFO %d\n", test_num, LOG_INFO);
    F_NLog_Print(LOG_DEBUG, "Test num %d de log niveau LOG_DEBUG %d\n", test_num, LOG_DEBUG);
    
    test_num = 4;
    SLOGT(LOG_WARNING, "Message SLOGT sans arguments\n");
    SLOGT(LOG_INFO, "Message SLOGT %d %d %d\n", LOG_INFO, LOG_DEBUG, LOG_ERR);
    SLOGT(LOG_INFO, "Message SLOGT %d %d %d %d %d %d %d %d\n", LOG_EMERG, LOG_ALERT, LOG_CRIT, LOG_ERR, LOG_WARNING, LOG_NOTICE, LOG_INFO, LOG_DEBUG);
    
}
