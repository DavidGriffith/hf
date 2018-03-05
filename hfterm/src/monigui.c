# include "gui.h"

/*
 *  interface for monitor window
 */

GtkWidget*
create_wmonitor (void)
{
  GtkWidget *wmonitor;
  GtkWidget *scrolledwindow3;
  GtkWidget *textmonitor;
  GtkAccelGroup *moni_accels;
  
  wmonitor = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (wmonitor, "wmonitor");
  gtk_object_set_data (GTK_OBJECT (wmonitor), "wmonitor", wmonitor);
  gtk_window_set_title (GTK_WINDOW (wmonitor), _("Monitor"));
  gtk_window_set_policy (GTK_WINDOW (wmonitor), TRUE, TRUE, FALSE);
  gtk_window_set_wmclass (GTK_WINDOW (wmonitor), "hfterm", "hfterm");

  moni_accels = gtk_accel_group_new ();
  gtk_accel_group_attach(moni_accels, GTK_OBJECT(wmonitor));

  scrolledwindow3 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow3, "scrolledwindow3");
  gtk_widget_ref (scrolledwindow3);
  gtk_object_set_data_full (GTK_OBJECT (wmonitor), "scrolledwindow3", scrolledwindow3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow3);
  gtk_container_add (GTK_CONTAINER (wmonitor), scrolledwindow3);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow3), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  textmonitor = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (textmonitor, "textmonitor");
  gtk_widget_ref (textmonitor);
  gtk_object_set_data_full (GTK_OBJECT (wmonitor), "textmonitor", textmonitor,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (textmonitor);
  gtk_container_add (GTK_CONTAINER (scrolledwindow3), textmonitor);
  gtk_widget_set_usize (textmonitor, 512, 300);

  gtk_signal_connect (GTK_OBJECT (wmonitor), "delete_event",
                      GTK_SIGNAL_FUNC (gtk_widget_hide),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (textmonitor), "delete_event",
                      GTK_SIGNAL_FUNC (gtk_widget_hide),
                      NULL);

  gtk_widget_add_accelerator(wmonitor, "hide", moni_accels, 
  			      GDK_m, GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);  
  gtk_widget_add_accelerator(wmonitor, "hide", moni_accels, 
  			      GDK_Escape, 0, 
                              GTK_ACCEL_VISIBLE);  
  
  
 
  return wmonitor;
}

