/*****************************************************************************/
/*
 *      callbacks.h  --  callback functions for HF terminal.
 *
 *      Copyright (C) 1996  Thomas Sailer (sailer@ife.ee.ethz.ch)
 *        Swiss Federal Institute of Technology (ETH), Electronics Lab
 *
 *	modified 2001-2 by Axel Krystof DF3JRK
 * 	modified 2003 by Günther Montag DL4MGE
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 */

/*****************************************************************************/
      
#ifndef _CALLBACKS_H
#define _CALLBACKS_H

gboolean on_spec_motion_event(GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
gboolean on_spec_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
void on_squelchruler_click_event
    (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
void on_quit_activate(GtkMenuItem *menuitem, gpointer user_data);
//void on_rx_file_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_tx_file_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_beacon_file_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_become_irs_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_become_iss_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_qrt_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_speedup_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_uppercase_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_lowercase_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_figurecase_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_mailbox_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_standby_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_standby_pactor_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_pactor_arq_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_pactor_fec_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_gtor_arq1_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_amtor_arq_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_amtor_collective_fec_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_amtor_selective_fec_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_cw_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_rtty_receive_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_rtty_transmit_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_mt63_rx_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_mt63_tx_activate(GtkMenuItem *menuitem, gpointer user_data);

/* Personal Edit -----------------------------------------------------*/
void on_brag_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_brag_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_brag_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_brag_delete_button_clicked(GtkButton *button, gpointer user_data);

/* Fixtext -----------------------------------------------------------*/
void on_Wfixtext_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_button_Fixtext_OK_clicked (GtkButton *button, gpointer user_data);
void on_button_Fixtext_Cancel_clicked (GtkButton *button, gpointer user_data);
void on_Fix_button1_clicked(GtkButton *button, gpointer user_data);
void on_Fix_button2_clicked(GtkButton *button, gpointer user_data);
void on_Fix_button3_clicked(GtkButton *button, gpointer user_data);
void on_Fix_button4_clicked(GtkButton *button, gpointer user_data);
void on_Fix_button5_clicked(GtkButton *button, gpointer user_data);
void on_Fix_button6_clicked(GtkButton *button, gpointer user_data);
void on_Fix_button7_clicked(GtkButton *button, gpointer user_data);
void on_Fix_button8_clicked(GtkButton *button, gpointer user_data);
void on_Fix_button9_clicked(GtkButton *button, gpointer user_data);
void on_Fix_button10_clicked(GtkButton *button, gpointer user_data);
void on_Fix_button11_clicked(GtkButton *button, gpointer user_data);
void on_Fix_button12_clicked(GtkButton *button, gpointer user_data);

/* Utilities ---------------------------------------------------------*/
void on_frequency_spectrum_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_parameters_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_parabutton_clicked(GtkButton *button, gpointer user_data);
void on_parok_clicked(GtkButton *button, gpointer user_data);
void on_parcancel_clicked(GtkButton *button, gpointer user_data);

void on_monitor_activate(GtkMenuItem *menuitem, gpointer user_data);

void on_index1_activate(GtkMenuItem *menuitem, gpointer user_data);


/* Log ---------------------------------------------------------------*/
void logbook_window_show(GtkMenuItem *menuitem, gpointer user_data);
void on_new_logentry_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_save_logentry_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_clear_logentry_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_list_all_entrys1_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_search_entry1_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_loglist_ok_button_clicked (GtkButton *button, gpointer user_data);
void on_logentry_edit_clicked (GtkButton *button, gpointer user_data);
void on_logentry_clear_clicked (GtkButton *button, gpointer user_data);
void on_logsearch_cancel_clicked (GtkButton *button, gpointer user_data);
void on_show_qso_details1_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_qso_save_clicked (GtkButton *button, gpointer user_data);
void on_qso_clear_clicked (GtkButton *button, gpointer user_data);
void on_qso_cancel_clicked (GtkButton *button, gpointer user_data);
void loglist_select(GtkCList *clist, gint row, gint column, GdkEventButton *event, gpointer user_data);
void on_delete1_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_archivate_activate(GtkMenuItem *menuitem, gpointer user_data);
/*void loglist_select(GtkClist *clist, gpointer user_data);*/
void on_map1_activate(GtkMenuItem *menuitem, gpointer user_data);
gboolean on_tx_keypress_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
gboolean on_tx_keyrelease_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
gboolean on_textedit_button_press_event
    (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gboolean on_textedit_button_release_event
    (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gboolean on_rx_keypress_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
gboolean on_rx_keyrelease_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
gboolean on_wspec_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
gboolean on_wmain_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
gboolean on_wmain_destroy_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);

void on_about_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_aboutok_clicked (GtkButton *button, gpointer user_data);
void on_button17_clicked (GtkButton *button, gpointer user_data);

void on_button22_clicked (GtkButton *button, gpointer user_data);
void on_enough_clicked (GtkButton *button, gpointer user_data);

extern int lastrxcmd, lasttxcmd, way, fbbtest, beacon_file_prepared;
#endif /* _CALLBACKS_H */
