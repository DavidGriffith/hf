# include "hft.h"

/*
 *  interface for about, warning, help windows
 */

void showhelp(char* helptext, GtkWidget* helpwindow) 
{
  char docpath[256];
  memset(docpath, 0, sizeof(docpath));
  sprintf(docpath, "%s/share/hf/%s", PREFIX, helptext);
  textinsert(docpath, GTK_TEXT(helpwindow));
  memset(docpath, 0, sizeof(docpath));
}

GtkWidget*
create_wabout (void)
{
  GtkWidget *wabout;
  GtkWidget *vbox4;
  GtkWidget *vbox5;
  GtkWidget *label29;
  GtkWidget *label30;
  GtkWidget *hbuttonbox1;
  GtkWidget *button1;

  wabout = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (wabout, "wabout");
  gtk_object_set_data (GTK_OBJECT (wabout), "wabout", wabout);
  gtk_container_set_border_width (GTK_CONTAINER (wabout), 3);
  gtk_window_set_title (GTK_WINDOW (wabout), _("About hfterm..."));
  gtk_window_set_wmclass (GTK_WINDOW (wabout), "hfterm", "hfterm");

  vbox4 = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (vbox4, "vbox4");
  gtk_widget_ref (vbox4);
  gtk_object_set_data_full (GTK_OBJECT (wabout), "vbox4", vbox4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox4);
  gtk_container_add (GTK_CONTAINER (wabout), vbox4);

  vbox5 = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (vbox5, "vbox5");
  gtk_widget_ref (vbox5);
  gtk_object_set_data_full (GTK_OBJECT (wabout), "vbox5", vbox5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox5);
  gtk_box_pack_start (GTK_BOX (vbox4), vbox5, TRUE, TRUE, 0);

  label29 = gtk_label_new (_("HFTERM"));
  gtk_widget_set_name (label29, "label29");
  gtk_widget_ref (label29);
  gtk_object_set_data_full (GTK_OBJECT (wabout), "label29", label29,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label29);
  gtk_box_pack_start (GTK_BOX (vbox5), label29, TRUE, TRUE, 3);

  label30 = gtk_label_new (_("(C) 1999 Thomas Sailer, HB9JNX/AE4WA,\nwhith a little help by\nRalf-Axel Krystof, DF3JRK (df3jrk@gmx.de)\nGünther Montag, DL4MGE (dl4mge@darc.de) "));
  gtk_widget_set_name (label30, "label30");
  gtk_widget_ref (label30);
  gtk_object_set_data_full (GTK_OBJECT (wabout), "label30", label30,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label30);
  gtk_box_pack_start (GTK_BOX (vbox5), label30, TRUE, TRUE, 3);

  hbuttonbox1 = gtk_hbutton_box_new ();
  gtk_widget_set_name (hbuttonbox1, "hbuttonbox1");
  gtk_widget_ref (hbuttonbox1);
  gtk_object_set_data_full (GTK_OBJECT (wabout), "hbuttonbox1", hbuttonbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbuttonbox1);
  gtk_box_pack_start (GTK_BOX (vbox4), hbuttonbox1, TRUE, TRUE, 0);

  button1 = gtk_button_new_with_label (_("OK"));
  gtk_widget_set_name (button1, "button1");
  gtk_widget_ref (button1);
  gtk_object_set_data_full (GTK_OBJECT (wabout), "button1", button1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button1);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button1);
  gtk_container_set_border_width (GTK_CONTAINER (button1), 0);
  GTK_WIDGET_SET_FLAGS (button1, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT (wabout), "delete_event",
                      GTK_SIGNAL_FUNC (gtk_widget_hide),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (button1), "clicked",
                      GTK_SIGNAL_FUNC (on_aboutok_clicked),
                      NULL);
  return wabout;
}

