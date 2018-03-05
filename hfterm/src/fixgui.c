# include "hft.h"

/*
 *  interface for fixtext window
 */

//helptext fixmagic defined in rxtx.c

GtkWidget*
create_Wfixtext (void)
{
  GtkWidget *Wfixtext;
  GtkWidget *vbox_Fixtext;
  GtkWidget *Fixtext_frame2;
  GtkWidget *notebook_Fixtext;

  GtkWidget *scrolledwindow1;
  GtkWidget *text1;
  GtkWidget *label1;
  
  GtkWidget *scrolledwindow2;
  GtkWidget *text2;
  GtkWidget *label2;
  
  GtkWidget *scrolledwindow3;
  GtkWidget *text3;
  GtkWidget *label3;
  
  GtkWidget *scrolledwindow4;
  GtkWidget *text4;
  GtkWidget *label4;
  
  GtkWidget *scrolledwindow5;
  GtkWidget *text5;
  GtkWidget *label5;
  
  GtkWidget *scrolledwindow6;
  GtkWidget *text6;
  GtkWidget *label6;
  
  GtkWidget *scrolledwindow7;
  GtkWidget *text7;
  GtkWidget *label7;
  
  GtkWidget *scrolledwindow8;
  GtkWidget *text8;
  GtkWidget *label8;
  
  GtkWidget *scrolledwindow9;
  GtkWidget *text9;
  GtkWidget *label9;
  
  GtkWidget *scrolledwindow10;
  GtkWidget *text10;
  GtkWidget *label10;
  
  GtkWidget *scrolledwindow11;
  GtkWidget *text11;
  GtkWidget *label11;
  
  GtkWidget *scrolledwindow12;
  GtkWidget *text12;
  GtkWidget *label12;
  
  GtkWidget *fixbutton_hbox;
  GtkWidget *fixhelptext;
  GtkWidget *fixhelpwin;
  GtkWidget *fixhelpframe;
  
  GtkWidget *button_Fixtext_OK;
  GtkWidget *button_Fixtext_Cancel;
  GtkTooltips *tooltips;
  GtkAccelGroup *fix_accels;
  tooltips = gtk_tooltips_new ();

  Wfixtext = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (Wfixtext, "Wfixtext");
  gtk_object_set_data (GTK_OBJECT (Wfixtext), "Wfixtext", Wfixtext);
  gtk_tooltips_set_tip (tooltips, Wfixtext, _("Here you can enter some fixtexts you need ..."), NULL);
  gtk_window_set_title (GTK_WINDOW (Wfixtext), _("Text macros"));
  gtk_window_set_position (GTK_WINDOW (Wfixtext), GTK_WIN_POS_CENTER);
  gtk_window_set_policy (GTK_WINDOW (Wfixtext), FALSE, TRUE, TRUE);
  gtk_window_set_wmclass (GTK_WINDOW (Wfixtext), "hfterm", "hfterm");
  fix_accels = gtk_accel_group_new ();
  gtk_accel_group_attach(fix_accels, GTK_OBJECT(Wfixtext));


  vbox_Fixtext = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (vbox_Fixtext, "vbox_Fixtext");
  gtk_widget_ref (vbox_Fixtext);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "vbox_Fixtext", vbox_Fixtext,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox_Fixtext);
  gtk_container_add (GTK_CONTAINER (Wfixtext), vbox_Fixtext);

  Fixtext_frame2 = gtk_frame_new (_("Enter Fixtext"));
  gtk_widget_set_name (Fixtext_frame2, "Fixtext_frame2");
  gtk_widget_ref (Fixtext_frame2);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "Fixtext_frame2", Fixtext_frame2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (Fixtext_frame2);
  gtk_box_pack_start (GTK_BOX (vbox_Fixtext), Fixtext_frame2, TRUE, TRUE, 3);
  gtk_container_set_border_width (GTK_CONTAINER (Fixtext_frame2), 3);

  notebook_Fixtext = gtk_notebook_new ();
  gtk_widget_set_name (notebook_Fixtext, "notebook_Fixtext");
  gtk_widget_ref (notebook_Fixtext);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "notebook_Fixtext", notebook_Fixtext,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (notebook_Fixtext);
  gtk_container_add (GTK_CONTAINER (Fixtext_frame2), notebook_Fixtext);
  gtk_widget_set_usize (notebook_Fixtext, 750, 325);
  gtk_notebook_set_tab_hborder (GTK_NOTEBOOK (notebook_Fixtext), 7);
  gtk_notebook_set_tab_vborder (GTK_NOTEBOOK (notebook_Fixtext), 3);

