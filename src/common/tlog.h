/*!
 * \file tlog.h
 * \brief Systeme de log des informations du programme
 * \author Nicolas Matkowski (nmatkowski@gmail.com)
 * \version 0.1
 * \date 24/06/2014
 */
 
#ifndef _TLOG_H_
#define _TLOG_H_

#include <syslog.h>
#include <errno.h>

#define NLOG_DEST_STDOUT 0x01
#define NLOG_DEST_SYSLOG 0x02

#define NLOG_INFO(a) if (F_NLog_GetLogLevel() >= LOG_INFO) { a; fflush(stdout); }
#define NLOG_DEBUG(a) if (F_NLog_GetLogLevel() == LOG_DEBUG) { a; fflush(stdout); }
#define NLOG_NOTICE(a) if (F_NLog_GetLogLevel() >= LOG_NOTICE) { a; fflush(stdout); }
#define NLOG_DEBUG2(a) if (_debug >= 2) { a; fflush(stdout); }
#define DEFAULT_VERB_LVL LOG_WARNING

/*!
 * \enum Couleur tlog.h
 * \brief Liste des couleurs utilisables lors de l'affichage des messages
 *
 * Un peu de couleur pour la sortie standard
 */
enum Couleur {
    C_RESET     = 0,     /*!< Reinitialise le système de couleur */
    C_RED       = 31,    /*!< Rouge */
    C_GREEN     = 32,    /*!< Vert */
    C_YELLOW    = 33,    /*!< Jaune */
    C_BLUE      = 34,    /*!< Bleu */
    C_MAGENTA   = 35,    /*!< Magenta */
    C_CYAN      = 36,    /*!< Cyan */
    C_WHITE     = 37     /*!< White */
};


/*!
  \def couleur(param)
  \brief Selectionne la couleur param
*/
#define couleur(param) printf("\033[%d;01m",param)
/*!
  \def couleur_r
  \brief Réinitialise le système de couleur aux valeurs par défaut
*/
#define couleur_r printf("\033[0m"); fflush(stdout);
/*!
 * \def MAX_LOG_MSG_SIZE
 * \brief Taille maximale d'un message de log (en octets)
 */
#define MAX_LOG_MSG_SIZE 512
/*!
 * \def SLOGT(lvl, msg, ...)
 * \brief Macro simplifiant l'appel de la fonction d'enregistrement d'un message de log
 */
#define SLOGT(lvl, ...) F_NLog_Print(lvl, __VA_ARGS__);

/*!
 * \fn void log_init(const char * progname)
 * \brief Initialise le système de log
 * \param[in] progname: nom du programme utilisé pour l'enregistrement dans les logs
 */
void F_NLog_Init(const char * progname, int dest_flags, int verbose_lvl);
void F_NLog_SetLogLevel(int a);
int F_NLog_GetLogLevel(void);
void F_NLog_IncreaseLogLevel(void);
void F_NLog_DecreaseLogLevel(void);

/*!
 * \fn void F_NLog_Close(void)
 * \brief Fermeture de l'acces au système de log
 */
void F_NLog_Close(void);

/*!
 * \fn void F_NLog_Print(int lvl, const char *msg, ...)
 * \brief Envoi un message au logger
 * \param[in] lvl: criticité du message
 * \param[in] msg: Chaine de caractère contenant le message
 * \param[in] ...: Liste variable d'argument
 *
 * La criticité peut être utilisé avec les niveaux suivants:\n
 * - LOG_EMERG    system is unusable\n
 * - LOG_ALERT    action must be taken immediately\n
 * - LOG_CRIT     critical conditions\n
 * - LOG_ERR      error conditions\n
 * - LOG_WARNING  warning conditions\n
 * - LOG_NOTICE   normal, but significant, conditionv
 * - LOG_INFO     informational message\n
 * - LOG_DEBUG    debug-level message\n
 *
 */
void F_NLog_Print(int lvl, const char *msg, ...)  __attribute__ ((format (printf, 2, 3)));

/*!
 * \fn void log_hexprint(const char * msg)
 * \brief Affiche les données d'une chaine d'octets au format hexadécimal
 * \param[in] msg: Chaine de caractère contenant le message à afficher en hexa
 * \param[in] size: Taille du message à afficher. Si size = -1, utilise la longueur de chaine pour detérminer la taille (DANGEREUX !!)
 */
void log_hexprint(const unsigned char * msg, int size);

void F_NLog_TestUsage(void);


#endif /* _TLOG_H_ */