GtkWidget*
create_whinweis (void)
{
  GtkWidget *whinweis;
  GtkWidget *vbox14;
  GtkWidget *vbox15;
  GtkWidget *label74;
  GtkWidget *label75;
  GtkWidget *hbuttonbox2;
  GtkWidget *button22;

  whinweis = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (whinweis, "whinweis");
  gtk_object_set_data (GTK_OBJECT (whinweis), "whinweis", whinweis);
  gtk_container_set_border_width (GTK_CONTAINER (whinweis), 3);
  gtk_window_set_title (GTK_WINDOW (whinweis), _("About this function"));
  gtk_window_set_position (GTK_WINDOW (whinweis), GTK_WIN_POS_CENTER);
  gtk_window_set_wmclass (GTK_WINDOW (whinweis), "hfterm", "hfterm");

  vbox14 = gtk_vbox_new (FALSE, 3);
  gtk_widget_set_name (vbox14, "vbox14");
  gtk_widget_ref (vbox14);
  gtk_object_set_data_full (GTK_OBJECT (whinweis), "vbox14", vbox14,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox14);
  gtk_container_add (GTK_CONTAINER (whinweis), vbox14);
  gtk_container_set_border_width (GTK_CONTAINER (vbox14), 3);

  vbox15 = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (vbox15, "vbox15");
  gtk_widget_ref (vbox15);
  gtk_object_set_data_full (GTK_OBJECT (whinweis), "vbox15", vbox15,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox15);
  gtk_box_pack_start (GTK_BOX (vbox14), vbox15, TRUE, TRUE, 0);

  label74 = gtk_label_new (_("HFTERM"));
  gtk_widget_set_name (label74, "label74");
  gtk_widget_ref (label74);
  gtk_object_set_data_full (GTK_OBJECT (whinweis), "label74", label74,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label74);
  gtk_box_pack_start (GTK_BOX (vbox15), label74, FALSE, TRUE, 3);

  label75 = gtk_label_new (_("Sri, ... Hfterm is still ALPHA !\nSo, this is Funktion is still not\nto use ..."));
  gtk_widget_set_name (label75, "label75");
  gtk_widget_ref (label75);
  gtk_object_set_data_full (GTK_OBJECT (whinweis), "label75", label75,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label75);
  gtk_box_pack_start (GTK_BOX (vbox15), label75, TRUE, TRUE, 3);

  hbuttonbox2 = gtk_hbutton_box_new ();
  gtk_widget_set_name (hbuttonbox2, "hbuttonbox2");
  gtk_widget_ref (hbuttonbox2);
  gtk_object_set_data_full (GTK_OBJECT (whinweis), "hbuttonbox2", hbuttonbox2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbuttonbox2);
  gtk_box_pack_start (GTK_BOX (vbox14), hbuttonbox2, FALSE, FALSE, 3);
  gtk_container_set_border_width (GTK_CONTAINER (hbuttonbox2), 3);

  button22 = gtk_button_new_with_label (_("OK"));
  gtk_widget_set_name (button22, "button22");
  gtk_widget_ref (button22);
  gtk_object_set_data_full (GTK_OBJECT (whinweis), "button22", button22,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button22);
  gtk_container_add (GTK_CONTAINER (hbuttonbox2), button22);
  GTK_WIDGET_SET_FLAGS (button22, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT (button22), "clicked",
                      GTK_SIGNAL_FUNC (on_button22_clicked),
                      NULL);

  return whinweis;
}
GtkWidget*
create_whilfe (void)
{


  GtkWidget *whilfe;
  GtkWidget *vbox10;
  GtkWidget *frame5;
  GtkWidget *vbox11;
  GtkWidget *helpnotebook;
  GtkWidget *label_help_1;
  GtkWidget *frame_help_1;
  GtkWidget *scrw_help_1;
  GtkWidget *text_help_1;

  GtkWidget *label_help_2;
  GtkWidget *frame_help_2;
  GtkWidget *scrw_help_2;
  GtkWidget *text_help_2;

  GtkWidget *label_help_3;
  GtkWidget *frame_help_3;
  GtkWidget *scrw_help_3;
  GtkWidget *text_help_3;

  GtkWidget *label_help_4;
  GtkWidget *frame_help_4;
  GtkWidget *scrw_help_4;
  GtkWidget *text_help_4;

  GtkWidget *label_help_5;
  GtkWidget *frame_help_5;
  GtkWidget *scrw_help_5;
  GtkWidget *text_help_5;

  GtkWidget *label_help_6;
  GtkWidget *frame_help_6;
  GtkWidget *scrw_help_6;
  GtkWidget *text_help_6;

  GtkWidget *label_help_7;
  GtkWidget *frame_help_7;
  GtkWidget *scrw_help_7;
  GtkWidget *text_help_7;

  GtkWidget *label_help_8;
  GtkWidget *frame_help_8;
  GtkWidget *scrw_help_8;
  GtkWidget *text_help_8;

  GtkWidget *label_help_9;
  GtkWidget *frame_help_9;
  GtkWidget *scrw_help_9;
  GtkWidget *text_help_9;

  GtkWidget *label_help_10;
  GtkWidget *frame_help_10;
  GtkWidget *scrw_help_10;
  GtkWidget *text_help_10;

  GtkWidget *label_help_11;
  GtkWidget *frame_help_11;
  GtkWidget *scrw_help_11;
  GtkWidget *text_help_11;

  GtkWidget *label_help_12;
  GtkWidget *frame_help_12;
  GtkWidget *scrw_help_12;
  GtkWidget *text_help_12;

  GtkWidget *hbox17;
  GtkWidget *button17;
  GtkAccelGroup *help_accels;
  GtkTooltips *tooltips;

  tooltips = gtk_tooltips_new ();
  whilfe = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (whilfe, "whilfe");
  gtk_object_set_data (GTK_OBJECT (whilfe), "whilfe", whilfe);
  gtk_window_set_title (GTK_WINDOW (whilfe), _("help for hfkernel & hfterm"));
  gtk_window_set_position (GTK_WINDOW (whilfe), GTK_WIN_POS_CENTER);

  gtk_window_set_wmclass (GTK_WINDOW (whilfe), "hfterm", "hfterm");
  help_accels = gtk_accel_group_new ();
  gtk_accel_group_attach(help_accels, GTK_OBJECT(whilfe));

  vbox10 = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (vbox10, "vbox10");
  gtk_widget_ref (vbox10);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "vbox10", vbox10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox10);
  gtk_container_add (GTK_CONTAINER (whilfe), vbox10);

  frame5 = gtk_frame_new (NULL);
  gtk_widget_set_name (frame5, "frame5");
  gtk_widget_ref (frame5);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "frame5", frame5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame5);
  gtk_box_pack_start (GTK_BOX (vbox10), frame5, TRUE, TRUE, 0);

  vbox11 = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (vbox11, "vbox11");
  gtk_widget_ref (vbox11);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "vbox11", vbox11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox11);
  gtk_container_add (GTK_CONTAINER (frame5), vbox11);

  helpnotebook = gtk_notebook_new ();
  gtk_widget_set_name (helpnotebook, "helpnotebook");
  gtk_widget_ref (helpnotebook);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "helpnotebook", helpnotebook,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (helpnotebook);
  gtk_box_pack_start (GTK_BOX (vbox11), helpnotebook, TRUE, TRUE, 0);

 
