
////////////////////////////////////////////
// fonctions d'affichage pour le debogage //
////////////////////////////////////////////

#include "asterisk_logger.h"

#include <stdlib.h>
#include <stdio.h>

/* MACROS Debug */
extern int _siema_asterisk_debug;
int _siema_asterisk_debug = 0;

char verbose_text_color[20]= color_reset;
char log_text_color[20] = color_reset;
char last_verbose_text_color[20]= color_reset;
char last_log_text_color[20] = color_reset;

/* Some defines */
#define SIAMA_AST_DEBUG(a) if (_siema_asterisk_debug) { a; fflush(stdout); }
#define SIAMA_AST_DEBUG2(a) if (_siema_asterisk_debug >= 2) { a; fflush(stdout); }


void SetDebugLevel(int a)
{
    _siema_asterisk_debug = a;
}
void IncreaseDebugLevel(void)
{
    _siema_asterisk_debug++;
}
void DecreaseDebugLevel(void)
{
    _siema_asterisk_debug--;
}


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

    printf("%c%s",ESC, log_text_color);  // applique la couleur de texte //
	SIAMA_AST_DEBUG(printf("LOG: lev:%d file:%s  line:%d func: %s  ", level, file, line, function));
	SIAMA_AST_DEBUG(vprintf(fmt, vars));
	printf("%c%s",ESC,color_reset); // reinitialise la couleur de texte //
	fflush(stdout);
	va_end(vars);

	//if (debug_pf != NULL) vfprintf(debug_pf, fmt, vars);

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

	//if (debug_pf != NULL) vfprintf(debug_pf, fmt, vars);
}

void color_printf(const char *color, const char *fmt, ...)
{
	va_list vars;
	va_start(vars,fmt);
    printf("%c%s",ESC, color);  // applique la couleur de texte //
	vprintf(fmt, vars);
	printf("%c%s",ESC,color_reset); // reinitialise la couleur de texte //
	fflush(stdout);
	va_end(vars);

//	if (debug_pf != NULL) vfprintf(debug_pf, fmt, vars);
}


void pos_color_printf(int x, int y, const char *color, const char *fmt, ...)
{
	va_list vars;
	va_start(vars,fmt);
	printf("%c[s",ESC);  // sauvegarde la position du curseur
	if ((x >= 0) && ( y >= 0)) printf("%c[%d;%dH",ESC, y,x);  // positionne le curseur //
    else if (x >=0) printf("%c[%dG",ESC, x);  // positionne le curseur //
    else printf("%c[%dA",ESC,y);  // positionne le curseur //
	printf("%c%s",ESC, color);  // applique la couleur de texte //
	vprintf(fmt, vars);
	printf("%c%s",ESC,color_reset); // reinitialise la couleur de texte //
	printf("%c[u",ESC);  // restaure la position du curseur
	fflush(stdout);
	va_end(vars);

//	if (debug_pf != NULL) vfprintf(debug_pf, fmt, vars);
}
