#ifndef _GUI_H_
#define _GUI_H_

#include "hft.h"

/* rxtx.c : --------------------------------------------------------*/
GtkWidget* create_fileselection (gchar *title, gchar *filename);
/* specgui.c : -----------------------------------------------------*/
extern GtkWidget* create_wspec (void);
/* pargui.c : ------------------------------------------------------*/
extern GtkWidget* create_wpar (void);
/* maingui.c : -----------------------------------------------------*/
extern GtkWidget* create_wmain (void);

/* monigui.c : -----------------------------------------------------*/
extern GtkWidget* create_wmonitor (void);
/* fixgui.c : ------------------------------------------------------*/
extern GtkWidget* create_Wfixtext (void);
/* loggui.c : ------------------------------------------------------*/
extern GtkWidget* create_Wsearchlogentr (void);
GtkWidget* create_wlistalllog (void);
GtkWidget* create_wqsoeditor (void);
/* helpgui.c : -----------------------------------------------------*/
GtkWidget* create_wabout (void);
GtkWidget* create_whinweis (void);
GtkWidget* create_whilfe (void);
/* braggui.c : -----------------------------------------------------*/
GtkWidget* create_wpersonal (void);
/* mapgui.c : ------------------------------------------------------*/
//GtkWidget* create_wmap (void);

extern GtkWidget *textmain;
#endif /* _GUI_H_ */
