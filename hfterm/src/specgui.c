# include "gui.h"

/*
 *  interface for spectrum window
 */

GtkWidget*
create_wspec (void)
{
  GtkWidget *wspec;
  GtkWidget *vbox1;
  GtkWidget *hbox1;
  GtkWidget *label1;
  GtkWidget *specfreqpointer;
  GtkWidget *label2;
  GtkWidget *specfreqmark;
  GtkWidget *label3;
  GtkWidget *specfreqspace;
  GtkWidget *frame1;
  GtkWidget *hbox2;
  GSList *hbox2_group = NULL;
  GtkWidget *shift170;
  GtkWidget *shift200;
  GtkWidget *shift425;
  GtkWidget *shift800;
  GtkWidget *shiftother;
  GtkWidget *frame4;
  GtkWidget *table9;
  GtkWidget *spec;
  GtkWidget *hruler1;
  GtkWidget *squelch_vruler;
  GtkAccelGroup *spec_accels;
  
  wspec = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (wspec, "wspec");
  gtk_object_set_data (GTK_OBJECT (wspec), "wspec", wspec);
  gtk_window_set_title (GTK_WINDOW (wspec), _("Frequency Spectrum"));
  gtk_window_set_policy (GTK_WINDOW (wspec), FALSE, FALSE, FALSE);
  gtk_window_set_wmclass (GTK_WINDOW (wspec), "hfterm", "hfterm");
  spec_accels = gtk_accel_group_new ();
  gtk_accel_group_attach(spec_accels, GTK_OBJECT(wspec));

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (vbox1, "vbox1");
  gtk_widget_ref (vbox1);
  gtk_object_set_data_full (GTK_OBJECT (wspec), "vbox1", vbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (wspec), vbox1);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (hbox1, "hbox1");
  gtk_widget_ref (hbox1);
  gtk_object_set_data_full (GTK_OBJECT (wspec), "hbox1", hbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, TRUE, 3);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 1);

  label1 = gtk_label_new (_("Pointer"));
  gtk_widget_set_name (label1, "label1");
  gtk_widget_ref (label1);
  gtk_object_set_data_full (GTK_OBJECT (wspec), "label1", label1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (hbox1), label1, FALSE, FALSE, 0);

  specfreqpointer = gtk_entry_new ();
  gtk_widget_set_name (specfreqpointer, "specfreqpointer");
  gtk_widget_ref (specfreqpointer);
  gtk_object_set_data_full (GTK_OBJECT (wspec), "specfreqpointer", specfreqpointer,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (specfreqpointer);
  gtk_box_pack_start (GTK_BOX (hbox1), specfreqpointer, FALSE, TRUE, 4);
  gtk_widget_set_usize (specfreqpointer, 60, -2);
  GTK_WIDGET_UNSET_FLAGS (specfreqpointer, GTK_CAN_FOCUS);
  gtk_entry_set_editable (GTK_ENTRY (specfreqpointer), FALSE);

  label2 = gtk_label_new (_("Mark"));
  gtk_widget_set_name (label2, "label2");
  gtk_widget_ref (label2);
  gtk_object_set_data_full (GTK_OBJECT (wspec), "label2", label2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label2);
  gtk_box_pack_start (GTK_BOX (hbox1), label2, FALSE, FALSE, 0);

  specfreqmark = gtk_entry_new ();
  gtk_widget_set_name (specfreqmark, "specfreqmark");
  gtk_widget_ref (specfreqmark);
  gtk_object_set_data_full (GTK_OBJECT (wspec), "specfreqmark", specfreqmark,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (specfreqmark);
  gtk_box_pack_start (GTK_BOX (hbox1), specfreqmark, FALSE, TRUE, 4);
  gtk_widget_set_usize (specfreqmark, 60, -2);
  GTK_WIDGET_UNSET_FLAGS (specfreqmark, GTK_CAN_FOCUS);
  gtk_entry_set_editable (GTK_ENTRY (specfreqmark), FALSE);

  label3 = gtk_label_new (_("Space"));
  gtk_widget_set_name (label3, "label3");
  gtk_widget_ref (label3);
  gtk_object_set_data_full (GTK_OBJECT (wspec), "label3", label3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label3);
  gtk_box_pack_start (GTK_BOX (hbox1), label3, FALSE, FALSE, 0);

  specfreqspace = gtk_entry_new ();
  gtk_widget_set_name (specfreqspace, "specfreqspace");
  gtk_widget_ref (specfreqspace);
  gtk_object_set_data_full (GTK_OBJECT (wspec), "specfreqspace", specfreqspace,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (specfreqspace);
  gtk_box_pack_start (GTK_BOX (hbox1), specfreqspace, FALSE, TRUE, 4);
  gtk_widget_set_usize (specfreqspace, 60, -2);
  GTK_WIDGET_UNSET_FLAGS (specfreqspace, GTK_CAN_FOCUS);
  gtk_entry_set_editable (GTK_ENTRY (specfreqspace), FALSE);

  frame1 = gtk_frame_new (_("Shift"));
  gtk_widget_set_name (frame1, "frame1");
  gtk_widget_ref (frame1);
  gtk_object_set_data_full (GTK_OBJECT (wspec), "frame1", frame1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame1);
  gtk_box_pack_end (GTK_BOX (hbox1), frame1, FALSE, TRUE, 0);

  hbox2 = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (hbox2, "hbox2");
  gtk_widget_ref (hbox2);
  gtk_object_set_data_full (GTK_OBJECT (wspec), "hbox2", hbox2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox2);
  gtk_container_add (GTK_CONTAINER (frame1), hbox2);

  shift170 = gtk_radio_button_new_with_label (hbox2_group, _("170"));
  hbox2_group = gtk_radio_button_group (GTK_RADIO_BUTTON (shift170));
  gtk_widget_set_name (shift170, "shift170");
  gtk_widget_ref (shift170);
  gtk_object_set_data_full (GTK_OBJECT (wspec), "shift170", shift170,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (shift170);
  gtk_box_pack_start (GTK_BOX (hbox2), shift170, FALSE, FALSE, 0);

  shift200 = gtk_radio_button_new_with_label (hbox2_group, _("200"));
  hbox2_group = gtk_radio_button_group (GTK_RADIO_BUTTON (shift200));
  gtk_widget_set_name (shift200, "shift200");
  gtk_widget_ref (shift200);
  gtk_object_set_data_full (GTK_OBJECT (wspec), "shift200", shift200,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (shift200);
  gtk_box_pack_start (GTK_BOX (hbox2), shift200, FALSE, FALSE, 0);

  shift425 = gtk_radio_button_new_with_label (hbox2_group, _("425"));
  hbox2_group = gtk_radio_button_group (GTK_RADIO_BUTTON (shift425));
  gtk_widget_set_name (shift425, "shift425");
  gtk_widget_ref (shift425);
  gtk_object_set_data_full (GTK_OBJECT (wspec), "shift425", shift425,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (shift425);
  gtk_box_pack_start (GTK_BOX (hbox2), shift425, FALSE, FALSE, 0);

  shift800 = gtk_radio_button_new_with_label (hbox2_group, _("800"));
  hbox2_group = gtk_radio_button_group (GTK_RADIO_BUTTON (shift800));
  gtk_widget_set_name (shift800, "shift800");
  gtk_widget_ref (shift800);
  gtk_object_set_data_full (GTK_OBJECT (wspec), "shift800", shift800,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (shift800);
  gtk_box_pack_start (GTK_BOX (hbox2), shift800, FALSE, FALSE, 0);

  shiftother = gtk_radio_button_new_with_label (hbox2_group, _("other"));
  hbox2_group = gtk_radio_button_group (GTK_RADIO_BUTTON (shiftother));
  gtk_widget_set_name (shiftother, "shiftother");
  gtk_widget_ref (shiftother);
  gtk_object_set_data_full (GTK_OBJECT (wspec), "shiftother", shiftother,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (shiftother);
  gtk_box_pack_start (GTK_BOX (hbox2), shiftother, FALSE, FALSE, 0);

  frame4 = gtk_frame_new (_("Spectrum"));
  gtk_widget_set_name (frame4, "frame4");
  gtk_widget_ref (frame4);
  gtk_object_set_data_full (GTK_OBJECT (wspec), "frame4", frame4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame4);
  gtk_box_pack_start (GTK_BOX (vbox1), frame4, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame4), 1);

  table9 = gtk_table_new (2, 2, FALSE);
  gtk_widget_set_name (table9, "table9");
  gtk_widget_ref (table9);
  gtk_object_set_data_full (GTK_OBJECT (wspec), "table9", table9,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table9);
  gtk_container_add (GTK_CONTAINER (frame4), table9);

  spec = spectrum_new ("spec", NULL, NULL, 0, 0);
  gtk_widget_set_name (spec, "spec");
  gtk_widget_ref (spec);
  gtk_object_set_data_full (GTK_OBJECT (wspec), "spec", spec,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (spec);
  gtk_table_attach (GTK_TABLE (table9), spec, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_widget_set_usize (spec, 512, 384);
  GTK_WIDGET_SET_FLAGS (spec, GTK_CAN_FOCUS);
  GTK_WIDGET_UNSET_FLAGS (spec, GTK_CAN_DEFAULT);
  gtk_widget_set_events (spec, GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK);

  hruler1 = gtk_hruler_new ();
  gtk_widget_set_name (hruler1, "hruler1");
  gtk_widget_ref (hruler1);
  gtk_object_set_data_full (GTK_OBJECT (wspec), "hruler1", hruler1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hruler1);
  gtk_table_attach (GTK_TABLE (table9), hruler1, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_ruler_set_range (GTK_RULER (hruler1), 0, 4000, 5.91797, 10);

  squelch_vruler = gtk_vruler_new ();
  gtk_widget_set_name (squelch_vruler, "squelch_vruler");
  gtk_widget_ref (squelch_vruler);
  gtk_object_set_data_full (GTK_OBJECT (wspec), "squelch_vruler", squelch_vruler,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (squelch_vruler);
  gtk_table_attach (GTK_TABLE (table9), squelch_vruler, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_ruler_set_range (GTK_RULER (squelch_vruler), 0, 10, 4.24479, 10);

  gtk_signal_connect (GTK_OBJECT (wspec), "delete_event",
                      GTK_SIGNAL_FUNC (on_wspec_delete_event),
                      NULL);
  gtk_widget_add_accelerator(wspec, "hide", spec_accels, 
  			      GDK_f, GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);  
  gtk_widget_add_accelerator(wspec, "hide", spec_accels, 
  			      GDK_Escape, 0, 
                              GTK_ACCEL_VISIBLE);  
  gtk_signal_connect (GTK_OBJECT (spec), "button_press_event",
                      GTK_SIGNAL_FUNC (on_spec_button_press_event),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (spec), "motion_notify_event",
                      GTK_SIGNAL_FUNC (on_spec_motion_event),
                      NULL);
  return wspec;
}