/* - Hilfe   1 -------------------------------------------------------*/

  frame_help_1 = gtk_frame_new (_(""));
  gtk_widget_set_name (frame_help_1, "frame_help_1");
  gtk_widget_ref (frame_help_1);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "frame_help_1", frame_help_1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame_help_1);
  gtk_container_add (GTK_CONTAINER (helpnotebook), frame_help_1);
  gtk_container_set_border_width (GTK_CONTAINER (frame_help_1), 3);

  scrw_help_1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrw_help_1, "scrw_help_1");
  gtk_widget_ref (scrw_help_1);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "scrw_help_1", scrw_help_1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrw_help_1);
  gtk_container_add (GTK_CONTAINER (frame_help_1), scrw_help_1);
  gtk_container_set_border_width (GTK_CONTAINER (scrw_help_1), 3);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrw_help_1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  text_help_1 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text_help_1, "text_help_1");
  gtk_widget_ref (text_help_1);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "text_help_1", text_help_1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text_help_1);
  gtk_container_add (GTK_CONTAINER (scrw_help_1), text_help_1);
  gtk_tooltips_set_tip (tooltips, text_help_1, _("Left Side English \nRight Side German"), NULL);
  gtk_text_set_editable (GTK_TEXT (text_help_1), FALSE);
  //textinsert("/usr/share/hf/HF-HOWTO.txt  ", GTK_TEXT(text_help_1));

  label_help_1 = gtk_label_new (_(""));
  gtk_widget_set_name (label_help_1, "label_help_1");
  gtk_widget_ref (label_help_1);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "label_help_1", label_help_1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_help_1);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (helpnotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (helpnotebook), 0), label_help_1);