/* - Fixtext  1 ----------------------------------------------------*/
    
  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow1, "scrolledwindow1");
  gtk_widget_ref (scrolledwindow1);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "scrolledwindow1", scrolledwindow1,
  (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow1);
  gtk_container_add (GTK_CONTAINER (notebook_Fixtext), scrolledwindow1);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  text1 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text1, "text1");
  gtk_widget_ref (text1);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "text1", text1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text1);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), text1);
  gtk_text_set_editable (GTK_TEXT (text1), TRUE);
/*
//	example:
	gtk_text_insert (GTK_TEXT (text1), NULL, NULL, NULL,
        _("CQ CQ CQ de [MYCALL] [MYCALL] [MYCALL] pse k"), -1);
*/
  label1 = gtk_label_new (_("1 cq"));
  gtk_widget_set_name (label1, "label1");
  gtk_widget_ref (label1);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "label1", label1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label1);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_Fixtext), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_Fixtext), 0), label1);

/* - Fixtext  2 ----------------------------------------------------*/

  scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow2, "scrolledwindow2");
  gtk_widget_ref (scrolledwindow2);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "scrolledwindow2", scrolledwindow2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow2);
  gtk_container_add (GTK_CONTAINER (notebook_Fixtext), scrolledwindow2);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  text2 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text2, "text2");
  gtk_widget_ref (text2);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "text2", text2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text2);
  gtk_container_add (GTK_CONTAINER (scrolledwindow2), text2);
  gtk_text_set_editable (GTK_TEXT (text2), TRUE);

  label2 = gtk_label_new (_("2 qrz?"));
  gtk_widget_set_name (label2, "label2");
  gtk_widget_ref (label2);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "label2", label2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label2);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_Fixtext), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_Fixtext), 1), label2);


/* - Fixtext 3 ----------------------------------------------------*/
  
  scrolledwindow3 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow3, "scrolledwindow3");
  gtk_widget_ref (scrolledwindow3);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "scrolledwindow3", scrolledwindow3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow3);
  gtk_container_add (GTK_CONTAINER (notebook_Fixtext), scrolledwindow3);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow3), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  text3 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text3, "text3");
  gtk_widget_ref (text3);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "text3", text3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text3);
  gtk_container_add (GTK_CONTAINER (scrolledwindow3), text3);
  gtk_text_set_editable (GTK_TEXT (text3), TRUE);

  label3 = gtk_label_new (_("3 de"));
  gtk_widget_set_name (label3, "label3");
  gtk_widget_ref (label3);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "label3", label3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label3);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_Fixtext), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_Fixtext), 2), label3);

/* - Fixtext  4 ----------------------------------------------------*/
  
  scrolledwindow4 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow4, "scrolledwindow4");
  gtk_widget_ref (scrolledwindow4);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "scrolledwindow4", scrolledwindow4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow4);
  gtk_container_add (GTK_CONTAINER (notebook_Fixtext), scrolledwindow4);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow4), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  text4 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text4, "text4");
  gtk_widget_ref (text4);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "text4", text4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text4);
  gtk_container_add (GTK_CONTAINER (scrolledwindow4), text4);
  gtk_text_set_editable (GTK_TEXT (text4), TRUE);

  label4 = gtk_label_new (_("4 brag"));
  gtk_widget_set_name (label4, "label4");
  gtk_widget_ref (label4);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "label4", label4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label4);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_Fixtext), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_Fixtext), 3), label4);

/* - Fixtext  5 ----------------------------------------------------*/  
  
  scrolledwindow5 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow5, "scrolledwindow5");
  gtk_widget_ref (scrolledwindow5);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "scrolledwindow5", scrolledwindow5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow5);
  gtk_container_add (GTK_CONTAINER (notebook_Fixtext), scrolledwindow5);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow5), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  text5 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text5, "text5");
  gtk_widget_ref (text5);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "text5", text5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text5);
  gtk_container_add (GTK_CONTAINER (scrolledwindow5), text5);
  gtk_text_set_editable (GTK_TEXT (text5), TRUE);

  label5 = gtk_label_new (_("5 prog"));
  gtk_widget_set_name (label5, "label5");
  gtk_widget_ref (label5);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "label5", label5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label5);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_Fixtext), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_Fixtext), 4), label5);

/* - Fixtext  6 ----------------------------------------------------*/  
  
  scrolledwindow6 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow6, "scrolledwindow6");
  gtk_widget_ref (scrolledwindow6);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "scrolledwindow6", scrolledwindow6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow6);
  gtk_container_add (GTK_CONTAINER (notebook_Fixtext), scrolledwindow6);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow6), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  text6 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text6, "text6");
  gtk_widget_ref (text6);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "text6", text6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text6);
  gtk_container_add (GTK_CONTAINER (scrolledwindow6), text6);
  gtk_text_set_editable (GTK_TEXT (text6), TRUE);
  
  label6 = gtk_label_new (_("6 qsl"));
  gtk_widget_set_name (label6, "label6");
  gtk_widget_ref (label6);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "label6", label6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label6);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_Fixtext), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_Fixtext), 5), label6);

