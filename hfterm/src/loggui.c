# include "gui.h"

/*
 *  interface for searchlogentry, listallog, qsoeditor windows
 */


GtkWidget* create_Wsearchlogentr (void)
{
  GtkWidget *Wsearchlogentr;
  GtkWidget *vbox7;
  GtkWidget *searchcall;
  GtkWidget *hseparator1;
  GtkWidget *hbox6;
  GtkWidget *button4;
  GtkWidget *button5;
  GtkWidget *button6;
  GtkTooltips *tooltips;
  GtkAccelGroup *logsearch_accels;
  tooltips = gtk_tooltips_new ();

  Wsearchlogentr = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (Wsearchlogentr, "Wsearchlogentr");
  gtk_object_set_data (GTK_OBJECT (Wsearchlogentr), "Wsearchlogentr", Wsearchlogentr);
  gtk_container_set_border_width (GTK_CONTAINER (Wsearchlogentr), 3);
  gtk_tooltips_set_tip (tooltips, Wsearchlogentr, _("Enter the call you search for, ..."), NULL);
  gtk_window_set_title (GTK_WINDOW (Wsearchlogentr), _("Search logbook entry"));
  gtk_window_set_wmclass (GTK_WINDOW (Wsearchlogentr), "hfterm", "hfterm");

  logsearch_accels = gtk_accel_group_new ();
  gtk_accel_group_attach(logsearch_accels, GTK_OBJECT(Wsearchlogentr));

  vbox7 = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (vbox7, "vbox7");
  gtk_widget_ref (vbox7);
  gtk_object_set_data_full (GTK_OBJECT (Wsearchlogentr), "vbox7", vbox7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox7);
  gtk_container_add (GTK_CONTAINER (Wsearchlogentr), vbox7);
  gtk_container_set_border_width (GTK_CONTAINER (vbox7), 3);

  searchcall = gtk_entry_new_with_max_length (15);
  gtk_widget_set_name (searchcall, "searchcall");
  gtk_widget_ref (searchcall);
  gtk_object_set_data_full (GTK_OBJECT (Wsearchlogentr), "searchcall", searchcall,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (searchcall);
  gtk_box_pack_start (GTK_BOX (vbox7), searchcall, TRUE, TRUE, 3);

  hseparator1 = gtk_hseparator_new ();
  gtk_widget_set_name (hseparator1, "hseparator1");
  gtk_widget_ref (hseparator1);
  gtk_object_set_data_full (GTK_OBJECT (Wsearchlogentr), "hseparator1", hseparator1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hseparator1);
  gtk_box_pack_start (GTK_BOX (vbox7), hseparator1, FALSE, FALSE, 0);

  hbox6 = gtk_hbox_new (TRUE, 0);
  gtk_widget_set_name (hbox6, "hbox6");
  gtk_widget_ref (hbox6);
  gtk_object_set_data_full (GTK_OBJECT (Wsearchlogentr), "hbox6", hbox6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox6);
  gtk_box_pack_start (GTK_BOX (vbox7), hbox6, FALSE, TRUE, 3);
  gtk_container_set_border_width (GTK_CONTAINER (hbox6), 3);

  button4 = gtk_button_new_with_label (_("Edit entry"));
  gtk_widget_set_name (button4, "button4");
  gtk_widget_ref (button4);
  gtk_object_set_data_full (GTK_OBJECT (Wsearchlogentr), "button4", button4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button4);
  gtk_box_pack_start (GTK_BOX (hbox6), button4, FALSE, FALSE, 0);

  button5 = gtk_button_new_with_label (_("Clear entry"));
  gtk_widget_set_name (button5, "button5");
  gtk_widget_ref (button5);
  gtk_object_set_data_full (GTK_OBJECT (Wsearchlogentr), "button5", button5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button5);
  gtk_box_pack_start (GTK_BOX (hbox6), button5, FALSE, FALSE, 0);

  button6 = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_set_name (button6, "button6");
  gtk_widget_ref (button6);
  gtk_object_set_data_full (GTK_OBJECT (Wsearchlogentr), "button6", button6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button6);
  gtk_box_pack_start (GTK_BOX (hbox6), button6, FALSE, FALSE, 0);

  gtk_signal_connect (GTK_OBJECT (button4), "clicked",
                      GTK_SIGNAL_FUNC (on_logentry_edit_clicked),
                      NULL);
  gtk_widget_add_accelerator (button4, "clicked",
  			logsearch_accels,
			GDK_Return, 0, 
      	                GTK_ACCEL_VISIBLE);
  
  gtk_signal_connect (GTK_OBJECT (button5), "clicked",
                      GTK_SIGNAL_FUNC (on_logentry_clear_clicked),
                      NULL);

  gtk_signal_connect (GTK_OBJECT (Wsearchlogentr), "delete_event",
                      GTK_SIGNAL_FUNC (on_logsearch_cancel_clicked),
                      NULL);
                
  gtk_signal_connect (GTK_OBJECT (button6), "clicked",
                      GTK_SIGNAL_FUNC (on_logsearch_cancel_clicked),
                      NULL);
  gtk_widget_add_accelerator (button6, "clicked",
  			logsearch_accels,
			GDK_Escape, 0, 
      	                GTK_ACCEL_VISIBLE);

  gtk_object_set_data (GTK_OBJECT (Wsearchlogentr), "tooltips", tooltips);

  return Wsearchlogentr;
}

GtkWidget*
create_wlistalllog (void)
{
  GtkWidget *wlistalllog;
  GtkWidget *vbox8;
  GtkWidget *scrolledwindow16;
  GtkWidget *clist1;
  GtkWidget *label49;
  GtkWidget *label50;
  GtkWidget *label51;
  GtkWidget *label52;
  GtkWidget *label53;
  GtkWidget *label54;
  GtkWidget *label55;
  GtkWidget *label56;
  GtkWidget *label57;
  GtkWidget *hseparator3;
  GtkWidget *hbox16;
  GtkWidget *button11;
  GtkTooltips *tooltips;
  GtkAccelGroup *loglist_accels;
  tooltips = gtk_tooltips_new ();

  wlistalllog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (wlistalllog, "wlistalllog");
  gtk_object_set_data (GTK_OBJECT (wlistalllog), "wlistalllog", wlistalllog);
  gtk_widget_set_usize (wlistalllog, 770, 455);
  gtk_tooltips_set_tip (tooltips, wlistalllog, _("\rThis is all you work ??? Hey OM, you got a lot of QRL ..."), NULL);
  gtk_window_set_title (GTK_WINDOW (wlistalllog), _("LOGBOOK"));
  gtk_window_set_wmclass (GTK_WINDOW (wlistalllog), "hfterm", "hfterm");
  gtk_window_set_policy (GTK_WINDOW (wlistalllog), FALSE, TRUE, TRUE);
  
  loglist_accels = gtk_accel_group_new ();
  gtk_accel_group_attach(loglist_accels, GTK_OBJECT(wlistalllog));

  vbox8 = gtk_vbox_new (FALSE, 3);
  gtk_widget_set_name (vbox8, "vbox8");
  gtk_widget_ref (vbox8);
  gtk_object_set_data_full (GTK_OBJECT (wlistalllog), "vbox8", vbox8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox8);
  gtk_container_add (GTK_CONTAINER (wlistalllog), vbox8);
  gtk_container_set_border_width (GTK_CONTAINER (vbox8), 1);

  scrolledwindow16 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow16, "scrolledwindow16");
  gtk_widget_ref (scrolledwindow16);
  gtk_object_set_data_full (GTK_OBJECT (wlistalllog), "scrolledwindow16", scrolledwindow16,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow16);
  gtk_box_pack_start (GTK_BOX (vbox8), scrolledwindow16, TRUE, TRUE, 3);
  gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow16), 3);

  clist1 = gtk_clist_new (9);
  gtk_widget_set_name (clist1, "clist1");
  gtk_widget_ref (clist1);
  gtk_object_set_data_full (GTK_OBJECT (wlistalllog), "clist1", clist1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (clist1);
  gtk_container_add (GTK_CONTAINER (scrolledwindow16), clist1);
  gtk_container_set_border_width (GTK_CONTAINER (clist1), 3);
 
  gtk_clist_set_column_auto_resize (GTK_CLIST (clist1), 0, TRUE);
  //gtk_clist_optimal_column_width (GTK_CLIST (clist1), 0);
  gtk_clist_set_column_auto_resize (GTK_CLIST (clist1), 1, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (clist1), 2, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (clist1), 3, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (clist1), 4, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (clist1), 5, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (clist1), 6, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (clist1), 7, TRUE);
  gtk_clist_set_column_auto_resize (GTK_CLIST (clist1), 8, TRUE);
  gtk_clist_column_titles_show (GTK_CLIST (clist1));

  label57 = gtk_label_new (_("  DATE  TIME  "));
  gtk_widget_set_name (label57, "label57");
  gtk_widget_ref (label57);
  gtk_object_set_data_full (GTK_OBJECT (wlistalllog), "label57", label57,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label57);
  gtk_clist_set_column_widget (GTK_CLIST (clist1), 0, label57);

  label50 = gtk_label_new (_(" CALL "));
  gtk_widget_set_name (label50, "label50");
  gtk_widget_ref (label50);
  gtk_object_set_data_full (GTK_OBJECT (wlistalllog), "label50", label50,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label50);
  gtk_clist_set_column_widget (GTK_CLIST (clist1), 1, label50);

  label49 = gtk_label_new (_("   NAME    "));
  gtk_widget_set_name (label49, "label49");
  gtk_widget_ref (label49);
  gtk_object_set_data_full (GTK_OBJECT (wlistalllog), "label49", label49,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label49);
  gtk_clist_set_column_widget (GTK_CLIST (clist1), 2, label49);

  label51 = gtk_label_new (_("    QTH    "));
  gtk_widget_set_name (label51, "label51");
  gtk_widget_ref (label51);
  gtk_object_set_data_full (GTK_OBJECT (wlistalllog), "label51", label51,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label51);
  gtk_clist_set_column_widget (GTK_CLIST (clist1), 3, label51);

  label52 = gtk_label_new (_("RST  in"));
  gtk_widget_set_name (label52, "label52");
  gtk_widget_ref (label52);
  gtk_object_set_data_full (GTK_OBJECT (wlistalllog), "label52", label52,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label52);
  gtk_clist_set_column_widget (GTK_CLIST (clist1), 4, label52);

  label53 = gtk_label_new (_("RST out"));
  gtk_widget_set_name (label53, "label53");
  gtk_widget_ref (label53);
  gtk_object_set_data_full (GTK_OBJECT (wlistalllog), "label53", label53,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label53);
  gtk_clist_set_column_widget (GTK_CLIST (clist1), 5, label53);

  label54 = gtk_label_new (_(" MODE "));
  gtk_widget_set_name (label54, "label54");
  gtk_widget_ref (label54);
  gtk_object_set_data_full (GTK_OBJECT (wlistalllog), "label54", label54,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label54);
  gtk_clist_set_column_widget (GTK_CLIST (clist1), 6, label54);

  label55 = gtk_label_new (_(" BAND "));
  gtk_widget_set_name (label55, "label55");
  gtk_widget_ref (label55);
  gtk_object_set_data_full (GTK_OBJECT (wlistalllog), "label55", label55,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label55);
  gtk_clist_set_column_widget (GTK_CLIST (clist1), 7, label55);

  label56 = gtk_label_new (_("    NOTES    "));
  gtk_widget_set_name (label56, "label56");
  gtk_widget_ref (label56);
  gtk_object_set_data_full (GTK_OBJECT (wlistalllog), "label56", label56,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label56);
  gtk_clist_set_column_widget (GTK_CLIST (clist1), 8, label56);

  hseparator3 = gtk_hseparator_new ();
  gtk_widget_set_name (hseparator3, "hseparator3");
  gtk_widget_ref (hseparator3);
  gtk_object_set_data_full (GTK_OBJECT (wlistalllog), "hseparator3", hseparator3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hseparator3);
  gtk_box_pack_start (GTK_BOX (vbox8), hseparator3, FALSE, FALSE, 0);

  hbox16 = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (hbox16, "hbox16");
  gtk_widget_ref (hbox16);
  gtk_object_set_data_full (GTK_OBJECT (wlistalllog), "hbox16", hbox16,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox16);
  gtk_box_pack_start (GTK_BOX (vbox8), hbox16, FALSE, FALSE, 3);

  button11 = gtk_button_new_with_label (_("OK"));
  gtk_widget_set_name (button11, "button11");
  gtk_widget_ref (button11);
  gtk_object_set_data_full (GTK_OBJECT (wlistalllog), "button11", button11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button11);
  gtk_box_pack_start (GTK_BOX (hbox16), button11, TRUE, TRUE, 0);
  
  gtk_signal_connect (GTK_OBJECT (clist1), "select-row",
                      GTK_SIGNAL_FUNC (loglist_select),
                      NULL);
 
  gtk_signal_connect (GTK_OBJECT (button11), "clicked",
                      GTK_SIGNAL_FUNC (on_loglist_ok_button_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (wlistalllog), "delete_event",
                      GTK_SIGNAL_FUNC (on_loglist_ok_button_clicked),
                      NULL);
  gtk_widget_add_accelerator (button11, "clicked",
  			loglist_accels,
			GDK_l, GDK_SHIFT_MASK, 
      	                GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (button11, "clicked",
  			loglist_accels,
			GDK_Escape, 0, 
      	                GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (button11, "clicked",
  			loglist_accels,
			GDK_Return, 0, 
      	                GTK_ACCEL_VISIBLE);

  gtk_object_set_data (GTK_OBJECT (wlistalllog), "tooltips", tooltips);

  return wlistalllog;
}

GtkWidget*
create_wqsoeditor (void)
{
  GtkWidget *wqsoeditor;
  GtkWidget *vbox9;
  GtkWidget *hbox7;
  GtkWidget *label57;
  GtkWidget *entry_call;
  GtkWidget *label58;
  GtkWidget *entry_name;
  GtkWidget *hseparator14;
  GtkWidget *hbox8;
  GtkWidget *label59;
  GtkWidget *entry_qth;
  GtkWidget *hseparator15;
  GtkWidget *hbox10;
  GtkWidget *hbox12;
  GtkWidget *label60;
  GtkWidget *entry_rstin;
  GtkWidget *label61;
  GtkWidget *entry_rstout;
  GtkWidget *hseparator17;
  GtkWidget *hbox11;
  GtkWidget *hbox13;
  GtkWidget *label62;
  GtkWidget *combo3;
  GList *combo3_items = NULL;
  GtkWidget *combo_entry_band;
  GtkWidget *label63;
  GtkWidget *combo4;
  GList *combo4_items = NULL;
  GtkWidget *combo_entry_mode;
  GtkWidget *hseparator18;
  GtkWidget *hbox14;
  GtkWidget *label64;
  GtkWidget *entry_notes;
  GtkWidget *hseparator2;
  GtkWidget *hbox15;
  GtkWidget *button8;
  GtkWidget *button9;
  GtkWidget *button10;
  GtkTooltips *tooltips;
  GtkAccelGroup *qso_accels;
  tooltips = gtk_tooltips_new ();

  wqsoeditor = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (wqsoeditor, "wqsoeditor");
  gtk_object_set_data (GTK_OBJECT (wqsoeditor), "wqsoeditor", wqsoeditor);
  gtk_container_set_border_width (GTK_CONTAINER (wqsoeditor), 3);
  gtk_tooltips_set_tip (tooltips, wqsoeditor, _("Log editor window! Edit, delete or cancel ..."), NULL);
  gtk_window_set_title (GTK_WINDOW (wqsoeditor), _("QSO Editor"));
  gtk_window_set_wmclass (GTK_WINDOW (wqsoeditor), "hfterm", "hfterm");

  qso_accels = gtk_accel_group_new ();
  gtk_accel_group_attach(qso_accels, GTK_OBJECT(wqsoeditor));

  vbox9 = gtk_vbox_new (FALSE, 1);
  gtk_widget_set_name (vbox9, "vbox9");
  gtk_widget_ref (vbox9);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "vbox9", vbox9,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox9);
  gtk_container_add (GTK_CONTAINER (wqsoeditor), vbox9);
  gtk_container_set_border_width (GTK_CONTAINER (vbox9), 3);

  hbox7 = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (hbox7, "hbox7");
  gtk_widget_ref (hbox7);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "hbox7", hbox7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox7);
  gtk_box_pack_start (GTK_BOX (vbox9), hbox7, FALSE, FALSE, 0);

  label57 = gtk_label_new (_("Call"));
  gtk_widget_set_name (label57, "label57");
  gtk_widget_ref (label57);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "label57", label57,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label57);
  gtk_box_pack_start (GTK_BOX (hbox7), label57, FALSE, FALSE, 3);
  gtk_misc_set_padding (GTK_MISC (label57), 3, 3);

  entry_call = gtk_entry_new_with_max_length (15); //call
  gtk_widget_set_name (entry_call, "entry_call");
  gtk_widget_ref (entry_call);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "entry_call", entry_call,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry_call);
  gtk_box_pack_start (GTK_BOX (hbox7), entry_call, TRUE, TRUE, 3);
  gtk_entry_set_editable (GTK_ENTRY (entry_call), TRUE);

  label58 = gtk_label_new(_("Name"));
  gtk_widget_set_name (label58, "label58");
  gtk_widget_ref (label58);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "label58", label58,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label58);
  gtk_box_pack_start (GTK_BOX (hbox7), label58, FALSE, FALSE, 0);
  gtk_misc_set_padding (GTK_MISC (label58), 3, 3);

  entry_name = gtk_entry_new_with_max_length (31); //name
  gtk_widget_set_name (entry_name, "entry_name");
  gtk_widget_ref (entry_name);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "entry_name", entry_name,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry_name);
  gtk_box_pack_start (GTK_BOX (hbox7), entry_name, TRUE, TRUE, 0);
  gtk_entry_set_editable (GTK_ENTRY (entry_name), TRUE);

  hseparator14 = gtk_hseparator_new ();
  gtk_widget_set_name (hseparator14, "hseparator14");
  gtk_widget_ref (hseparator14);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "hseparator14", hseparator14,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hseparator14);
  gtk_box_pack_start (GTK_BOX (vbox9), hseparator14, TRUE, TRUE, 0);

  hbox8 = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (hbox8, "hbox8");
  gtk_widget_ref (hbox8);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "hbox8", hbox8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox8);
  gtk_box_pack_start (GTK_BOX (vbox9), hbox8, FALSE, FALSE, 0);

  label59 = gtk_label_new (_("QTH"));
  gtk_widget_set_name (label59, "label59");
  gtk_widget_ref (label59);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "label59", label59,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label59);
  gtk_box_pack_start (GTK_BOX (hbox8), label59, FALSE, FALSE, 3);
  gtk_misc_set_padding (GTK_MISC (label59), 3, 3);

  entry_qth = gtk_entry_new_with_max_length (31); //qth
  gtk_widget_set_name (entry_qth, "entry_qth");
  gtk_widget_ref (entry_qth);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "entry_qth", entry_qth,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry_qth);
  gtk_box_pack_start (GTK_BOX (hbox8), entry_qth, TRUE, TRUE, 3);
  gtk_entry_set_editable (GTK_ENTRY (entry_qth), TRUE);

  hseparator15 = gtk_hseparator_new ();
  gtk_widget_set_name (hseparator15, "hseparator15");
  gtk_widget_ref (hseparator15);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "hseparator15", hseparator15,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hseparator15);
  gtk_box_pack_start (GTK_BOX (vbox9), hseparator15, TRUE, TRUE, 0);

  hbox10 = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (hbox10, "hbox10");
  gtk_widget_ref (hbox10);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "hbox10", hbox10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox10);
  gtk_box_pack_start (GTK_BOX (vbox9), hbox10, FALSE, FALSE, 0);

  hbox12 = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (hbox12, "hbox12");
  gtk_widget_ref (hbox12);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "hbox12", hbox12,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox12);
  gtk_box_pack_start (GTK_BOX (hbox10), hbox12, FALSE, FALSE, 0);

  label60 = gtk_label_new (_("RST  in"));
  gtk_widget_set_name (label60, "label60");
  gtk_widget_ref (label60);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "label60", label60,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label60);
  gtk_box_pack_start (GTK_BOX (hbox12), label60, TRUE, TRUE, 3);
  gtk_misc_set_padding (GTK_MISC (label60), 3, 3);

  entry_rstin = gtk_entry_new_with_max_length (6); //rstin
  gtk_widget_set_name (entry_rstin, "entry_rstin");
  gtk_widget_ref (entry_rstin);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "entry_rstin", entry_rstin,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry_rstin);
  gtk_box_pack_start (GTK_BOX (hbox12), entry_rstin, TRUE, TRUE, 3);
  gtk_entry_set_editable (GTK_ENTRY (entry_rstin), TRUE);

  label61 = gtk_label_new (_("RST out"));
  gtk_widget_set_name (label61, "label61");
  gtk_widget_ref (label61);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "label61", label61,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label61);
  gtk_box_pack_start (GTK_BOX (hbox12), label61, TRUE, TRUE, 3);
  gtk_misc_set_padding (GTK_MISC (label61), 3, 3);

  entry_rstout = gtk_entry_new_with_max_length (6); //rstout
  gtk_widget_set_name (entry_rstout, "entry_rstout");
  gtk_widget_ref (entry_rstout);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "entry_rstout", entry_rstout,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry_rstout);
  gtk_box_pack_start (GTK_BOX (hbox12), entry_rstout, TRUE, TRUE, 3);
  gtk_entry_set_editable (GTK_ENTRY (entry_rstout), TRUE);

  hseparator17 = gtk_hseparator_new ();
  gtk_widget_set_name (hseparator17, "hseparator17");
  gtk_widget_ref (hseparator17);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "hseparator17", hseparator17,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hseparator17);
  gtk_box_pack_start (GTK_BOX (vbox9), hseparator17, TRUE, TRUE, 0);

  hbox11 = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (hbox11, "hbox11");
  gtk_widget_ref (hbox11);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "hbox11", hbox11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox11);
  gtk_box_pack_start (GTK_BOX (vbox9), hbox11, FALSE, FALSE, 0);

  hbox13 = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (hbox13, "hbox13");
  gtk_widget_ref (hbox13);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "hbox13", hbox13,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox13);
  gtk_box_pack_start (GTK_BOX (hbox11), hbox13, TRUE, TRUE, 0);

  label62 = gtk_label_new (_("Band"));
  gtk_widget_set_name (label62, "label62");
  gtk_widget_ref (label62);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "label62", label62,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label62);
  gtk_box_pack_start (GTK_BOX (hbox13), label62, FALSE, FALSE, 3);
  gtk_misc_set_padding (GTK_MISC (label62), 3, 3);

  combo3 = gtk_combo_new();
  gtk_widget_set_name (combo3, "combo3");
  gtk_widget_ref (combo3);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "combo3", combo3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (combo3);
  gtk_box_pack_start (GTK_BOX (hbox13), combo3, FALSE, FALSE, 0);
  combo3_items = g_list_append (combo3_items, (gpointer) _("160m"));
  combo3_items = g_list_append (combo3_items, (gpointer) _("80m"));
  combo3_items = g_list_append (combo3_items, (gpointer) _("40m"));
  combo3_items = g_list_append (combo3_items, (gpointer) _("30m"));
  combo3_items = g_list_append (combo3_items, (gpointer) _("20m"));
  combo3_items = g_list_append (combo3_items, (gpointer) _("17m"));
  combo3_items = g_list_append (combo3_items, (gpointer) _("15m"));
  combo3_items = g_list_append (combo3_items, (gpointer) _("12m"));
  combo3_items = g_list_append (combo3_items, (gpointer) _("10m"));
  combo3_items = g_list_append (combo3_items, (gpointer) _("6m"));
  combo3_items = g_list_append (combo3_items, (gpointer) _("2m"));
  combo3_items = g_list_append (combo3_items, (gpointer) _("70cm"));
  combo3_items = g_list_append (combo3_items, (gpointer) _("23cm"));
  gtk_combo_set_popdown_strings (GTK_COMBO (combo3), combo3_items);
  g_list_free (combo3_items);

  combo_entry_band = GTK_COMBO (combo3)->entry;
  gtk_widget_set_name (combo_entry_band, "combo_entry_band");
  gtk_widget_ref (combo_entry_band);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "combo_entry_band", combo_entry_band,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (combo_entry_band);
  gtk_entry_set_editable (GTK_ENTRY (combo_entry_band), TRUE);
  gtk_entry_set_text (GTK_ENTRY (combo_entry_band), _("20 m"));

  label63 = gtk_label_new (_("Mode"));
  gtk_widget_set_name (label63, "label63");
  gtk_widget_ref (label63);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "label63", label63,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label63);
  gtk_box_pack_start (GTK_BOX (hbox13), label63, FALSE, FALSE, 3);
  gtk_misc_set_padding (GTK_MISC (label63), 3, 3);

  combo4 = gtk_combo_new();//_with_max_length (15);// mode, gtk_combo_new ();
  gtk_widget_set_name (combo4, "combo4");
  gtk_widget_ref (combo4);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "combo4", combo4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (combo4);
  gtk_box_pack_start (GTK_BOX (hbox13), combo4, FALSE, FALSE, 0);
  combo4_items = g_list_append (combo4_items, (gpointer) _("CW"));
  combo4_items = g_list_append (combo4_items, (gpointer) _("RTTY"));
  combo4_items = g_list_append (combo4_items, (gpointer) _("Amtor"));
  combo4_items = g_list_append (combo4_items, (gpointer) _("Pactor"));
  combo4_items = g_list_append (combo4_items, (gpointer) _("GTOR"));
  combo4_items = g_list_append (combo4_items, (gpointer) _("FSK"));
  combo4_items = g_list_append (combo4_items, (gpointer) _("MT63"));
  combo4_items = g_list_append (combo4_items, (gpointer) _("/*300 bd Packet Radio*/"));
  gtk_combo_set_popdown_strings (GTK_COMBO (combo4), combo4_items);
  g_list_free (combo4_items);

  combo_entry_mode = GTK_COMBO (combo4)->entry;
  gtk_widget_set_name (combo_entry_mode, "combo_entry_mode");
  gtk_widget_ref (combo_entry_mode);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "combo_entry_mode", combo_entry_mode,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (combo_entry_mode);
  gtk_entry_set_editable (GTK_ENTRY (combo_entry_mode), TRUE);
  gtk_entry_set_text (GTK_ENTRY (combo_entry_mode), _("Pactor"));

  hseparator18 = gtk_hseparator_new ();
  gtk_widget_set_name (hseparator18, "hseparator18");
  gtk_widget_ref (hseparator18);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "hseparator18", hseparator18,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hseparator18);
  gtk_box_pack_start (GTK_BOX (vbox9), hseparator18, TRUE, TRUE, 0);

  hbox14 = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (hbox14, "hbox14");
  gtk_widget_ref (hbox14);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "hbox14", hbox14,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox14);
  gtk_box_pack_start (GTK_BOX (vbox9), hbox14, FALSE, FALSE, 0);

  label64 = gtk_label_new (_("Notes"));
  gtk_widget_set_name (label64, "label64");
  gtk_widget_ref (label64);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "label64", label64,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label64);
  gtk_box_pack_start (GTK_BOX (hbox14), label64, FALSE, FALSE, 3);
  gtk_misc_set_padding (GTK_MISC (label64), 3, 3);

  entry_notes = gtk_entry_new_with_max_length (63);
  gtk_widget_set_name (entry_notes, "entry_notes");
  gtk_widget_ref (entry_notes);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "entry_notes", entry_notes,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry_notes);
  gtk_box_pack_start (GTK_BOX (hbox14), entry_notes, TRUE, TRUE, 3);
  gtk_entry_set_editable (GTK_ENTRY (entry_notes), TRUE);

  hseparator2 = gtk_hseparator_new ();
  gtk_widget_set_name (hseparator2, "hseparator2");
  gtk_widget_ref (hseparator2);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "hseparator2", hseparator2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hseparator2);
  gtk_box_pack_start (GTK_BOX (vbox9), hseparator2, FALSE, FALSE, 0);

  hbox15 = gtk_hbox_new (TRUE, 0);
  gtk_widget_set_name (hbox15, "hbox15");
  gtk_widget_ref (hbox15);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "hbox15", hbox15,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox15);
  gtk_box_pack_start (GTK_BOX (vbox9), hbox15, TRUE, TRUE, 3);
  gtk_container_set_border_width (GTK_CONTAINER (hbox15), 3);

  button8 = gtk_button_new_with_label (_("OK & Save"));
  gtk_widget_set_name (button8, "button8");
  gtk_widget_ref (button8);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "button8", button8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button8);
  gtk_box_pack_start (GTK_BOX (hbox15), button8, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (button8), 3);

  button9 = gtk_button_new_with_label (_("Clear"));
  gtk_widget_set_name (button9, "button9");
  gtk_widget_ref (button9);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "button9", button9,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button9);
  gtk_box_pack_start (GTK_BOX (hbox15), button9, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (button9), 3);

  button10 = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_set_name (button10, "button10");
  gtk_widget_ref (button10);
  gtk_object_set_data_full (GTK_OBJECT (wqsoeditor), "button10", button10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button10);
  gtk_box_pack_start (GTK_BOX (hbox15), button10, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (button10), 3);


  
  
  gtk_signal_connect (GTK_OBJECT (button8), "clicked",
                      GTK_SIGNAL_FUNC (on_qso_save_clicked),
                      NULL);
  gtk_widget_add_accelerator (button8, "clicked",
  			qso_accels,
			GDK_Return, 0, 
      	                GTK_ACCEL_VISIBLE);
  gtk_signal_connect (GTK_OBJECT (button9), "clicked",
                      GTK_SIGNAL_FUNC (on_qso_clear_clicked),
                      NULL);

  gtk_signal_connect (GTK_OBJECT (wqsoeditor), "delete_event",
                      GTK_SIGNAL_FUNC (gtk_widget_hide),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (button10), "clicked",
                      GTK_SIGNAL_FUNC (on_qso_cancel_clicked),
                      NULL);

  gtk_widget_add_accelerator (button10, "clicked",
  			qso_accels,
			GDK_Escape, 0, 
      	                GTK_ACCEL_VISIBLE);
 
  gtk_object_set_data (GTK_OBJECT (wqsoeditor), "tooltips", tooltips);

  return wqsoeditor;
}