/* - Hilfe 2 ------------------------------------------------------*/

  frame_help_2 = gtk_frame_new (_(""));
  gtk_widget_set_name (frame_help_2, "frame_help_2");
  gtk_widget_ref (frame_help_2);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "frame_help_2", frame_help_2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame_help_2);
  gtk_container_add (GTK_CONTAINER (helpnotebook), frame_help_2);
  gtk_container_set_border_width (GTK_CONTAINER (frame_help_2), 3);

  scrw_help_2 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrw_help_2, "scrw_help_2");
  gtk_widget_ref (scrw_help_2);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "scrw_help_2", scrw_help_2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrw_help_2);
  gtk_container_add (GTK_CONTAINER (frame_help_2), scrw_help_2);
  gtk_container_set_border_width (GTK_CONTAINER (scrw_help_2), 3);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrw_help_2), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  text_help_2 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text_help_2, "text_help_2");
  gtk_widget_ref (text_help_2);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "text_help_2", text_help_2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text_help_2);
  gtk_container_add (GTK_CONTAINER (scrw_help_2), text_help_2);
  gtk_tooltips_set_tip (tooltips, text_help_2, _("Left Side English \nRight Side German"), NULL);
  gtk_text_set_editable (GTK_TEXT (text_help_2), FALSE);
//  textinsert("/usr/share/hf/hfkernel.txt", GTK_TEXT(text_help_2));

  label_help_2 = gtk_label_new (_(""));
  gtk_widget_set_name (label_help_2, "label_help_2");
  gtk_widget_ref (label_help_2);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "label_help_2", label_help_2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_help_2);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (helpnotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (helpnotebook), 1), label_help_2);


/* - Hilfe 3 -------------------------------------------------------*/

  frame_help_3 = gtk_frame_new (_(""));
  gtk_widget_set_name (frame_help_3, "frame_help_3");
  gtk_widget_ref (frame_help_3);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "frame_help_3", frame_help_3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame_help_3);
  gtk_container_add (GTK_CONTAINER (helpnotebook), frame_help_3);
  gtk_container_set_border_width (GTK_CONTAINER (frame_help_3), 3);

  scrw_help_3 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrw_help_3, "scrw_help_3");
  gtk_widget_ref (scrw_help_3);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "scrw_help_3", scrw_help_3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrw_help_3);
  gtk_container_add (GTK_CONTAINER (frame_help_3), scrw_help_3);
  gtk_container_set_border_width (GTK_CONTAINER (scrw_help_3), 3);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrw_help_3), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  text_help_3 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text_help_3, "text_help_3");
  gtk_widget_ref (text_help_3);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "text_help_3", text_help_3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text_help_3);
  gtk_container_add (GTK_CONTAINER (scrw_help_3), text_help_3);
  gtk_tooltips_set_tip (tooltips, text_help_3, _("Left Side English \nRight Side German"), NULL);
  gtk_text_set_editable (GTK_TEXT (text_help_3), FALSE);
//  textinsert("/usr/share/hf/hfterm.txt", GTK_TEXT(text_help_3));

  label_help_3 = gtk_label_new (_(""));
  gtk_widget_set_name (label_help_3, "label_help_3");
  gtk_widget_ref (label_help_3);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "label_help_3", label_help_3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_help_3);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (helpnotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (helpnotebook), 2), label_help_3);