/* - Fixtext 7 ------------------------------------------------------*/  
  
  scrolledwindow7 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow7, "scrolledwindow7");
  gtk_widget_ref (scrolledwindow7);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "scrolledwindow7", scrolledwindow7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow7);
  gtk_container_add (GTK_CONTAINER (notebook_Fixtext), scrolledwindow7);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow7), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  text7 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text7, "text7");
  gtk_widget_ref (text7);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "text7", text7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text7);
  gtk_container_add (GTK_CONTAINER (scrolledwindow7), text7);
  gtk_text_set_editable (GTK_TEXT (text7), TRUE);
  
  label7 = gtk_label_new (_("7 test"));
  gtk_widget_set_name (label7, "label7");
  gtk_widget_ref (label7);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "label7", label7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label7);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_Fixtext), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_Fixtext), 6), label7);

/* - Fixtext  8 ----------------------------------------------------*/  
  
  scrolledwindow8 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow8, "scrolledwindow8");
  gtk_widget_ref (scrolledwindow8);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "scrolledwindow8", scrolledwindow8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow8);
  gtk_container_add (GTK_CONTAINER (notebook_Fixtext), scrolledwindow8);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow8), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
  
  text8 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text8, "text8");
  gtk_widget_ref (text8);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "text8", text8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text8);
  gtk_container_add (GTK_CONTAINER (scrolledwindow8), text8);
  gtk_text_set_editable (GTK_TEXT (text8), TRUE);
  
  label8 = gtk_label_new (_("8 time"));
  gtk_widget_set_name (label8, "label8");
  gtk_widget_ref (label8);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "label8", label8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label8);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_Fixtext), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_Fixtext), 7), label8);

/* - Fixtext 9 ------------------------------------------------------*/  
 
  scrolledwindow9 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow9, "scrolledwindow9");
  gtk_widget_ref (scrolledwindow9);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "scrolledwindow9", scrolledwindow9,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow9);
  gtk_container_add (GTK_CONTAINER (notebook_Fixtext), scrolledwindow9);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow9), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);


  text9 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text9, "text9");
  gtk_widget_ref (text9);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "text9", text9,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text9);
  gtk_container_add (GTK_CONTAINER (scrolledwindow9), text9);
  gtk_text_set_editable (GTK_TEXT (text9), TRUE);

  label9 = gtk_label_new (_("9 contest"));
  gtk_widget_set_name (label9, "label9");
  gtk_widget_ref (label9);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "label9", label9,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label9);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_Fixtext), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_Fixtext), 8), label9);

/* - Fixtext 10 ----------------------------------------------------*/  
  
  scrolledwindow10 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow10, "scrolledwindow10");
  gtk_widget_ref (scrolledwindow10);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "scrolledwindow10", scrolledwindow10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow10);
  gtk_container_add (GTK_CONTAINER (notebook_Fixtext), scrolledwindow10);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow10), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
 
  text10 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text10, "text10");
  gtk_widget_ref (text10);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "text10", text10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text10);
  gtk_container_add (GTK_CONTAINER (scrolledwindow10), text10);
  gtk_text_set_editable (GTK_TEXT (text10), TRUE);

  label10 = gtk_label_new (_("10 mailbox"));
  gtk_widget_set_name (label10, "label10");
  gtk_widget_ref (label10);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "label10", label10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label10);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_Fixtext), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_Fixtext), 9), label10);

/* - Fixtext 11 ----------------------------------------------------*/  

  scrolledwindow11 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow11, "scrolledwindow11");
  gtk_widget_ref (scrolledwindow11);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "scrolledwindow11", scrolledwindow11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow11);
  gtk_container_add (GTK_CONTAINER (notebook_Fixtext), scrolledwindow11);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow11), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  text11 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text11, "text11");
  gtk_widget_ref (text11);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "text11", text11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text11);
  gtk_container_add (GTK_CONTAINER (scrolledwindow11), text11);
  gtk_text_set_editable (GTK_TEXT (text11), TRUE);
  
  label11 = gtk_label_new (_("11 bye"));
  gtk_widget_set_name (label11, "label11");
  gtk_widget_ref (label11);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "label11", label11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label11);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_Fixtext), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_Fixtext), 10), label11);

