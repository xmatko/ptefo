/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 1999 - 2005, Digium, Inc.
 *
 * Mark Spencer <markster@digium.com>
 *
 * See http://www.asterisk.org for more information about
 * the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance;
 * the project provides a web site, mailing lists and IRC
 * channels for your use.
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2. See the LICENSE file
 * at the top of the source tree.
 */

#ifndef _ASTERISK_LOGGER_H
#define _ASTERISK_LOGGER_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif


#include <errno.h>

// SIEMA :definition des couleurs pour le debug //
#ifndef ASTERISK

#define ESC 0x1b

#define color_loc "[5;23f"		/* define row and column */
#define color_erasetoend "[0J"        /* clear the screen      */
#define color_cursorhome "[0H"	/* send the cursor home  */
#define color_clearscreen "[2J"  /* clear screen and send curor home */
#define color_revvideo "[7m"    /* reverse video */
#define color_reset "[0m"
#define color_bold "[1m"
#define color_underline "[4m"
#define color_black "[30m"
#define color_red "[0;31m"
#define color_green "[0;32m"
#define color_brown "[0;33m"
#define color_blue "[0;34m"
#define color_purple "[0;35m"
#define color_cyan "[0;36m"
#define color_yellow "[1;33m"   /* bold yellow */
#define color_bred "[1;31m"
#define color_bgreen "[1;32m"
#define color_bblue "[1;34m"
#define color_bpurple "[1;35m"
#define color_bcyan "[1;36m"
#define color_bwhite "[1;37m"
#define color_dred "[30;31m"    /* same as plain red    */
#define color_dgreen "[30;32m"  /* same as plain green  */
#define color_dblue "[30;34m"   /* same as plain blue   */
#define color_dpurple "[30;35m" /* same as plain purple */
#define color_dcyan "[30;36m"   /* same as plain cyan   */
#define color_grey "[30;37m"

// color_verbose ou color_log : = color definit plus haut
void siema_ast_text_color(char *color_verbose, char *color_log);
void siema_ast_last_text_color(); // reinitialise la couleur a la valeur d'avant le dernier appel s siema_ast_text_color

#endif // ASTERISK //


// SIEMA : issu du code d'Asterisk //

/*!
  \file logger.h
  \brief Support for logging to various files, console and syslog
	Configuration in file logger.conf
*/



//#include "asterisk/compat.h"

#include <stdarg.h>



#define EVENTLOG "event_log"
#define	QUEUELOG	"queue_log"

#define DEBUG_M(a) { \
	a; \
}

#define VERBOSE_PREFIX_1 " "
#define VERBOSE_PREFIX_2 "  == "
#define VERBOSE_PREFIX_3 "    -- "
#define VERBOSE_PREFIX_4 "       > "

void SetDebugLevel(int a);
void IncreaseDebugLevel(void);
void DecreaseDebugLevel(void);

/*! Used for sending a log message */
/*!
	\brief This is the standard logger function.  Probably the only way you will invoke it would be something like this:
	ast_log(LOG_WHATEVER, "Problem with the %s Captain.  We should get some more.  Will %d be enough?\n", "flux capacitor", 10);
	where WHATEVER is one of ERROR, DEBUG, EVENT, NOTICE, or WARNING depending
	on which log you wish to output to. These are implemented as macros, that
	will provide the function with the needed arguments.

 	\param level	Type of log event
	\param file	Will be provided by the LOG_* macro
	\param line	Will be provided by the LOG_* macro
	\param function	Will be provided by the LOG_* macro
	\param fmt	This is what is important.  The format is the same as your favorite breed of printf.  You know how that works, right? :-)
 */
void ast_log(int level, const char *file, int line, const char *function, const char *fmt, ...)
	__attribute__ ((format (printf, 5, 6)));



/*! Send a verbose message (based on verbose level)
 	\brief This works like ast_log, but prints verbose messages to the console depending on verbosity level set.
 	ast_verbose(VERBOSE_PREFIX_3 "Whatever %s is happening\n", "nothing");
 	This will print the message to the console if the verbose level is set to a level >= 3
 	Note the abscence of a comma after the VERBOSE_PREFIX_3.  This is important.
 	VERBOSE_PREFIX_1 through VERBOSE_PREFIX_3 are defined.
 */
void ast_verbose(const char *fmt, ...)
	__attribute__ ((format (printf, 1, 2)));

// printf avec une couleur d'affichage //
void color_printf(const char *color, const char *fmt, ...)
    __attribute__ ((format (printf, 2, 3)));

void pos_color_printf(int x, int y, const char *color, const char *fmt, ...)
    __attribute__ ((format (printf, 4, 5)));

#define _A_ __FILE__, __LINE__, __PRETTY_FUNCTION__

#ifdef __LOG_DEBUG
#undef __LOG_DEBUG
#endif
#ifndef LOG_DEBUG
#define LOG_DEBUG 7
#endif
#define __LOG_DEBUG      LOG_DEBUG, _A_

#ifdef __LOG_EVENT
#undef __LOG_EVENT
#endif
#ifndef LOG_INFO
#define LOG_INFO 6
#endif
#define LOG_EVENT   LOG_INFO
#define __LOG_EVENT      LOG_EVENT, _A_

#ifdef __LOG_NOTICE
#undef __LOG_NOTICE
#endif
#ifndef LOG_NOTICE
#define LOG_NOTICE 5
#endif
#define __LOG_NOTICE     LOG_NOTICE, _A_

#ifdef __LOG_WARNING
#undef __LOG_WARNING
#endif
#ifndef LOG_WARNING
#define LOG_WARNING 4
#endif
#define __LOG_WARNING    LOG_WARNING, _A_

#ifdef __LOG_ERROR
#undef __LOG_ERROR
#endif
#ifndef LOG_ERR
#define LOG_ERR 3
#endif
#define LOG_ERROR LOG_ERR
#define __LOG_ERROR      LOG_ERROR, _A_

#ifdef __LOG_VERBOSE
#undef __LOG_VERBOSE
#endif
#define LOG_VERBOSE LOG_DEBUG
#define __LOG_VERBOSE    LOG_VERBOSE, _A_

#ifdef __LOG_DTMF
#undef __LOG_DTMF
#endif
#define LOG_DTMF  LOG_DEBUG
#define __LOG_DTMF    LOG_DTMF, _A_

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _ASTERISK_LOGGER_H */