/* - Hilfe 4 ------------------------------------------------------*/

  frame_help_4 = gtk_frame_new (_("English main howto text"));
  gtk_widget_set_name (frame_help_4, "frame_help_4");
  gtk_widget_ref (frame_help_4);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "frame_help_4", frame_help_4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame_help_4);
  gtk_container_add (GTK_CONTAINER (helpnotebook), frame_help_4);
  gtk_container_set_border_width (GTK_CONTAINER (frame_help_4), 3);

  scrw_help_4 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrw_help_4, "scrw_help_4");
  gtk_widget_ref (scrw_help_4);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "scrw_help_4", scrw_help_4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrw_help_4);
  gtk_container_add (GTK_CONTAINER (frame_help_4), scrw_help_4);
  gtk_container_set_border_width (GTK_CONTAINER (scrw_help_4), 3);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrw_help_4), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  text_help_4 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text_help_4, "text_help_4");
  gtk_widget_ref (text_help_4);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "text_help_4", text_help_4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text_help_4);
  gtk_container_add (GTK_CONTAINER (scrw_help_4), text_help_4);
  gtk_tooltips_set_tip (tooltips, text_help_4, _("Left Side English \nRight Side German"), NULL);
  gtk_text_set_editable (GTK_TEXT (text_help_4), FALSE);

  showhelp("HF-HOWTO.txt", text_help_4);

  label_help_4 = gtk_label_new (_("HF-HOWTO"));
  gtk_widget_set_name (label_help_4, "label_help_4");
  gtk_widget_ref (label_help_4);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "label_help_4", label_help_4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_help_4);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (helpnotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (helpnotebook), 3), label_help_4);


/* - Hilfe 5 -------------------------------------------------------*/

  frame_help_5 = gtk_frame_new (_("Write MORE HELP & mail it to"
  	"hfterm-hackers@lists.sourceforge.net ! Thanks!"));
  gtk_widget_set_name (frame_help_5, "frame_help_5");
  gtk_widget_ref (frame_help_5);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "frame_help_5", frame_help_5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame_help_5);
  gtk_container_add (GTK_CONTAINER (helpnotebook), frame_help_5);
  gtk_container_set_border_width (GTK_CONTAINER (frame_help_5), 3);

  scrw_help_5 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrw_help_5, "scrw_help_5");
  gtk_widget_ref (scrw_help_5);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "scrw_help_5", scrw_help_5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrw_help_5);
  gtk_container_add (GTK_CONTAINER (frame_help_5), scrw_help_5);
  gtk_container_set_border_width (GTK_CONTAINER (scrw_help_5), 3);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrw_help_5), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  text_help_5 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text_help_5, "text_help_5");
  gtk_widget_ref (text_help_5);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "text_help_5", text_help_5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text_help_5);
  gtk_container_add (GTK_CONTAINER (scrw_help_5), text_help_5);
  gtk_tooltips_set_tip (tooltips, text_help_5, _("Left Side English \nRight Side German"), NULL);
  gtk_text_set_editable (GTK_TEXT (text_help_5), FALSE);
//  textinsert("/usr/share/hf/???", GTK_TEXT(text_help_5));
// 

  label_help_5 = gtk_label_new (_(""));
  gtk_widget_set_name (label_help_5, "label_help_5");
  gtk_widget_ref (label_help_5);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "label_help_5", label_help_5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_help_5);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (helpnotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (helpnotebook), 4), label_help_5);


/* - Hilfe 6  -----------------------------------------------------*/

  frame_help_6 = gtk_frame_new (_("Deutsches HF-HOWTO"));
  gtk_widget_set_name (frame_help_6, "frame_help_6");
  gtk_widget_ref (frame_help_6);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "frame_help_6", frame_help_6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame_help_6);
  gtk_container_add (GTK_CONTAINER (helpnotebook), frame_help_6);
  gtk_container_set_border_width (GTK_CONTAINER (frame_help_6), 3);

  scrw_help_6 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrw_help_6, "scrw_help_6");
  gtk_widget_ref (scrw_help_6);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "scrw_help_6", scrw_help_6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrw_help_6);
  gtk_container_add (GTK_CONTAINER (frame_help_6), scrw_help_6);
  gtk_container_set_border_width (GTK_CONTAINER (scrw_help_6), 3);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrw_help_6), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  text_help_6 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text_help_6, "text_help_6");
  gtk_widget_ref (text_help_6);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "text_help_6", text_help_6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text_help_6);
  gtk_container_add (GTK_CONTAINER (scrw_help_6), text_help_6);
  gtk_tooltips_set_tip (tooltips, text_help_6, _("Left Side English \nRight Side German"), NULL);
  gtk_text_set_editable (GTK_TEXT (text_help_6), FALSE);

  showhelp("DE-HF-HOWTO.txt", text_help_6);

  label_help_6 = gtk_label_new (_("DE-HF-HOWTO"));
  gtk_widget_set_name (label_help_6, "label_help_6");
  gtk_widget_ref (label_help_6);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "label_help_6", label_help_6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_help_6);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (helpnotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (helpnotebook), 5), label_help_6);

