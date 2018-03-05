#include "hft.h"
#include "callbacks.h"

/* ------------ LOGOOK ------------------------------------------ */ 

void logbook_window_show(GtkMenuItem *menuitem, gpointer user_data)
{
	if (log_on == 0)
	{
	  gtk_widget_show(gtk_object_get_data(GTK_OBJECT(wmain), "logbook_frame"));
	  gtk_widget_show (gtk_object_get_data(GTK_OBJECT(wmain),"save_logentry"));
	  gtk_widget_show (gtk_object_get_data(GTK_OBJECT(wmain),"clear_logentry"));
	  gtk_widget_hide (gtk_object_get_data(GTK_OBJECT(wmain),"new_logentry"));
	 
	  log_on = 1;
	  return;
	}
	else
	{
	  gtk_widget_hide(gtk_object_get_data(GTK_OBJECT(wmain), "logbook_frame")); 	
	  gtk_widget_hide (gtk_object_get_data(GTK_OBJECT(wmain),"save_logentry"));
	  gtk_widget_hide (gtk_object_get_data(GTK_OBJECT(wmain),"clear_logentry"));
	  gtk_widget_show (gtk_object_get_data(GTK_OBJECT(wmain),"new_logentry"));
	  log_on = 0;
	}
}

/* - durchsuchen ----------------------------------------------------- */
void on_search_entry1_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	gtk_widget_show(Wsearchlogentr);
}

void on_list_all_entrys1_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	gtk_widget_show(wlistalllog);
	log_list();
}

void on_show_qso_details1_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	gtk_widget_show(wqsoeditor);

}

void on_new_logentry_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	if (log_on == 1)
	{
	  display_status("log frame is visible, just enter dates and then click 'save entry'");
	}
	else
	{
	  display_status("New logentry: enter dates and then click 'OK & save'");
	  gtk_widget_show(wqsoeditor);	
	}
}

/* ----------------- SICHERN LOGBUCH -------------------------------- */
void on_save_logentry_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	log_save();
	log_list();
	display_status("log stored!");
}

void on_clear_logentry_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	if (actlog == 0) 
	//only clear window
	{
	  log_set(0);
	  return;
	}
	else
	{
	  log_delete_entry(actlog);
	  log_shift_entries(actlog);
	  actlog = 0;
	  log_set(0);
	  display_status("log entry cleared.");
	  log_store();
	}
}

void on_archivate_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	{
	  log_archivate();
	  log_delete_all();
	  log_list();
	  log_set(0);	  
	  log_store();
	}
	display_status("(log has been archivated on your demand.)");
}	
	
// at the moment unused	
void on_delete1_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	{
	  log_delete_all();
	  lb.logsize = 0;
	  log_list();
	  log_set(0);	  
	  log_store();
	}
	display_status("Why did you clear your log? Work them again!");
}	
	
void on_logentry_edit_clicked(GtkButton *button, gpointer user_data)
{
	log_search();
	gtk_widget_hide(Wsearchlogentr);
	if (actlog == 0) 
	{
	  return;
	}

	if (log_on == 0)
	{  
	  gtk_widget_show(wqsoeditor);
	}
	log_set(actlog);
	display_status("log entry loaded, You can edit it!");
}	

void on_logentry_clear_clicked (GtkButton *button, gpointer user_data)
{
	log_search();
	gtk_widget_hide(Wsearchlogentr);
	if (actlog == 0) 
	//only clear window
	{
	  log_set(0);
	  display_status("entry was not yet stored, only mask cleared.");
	  return;
	}
	else
	{
	  log_delete_entry(actlog);
	  log_shift_entries(actlog);
	  actlog = 0;
	  log_set(0);
	  display_status("log entry cleared.");
	  log_store();
	}
}

void on_logsearch_cancel_clicked (GtkButton *button, gpointer user_data)
{
	gtk_widget_hide(Wsearchlogentr);
	display_status("nothing changed in log.");
}

void on_loglist_ok_button_clicked (GtkButton *button, gpointer user_data)
{
	gtk_widget_hide(wlistalllog);
}

void on_qso_save_clicked (GtkButton *button, gpointer user_data)
{
	log_save();
	log_list();
	gtk_widget_hide(wqsoeditor);
}

void on_qso_clear_clicked (GtkButton *button, gpointer user_data)
{
	log_search();
	if (actlog > 0)
	{
	  log_delete_entry(actlog);
	  log_shift_entries(actlog);
	  actlog = 0;
	  log_set(0);
	  display_status("log entry cleared.");
	  log_store();
	}
	else 
	{
	  log_set(0);
          display_status("nothing changed in log.");	
	}
	gtk_widget_hide(wqsoeditor);
}

void on_qso_cancel_clicked (GtkButton *button, gpointer user_data)
{
	gtk_widget_hide(wqsoeditor);
	actlog = 0;
	log_set(0);
	display_status("nothing changed in log.");
}

void loglist_select(GtkCList *clist, gint row, gint column, GdkEventButton *event, gpointer user_data)
{
	actlog = row + 1;
	log_set (actlog);
	gtk_widget_hide(wlistalllog);
	if (log_on == 0)
	{
	  gtk_widget_show(wqsoeditor);
	}
	if (log_on == 0)
	{  
	  gtk_widget_show(wqsoeditor);
	}
	display_status("log entry loaded, You can edit it!");
}