/* - Fixtext 12 ----------------------------------------------------*/  
  
  scrolledwindow12 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow12, "scrolledwindow12");
  gtk_widget_ref (scrolledwindow12);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "scrolledwindow12", scrolledwindow12,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow12);
  gtk_container_add (GTK_CONTAINER (notebook_Fixtext), scrolledwindow12);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow12), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  text12 = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (text12, "text12");
  gtk_widget_ref (text12);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "text12", text12,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text12);
  gtk_container_add (GTK_CONTAINER (scrolledwindow12), text12);
  gtk_text_set_editable (GTK_TEXT (text12), TRUE);
  
  label12 = gtk_label_new (_("12 fun"));
  gtk_widget_set_name (label12, "label12");
  gtk_widget_ref (label12);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "label12", label12,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label12);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_Fixtext), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_Fixtext), 11), label12);

/* - Fixtexte Ende ----------------------------------------------------*/  
  
  fixhelpframe = gtk_frame_new (_("Your magic keys:"));
  gtk_widget_set_name (fixhelpframe, "fixhelpframe");
  gtk_widget_ref (fixhelpframe);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "fixhelpframe", fixhelpframe,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fixhelpframe);
  gtk_box_pack_start (GTK_BOX (vbox_Fixtext), fixhelpframe, TRUE, TRUE, 3);

  fixhelpwin = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (fixhelpwin, "fixhelpwin");
  gtk_widget_ref (fixhelpwin);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "fixhelpwin", fixhelpwin,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fixhelpwin);
  gtk_container_add (GTK_CONTAINER (fixhelpframe), fixhelpwin);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (fixhelpwin), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
  
  fixhelptext = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (fixhelptext, "fixhelptext");
  gtk_widget_ref (fixhelptext);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "fixhelptext", fixhelptext,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fixhelptext);
  gtk_container_add (GTK_CONTAINER (fixhelpwin), fixhelptext);
  gtk_text_set_editable (GTK_TEXT (fixhelptext), FALSE);
  gtk_text_insert (GTK_TEXT (fixhelptext), radiofont, NULL, NULL, fixmagic, -1);

  gtk_widget_show (fixhelptext);
 
  
  fixbutton_hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (fixbutton_hbox, "fixbutton_hbox");
  gtk_widget_ref (fixbutton_hbox);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "fixbutton_hbox", fixbutton_hbox,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fixbutton_hbox);
  gtk_box_pack_start (GTK_BOX (vbox_Fixtext), fixbutton_hbox, FALSE, FALSE, 3);

  button_Fixtext_OK = gtk_button_new_with_label (_("OK and Save"));
  gtk_widget_set_name (button_Fixtext_OK, "button_Fixtext_OK");
  gtk_widget_ref (button_Fixtext_OK);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "button_Fixtext_OK", button_Fixtext_OK,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button_Fixtext_OK);
  gtk_box_pack_start (GTK_BOX (fixbutton_hbox), button_Fixtext_OK, FALSE, FALSE, 3);

  button_Fixtext_Cancel = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_set_name (button_Fixtext_Cancel, "button_Fixtext_Cancel");
  gtk_widget_ref (button_Fixtext_Cancel);
  gtk_object_set_data_full (GTK_OBJECT (Wfixtext), "button_Fixtext_Cancel", button_Fixtext_Cancel,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button_Fixtext_Cancel);
  gtk_box_pack_start (GTK_BOX (fixbutton_hbox), button_Fixtext_Cancel, FALSE, FALSE, 3);

  gtk_signal_connect (GTK_OBJECT (button_Fixtext_OK), "clicked",
                      GTK_SIGNAL_FUNC (on_button_Fixtext_OK_clicked),
                      NULL);
  gtk_widget_add_accelerator (button_Fixtext_OK, "clicked",
  			fix_accels,
			GDK_Return, 0, 
      	                GTK_ACCEL_VISIBLE);
   
  gtk_signal_connect (GTK_OBJECT (button_Fixtext_Cancel), "clicked",
                      GTK_SIGNAL_FUNC (on_button_Fixtext_Cancel_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (Wfixtext), "delete_event",
                      GTK_SIGNAL_FUNC (on_button_Fixtext_Cancel_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (Wfixtext), "destroy_event",
                      GTK_SIGNAL_FUNC (on_button_Fixtext_Cancel_clicked),
                      NULL);

  gtk_widget_add_accelerator(Wfixtext, "hide", fix_accels, 
  			      GDK_t, GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);  
  gtk_widget_add_accelerator(Wfixtext, "hide", fix_accels, 
  			      GDK_Escape, 0,
                              GTK_ACCEL_VISIBLE);  
  gtk_object_set_data (GTK_OBJECT (Wfixtext), "tooltips", tooltips);

  return Wfixtext;
}