/* - Hilfe 7 ------------------------------------------------------*/

  frame_help_7 = gtk_frame_new (_(""));
  gtk_widget_set_name (frame_help_7, "frame_help_7");
  gtk_widget_ref (frame_help_7);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "frame_help_7", frame_help_7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame_help_7);
  gtk_container_add (GTK_CONTAINER (helpnotebook), frame_help_7);
  gtk_container_set_border_width (GTK_CONTAINER (frame_help_7), 3);

  scrw_help_7 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrw_help_7, "scrw_help_7");
  gtk_widget_ref (scrw_help_7);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "scrw_help_7", scrw_help_7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrw_help_7);
  gtk_container_add (GTK_CONTAINER (frame_help_7), scrw_help_7);
  gtk_container_set_border_width (GTK_CONTAINER (scrw_help_7), 3);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrw_help_7), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  text_help_7 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text_help_7, "text_help_7");
  gtk_widget_ref (text_help_7);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "text_help_7", text_help_7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text_help_7);
  gtk_container_add (GTK_CONTAINER (scrw_help_7), text_help_7);
  gtk_tooltips_set_tip (tooltips, text_help_7, _("Left Side English \nRight Side German\n hope more to come !!"), NULL);
  gtk_text_set_editable (GTK_TEXT (text_help_7), FALSE);
//  textinsert("/usr/share/hf/hfkernel-de.txt", GTK_TEXT(text_help_7));

  label_help_7 = gtk_label_new (_(""));
  gtk_widget_set_name (label_help_7, "label_help_7");
  gtk_widget_ref (label_help_7);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "label_help_7", label_help_7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_help_7);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (helpnotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (helpnotebook), 6), label_help_7);

/* - Hilfe 8 ------------------------------------------------------*/

  frame_help_8 = gtk_frame_new (_("Jost (ZS5S)'s Pactor Mailbox List"));
  gtk_widget_set_name (frame_help_8, "frame_help_8");
  gtk_widget_ref (frame_help_8);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "frame_help_8", frame_help_8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame_help_8);
  gtk_container_add (GTK_CONTAINER (helpnotebook), frame_help_8);
  gtk_container_set_border_width (GTK_CONTAINER (frame_help_8), 3);

  scrw_help_8 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrw_help_8, "scrw_help_8");
  gtk_widget_ref (scrw_help_8);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "scrw_help_8", scrw_help_8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrw_help_8);
  gtk_container_add (GTK_CONTAINER (frame_help_8), scrw_help_8);
  gtk_container_set_border_width (GTK_CONTAINER (scrw_help_8), 3);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrw_help_8), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  text_help_8 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text_help_8, "text_help_8");
  gtk_widget_ref (text_help_8);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "text_help_8", text_help_8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text_help_8);
  gtk_container_add (GTK_CONTAINER (scrw_help_8), text_help_8);
  gtk_tooltips_set_tip (tooltips, text_help_8, _("This mailbox list is provided monthly, \nyou can subscribe it via e-mail or packet.\n"), NULL);
  gtk_text_set_editable (GTK_TEXT (text_help_8), FALSE);

  showhelp("P-MB-LIST.TXT", text_help_8);

  label_help_8 = gtk_label_new (_("Pactor Mailbox List"));
  gtk_widget_set_name (label_help_8, "label_help_8");
  gtk_widget_ref (label_help_8);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "label_help_8", label_help_8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_help_8);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (helpnotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (helpnotebook), 7), label_help_8);

