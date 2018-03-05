# include "gui.h"

/*
 *  interface for map window
 */


GtkWidget*
create_wmap (void)
{
  GtkWidget *wmap;
  GtkWidget *vbox16;
  GtkWidget *scrolledwindow39;
  GtkWidget *viewport1;
  GtkWidget *pixmap1;
  GtkWidget *hbuttonbox3;
  GtkWidget *button23;
  GtkAccelGroup *map_accels;
  
  wmap = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (wmap, "wmap");
  gtk_object_set_data (GTK_OBJECT (wmap), "wmap", wmap);
  gtk_window_set_title (GTK_WINDOW (wmap), _("HFTERM - WORLD MAP"));
  gtk_window_set_wmclass (GTK_WINDOW (wmap), "hfterm", "hfterm");

  map_accels = gtk_accel_group_new ();
  gtk_accel_group_attach(map_accels, GTK_OBJECT(wmap));
  vbox16 = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (vbox16, "vbox16");
  gtk_widget_ref (vbox16);
  gtk_object_set_data_full (GTK_OBJECT (wmap), "vbox16", vbox16,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox16);
  gtk_container_add (GTK_CONTAINER (wmap), vbox16);

  scrolledwindow39 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow39, "scrolledwindow39");
  gtk_widget_ref (scrolledwindow39);
  gtk_object_set_data_full (GTK_OBJECT (wmap), "scrolledwindow39", scrolledwindow39,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow39);
  gtk_box_pack_start (GTK_BOX (vbox16), scrolledwindow39, TRUE, TRUE, 0);

  viewport1 = gtk_viewport_new (NULL, NULL);
  gtk_widget_set_name (viewport1, "viewport1");
  gtk_widget_ref (viewport1);
  gtk_object_set_data_full (GTK_OBJECT (wmap), "viewport1", viewport1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (viewport1);
  gtk_container_add (GTK_CONTAINER (scrolledwindow39), viewport1);
  
//  add_pixmap_directory("/usr/local/share/hfterm/pixmaps");
//  this is outcommented because it makes hfterm on my old box slooow  !!
//  same thing in main.c in main function
  pixmap1 = create_pixmap (wmap,"world3.xpm");
  gtk_widget_set_name (pixmap1, "pixmap1");
  gtk_widget_ref (pixmap1);
  gtk_object_set_data_full (GTK_OBJECT (wmap), "pixmap1", pixmap1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (pixmap1);
  gtk_container_add (GTK_CONTAINER (viewport1), pixmap1);
  gtk_misc_set_padding (GTK_MISC (pixmap1), 3, 3);

  hbuttonbox3 = gtk_hbutton_box_new ();
  gtk_widget_set_name (hbuttonbox3, "hbuttonbox3");
  gtk_widget_ref (hbuttonbox3);
  gtk_object_set_data_full (GTK_OBJECT (wmap), "hbuttonbox3", hbuttonbox3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbuttonbox3);
  gtk_box_pack_start (GTK_BOX (vbox16), hbuttonbox3, FALSE, FALSE, 3);
  gtk_container_set_border_width (GTK_CONTAINER (hbuttonbox3), 3);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonbox3), 0);
  gtk_button_box_set_child_size (GTK_BUTTON_BOX (hbuttonbox3), 0, 0);
  gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (hbuttonbox3), 3, 3);

  button23 = gtk_button_new_with_label (_("OK"));
  gtk_widget_set_name (button23, "button23");
  gtk_widget_ref (button23);
  gtk_object_set_data_full (GTK_OBJECT (wmap), "button23", button23,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button23);
  gtk_container_add (GTK_CONTAINER (hbuttonbox3), button23);
  GTK_WIDGET_SET_FLAGS (button23, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT (button23), "clicked",
                      GTK_SIGNAL_FUNC (on_enough_clicked),
                      NULL);
  gtk_widget_add_accelerator (button23, "clicked",
  			map_accels,
			GDK_Escape, 0, 
      	                GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (button23, "clicked",
  			map_accels,
			GDK_Return,
			0, 
      	                GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (button23, "clicked",
  			map_accels,
			GDK_w, GDK_SHIFT_MASK, 
      	                GTK_ACCEL_VISIBLE);

  return wmap;
}