/* - Hilfe 9 ------------------------------------------------------*/

  frame_help_9 = gtk_frame_new (_("�er das Pactor-Protokoll"));
  gtk_widget_set_name (frame_help_9, "frame_help_9");
  gtk_widget_ref (frame_help_9);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "frame_help_9", frame_help_9,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame_help_9);
  gtk_container_add (GTK_CONTAINER (helpnotebook), frame_help_9);
  gtk_container_set_border_width (GTK_CONTAINER (frame_help_9), 3);

  scrw_help_9 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrw_help_9, "scrw_help_9");
  gtk_widget_ref (scrw_help_9);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "scrw_help_9", scrw_help_9,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrw_help_9);
  gtk_container_add (GTK_CONTAINER (frame_help_9), scrw_help_9);
  gtk_container_set_border_width (GTK_CONTAINER (scrw_help_9), 3);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrw_help_9), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  text_help_9 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text_help_9, "text_help_9");
  gtk_widget_ref (text_help_9);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "text_help_9", text_help_9,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text_help_9);
  gtk_container_add (GTK_CONTAINER (scrw_help_9), text_help_9);
  gtk_tooltips_set_tip (tooltips, text_help_9, _("Left Side English \nRight German"), NULL);
  gtk_text_set_editable (GTK_TEXT (text_help_9), FALSE);

  showhelp("pactor.txt", text_help_9);

  label_help_9 = gtk_label_new (_("PACTOR spec"));
  gtk_widget_set_name (label_help_9, "label_help_9");
  gtk_widget_ref (label_help_9);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "label_help_9", label_help_9,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_help_9);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (helpnotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (helpnotebook), 8), label_help_9);

/* - Hilfe 10 -----------------------------------------------------*/

  frame_help_10 = gtk_frame_new (_("PROGRAMMIERERLATEIN (DAS UNTERE ENDE IST DEUTSCH)"));
  gtk_widget_set_name (frame_help_10, "frame_help_10");
  gtk_widget_ref (frame_help_10);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "frame_help_10", frame_help_10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame_help_10);
  gtk_container_add (GTK_CONTAINER (helpnotebook), frame_help_10);
  gtk_container_set_border_width (GTK_CONTAINER (frame_help_10), 3);

  scrw_help_10 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrw_help_10, "scrw_help_10");
  gtk_widget_ref (scrw_help_10);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "scrw_help_10", scrw_help_10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrw_help_10);
  gtk_container_add (GTK_CONTAINER (frame_help_10), scrw_help_10);
  gtk_container_set_border_width (GTK_CONTAINER (scrw_help_10), 3);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrw_help_10), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  text_help_10 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text_help_10, "text_help_10");
  gtk_widget_ref (text_help_10);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "text_help_10", text_help_10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text_help_10);
  gtk_container_add (GTK_CONTAINER (scrw_help_10), text_help_10);
//  gtk_tooltips_set_tip (tooltips, text_help_10, _("Left Side English \nRight German"), NULL);
  gtk_text_set_editable (GTK_TEXT (text_help_10), FALSE);
//  textinsert("/usr/share/hf/TODO", GTK_TEXT(text_help_10));

  label_help_10 = gtk_label_new (_(""));
  gtk_widget_set_name (label_help_10, "label_help_10");
  gtk_widget_ref (label_help_10);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "label_help_10", label_help_10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_help_10);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (helpnotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (helpnotebook), 9), label_help_10);

/* - Hilfe 11 -----------------------------------------------------*/

  frame_help_11 = gtk_frame_new (_("HILFE AUF PLATTDEUTSCH IN VORBEREITUNG"));
  gtk_widget_set_name (frame_help_11, "frame_help_11");
  gtk_widget_ref (frame_help_11);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "frame_help_11", frame_help_11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame_help_11);
  gtk_container_add (GTK_CONTAINER (helpnotebook), frame_help_11);
  gtk_container_set_border_width (GTK_CONTAINER (frame_help_11), 3);

  scrw_help_11 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrw_help_11, "scrw_help_11");
  gtk_widget_ref (scrw_help_11);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "scrw_help_11", scrw_help_11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrw_help_11);
  gtk_container_add (GTK_CONTAINER (frame_help_11), scrw_help_11);
  gtk_container_set_border_width (GTK_CONTAINER (scrw_help_11), 3);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrw_help_11), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  text_help_11 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text_help_11, "text_help_11");
  gtk_widget_ref (text_help_11);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "text_help_11", text_help_11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text_help_11);
  gtk_container_add (GTK_CONTAINER (scrw_help_11), text_help_11);
  gtk_tooltips_set_tip (tooltips, text_help_11, _("Left Side English \nRight German"), NULL);
  gtk_text_set_editable (GTK_TEXT (text_help_11), FALSE);
//  textinsert("/usr/share/hf/???", GTK_TEXT(text_help_11));
  gtk_text_insert(GTK_TEXT (text_help_11), NULL, NULL, NULL, "Das`n ganz annern Schnack !", -1);

  label_help_11 = gtk_label_new (_("PLATT ?"));
  gtk_widget_set_name (label_help_11, "label_help_11");
  gtk_widget_ref (label_help_11);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "label_help_11", label_help_11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_help_11);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (helpnotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (helpnotebook), 10), label_help_11);

/* - Hilfe 12  ----------------------------------------------------*/

  frame_help_12 = gtk_frame_new (_("Translations welcome, �ersetzungen willkommen!"));
  gtk_widget_set_name (frame_help_12, "frame_help_12");
  gtk_widget_ref (frame_help_12);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "frame_help_12", frame_help_12,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame_help_12);
  gtk_container_add (GTK_CONTAINER (helpnotebook), frame_help_12);
  gtk_container_set_border_width (GTK_CONTAINER (frame_help_12), 3);

  scrw_help_12 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrw_help_12, "scrw_help_12");
  gtk_widget_ref (scrw_help_12);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "scrw_help_12", scrw_help_12,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrw_help_12);
  gtk_container_add (GTK_CONTAINER (frame_help_12), scrw_help_12);
  gtk_container_set_border_width (GTK_CONTAINER (scrw_help_12), 3);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrw_help_12), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  text_help_12 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text_help_12, "text_help_12");
  gtk_widget_ref (text_help_12);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "text_help_12", text_help_12,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text_help_12);
  gtk_container_add (GTK_CONTAINER (scrw_help_12), text_help_12);
  gtk_tooltips_set_tip (tooltips, text_help_12, _("Left Side English \nRight German"), NULL);
  gtk_text_set_editable (GTK_TEXT (text_help_12), FALSE);
//  textinsert("/usr/share/hf/hfkernel-de.txt", GTK_TEXT(text_help_12));
  gtk_text_insert(GTK_TEXT (text_help_12), NULL, NULL, NULL, "Natamanna an yakun hadha 'lbarnamigh nafi'un li muhibbun arradio fi kulli 'l alam,\nman yutargim al musa'adat lana ?\nshukran!!! wa tahiat minna'l katibeen", -1);

  label_help_12 = gtk_label_new (_("arabic ..."));
  gtk_widget_set_name (label_help_12, "label_help_12");
  gtk_widget_ref (label_help_12);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "label_help_12", label_help_12,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_help_12);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (helpnotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (helpnotebook), 11), label_help_12);

// Hilfe Ende

  hbox17 = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (hbox17, "hbox17");
  gtk_widget_ref (hbox17);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "hbox17", hbox17,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox17);
  gtk_box_pack_start (GTK_BOX (vbox11), hbox17, FALSE, FALSE, 3);

  button17 = gtk_button_new_with_label (_("Ok & Cancel"));
  gtk_widget_set_name (button17, "button17");
  gtk_widget_ref (button17);
  gtk_object_set_data_full (GTK_OBJECT (whilfe), "button17", button17,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button17);
  gtk_box_pack_start (GTK_BOX (hbox17), button17, TRUE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (button17), 3);

  gtk_signal_connect (GTK_OBJECT (button17), "clicked",
                      GTK_SIGNAL_FUNC (on_button17_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (whilfe), "delete_event",
                      GTK_SIGNAL_FUNC (gtk_widget_hide),
                      NULL);
  gtk_widget_add_accelerator (button17, "clicked",
  			help_accels,
			GDK_Return, 0, 
      	                GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator(whilfe, "hide", help_accels, 
  			      GDK_h, GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);  
  gtk_widget_add_accelerator(whilfe, "hide", help_accels, 
  			      GDK_Escape, 0,
                              GTK_ACCEL_VISIBLE);  
  
  gtk_object_set_data (GTK_OBJECT (whilfe), "tooltips", tooltips);

  return whilfe;
}

