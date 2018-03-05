# include "gui.h"

/*
 *  interface for parameter window
 */
GtkWidget*
create_wpar (void)
{
  GtkWidget *wpar;
  GtkWidget *param_vbox;
  GtkWidget *paramnotebook;

  GtkWidget *label_brag;   
  GtkWidget *table8;
  GtkWidget *label65;
  GtkWidget *label66;
  GtkWidget *label67;
  GtkWidget *label68;
  GtkWidget *label69;
  GtkWidget *label70;
  GtkWidget *label71;
  GtkWidget *label72;
  GtkWidget *hseparator4;
  GtkWidget *hseparator5;
  GtkWidget *entry28;
  GtkWidget *entry29;
  GtkWidget *entry30;
  GtkWidget *entry31;
  GtkWidget *entry32;
  GtkWidget *entry33;
  GtkWidget *entry34;
  GtkWidget *entry27;
/*
  GtkWidget *hseparator6;
  GtkWidget *hseparator8;
  GtkWidget *hseparator9;
  GtkWidget *hseparator10;
  GtkWidget *hseparator11;
  GtkWidget *hseparator12;
  GtkWidget *hseparator13;
*/
  GtkWidget *label_general;
  GtkWidget *table_general;
  
  GtkWidget *label_soundcorr;
  GtkWidget *soundcorr;
  
  GtkWidget *label_timecorr;
  GtkWidget *timecorr;
  
  GtkWidget *label_cpumhzcorr;
  GtkWidget *cpumhzcorr;
  
  GtkWidget *label_soundcard;
  GtkWidget *soundcard;
  GList *soundcard_items = NULL;
  GtkWidget *soundcardvalue;
  
  GtkWidget *label_serial;
  GtkWidget *serial;
  GList *serial_items = NULL;
  GtkWidget *serialvalue;
  
  GtkWidget *label_kerneloptions;
  GtkWidget *kerneloptions;
  
  GtkWidget *label_beaconpause;
  GtkWidget *beaconpause;
  GList *beaconpause_items = NULL;
  GtkWidget *beaconpausevalue;
  
  GtkWidget *label_squelch;
  GtkWidget *squelchpercent;
    
  GtkWidget *table_fsk;
  GtkWidget *label_mark;
  GtkWidget *label_space;
  GtkWidget *fskmark;
  GList *fskmark_items = NULL;
  GtkWidget *fskmarkfreq;
  GtkWidget *fskspace;
  GList *fskspace_items = NULL;
  GtkWidget *fskspacefreq;
  GtkWidget *label_FSK;

  GtkWidget *table_cw;
  GtkWidget *label_wpm;
  GtkWidget *wpm;
  GtkWidget *label_tone;
  GtkWidget *tone;
  GtkWidget *farnsworth;
  GtkWidget *dtr;
  GtkWidget *label_CW;

  GtkWidget *table_rtty;
  GtkWidget *label_baudrate;
  GtkWidget *rttyinvert;
  GtkWidget *rttyrxtxinvert;
  GtkWidget *rttybaudrate;
  GtkWidget *label_RTTY;

  GtkWidget *table_amtor;
  GtkWidget *label_amtor_retries;
  GtkWidget *label_amtor_txdelay;
  GtkWidget *label_amtor_mycall;
  GtkWidget *label_amtor_selfeccall;
  GtkWidget *label_amtor_destcall;
  GtkWidget *amtorrxtxinvert;
  GtkWidget *amtorinvert;
  GtkWidget *amtordestcall;
  GtkWidget *amtorselfeccall;
  GtkWidget *amtormycall;
  GtkWidget *amtortxdelay;
  GtkWidget *amtorretry;
  GtkWidget *label_amtor;

  GtkWidget *table_Gtor;
  GtkWidget *gtordestcall;
//  GtkWidget *gtormycall;
  GtkWidget *gtortxdelay;
  GtkWidget *gtorretry;
  GtkWidget *label_gtor_destcall;
//  GtkWidget *label_gtor_mycall;
  GtkWidget *label_gtor_txdelay;
  GtkWidget *label_gtor_retries;
  GtkWidget *label_Gtor;

  GtkWidget *label_Pactor;
  GtkWidget *hbox_pactor_standard;
  GtkWidget *frame_pactor_standard;
  GtkWidget *table_pactor_standard;
//  GtkWidget *label_pactor_mycall;
  GtkWidget *label_pactor_txdelay;
  GtkWidget *label_pactor_retries;
  GtkWidget *pactorlongpath;
  GtkWidget *pactorcall;
//  GtkWidget *pactormycall;
  GtkWidget *pactortxdelay;
  GtkWidget *pactorretry;
  GtkWidget *label_pactor_destcall;
  GtkWidget *label_pactor_splp;

  GtkWidget *frame_pactor_advanced;
  GtkWidget *table_pactor_advanced;
  GtkWidget *pactorcrc0;
  GtkWidget *pactorcrc1;
  GtkWidget *pactorcrc3;
  GtkWidget *pactorcrc2;
  GtkWidget *label_pactor_crc100chg;
  GtkWidget *label_pactor_crc100;
  GtkWidget *label_pactor_crc200chg;
  GtkWidget *label_pactor_crc200;
  
  GtkWidget *table_mt63;
  GtkWidget *label_mt63;
  GtkWidget *label_mt63_bandwidth;
  GtkWidget *mt63_bandwidth;
  GList *mt63_bandwidth_items = NULL;
  GtkWidget *mt63_bandwidth_value;
  GtkWidget *label_mt63_integration;
  GtkWidget *mt63_integration;
  GtkWidget *label_mt63_cwcall;
  GtkWidget *mt63_cwcall;
  GtkWidget *mt63_doubleinterleave;

  GtkWidget *table_mailbox;
  GtkWidget *label_mailbox;
  GtkWidget *label_mailbox_host;
  GtkWidget *mailbox_host;
  GtkWidget *label_mailbox_port;
  GtkWidget *mailbox_port;

  GtkWidget *toolbar1;
  GtkWidget *parok;
  GtkWidget *parcancel;
  GtkAccelGroup *par_accels;
  
  GtkTooltips *tooltips;
  tooltips = gtk_tooltips_new ();  
  
  wpar = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (wpar, "wpar");
  gtk_object_set_data (GTK_OBJECT (wpar), "wpar", wpar);
  gtk_window_set_title (GTK_WINDOW (wpar), _("Parameters"));
  gtk_window_set_policy (GTK_WINDOW (wpar), FALSE, TRUE, TRUE);
  gtk_window_set_wmclass (GTK_WINDOW (wpar), "hfterm", "hfterm");

  par_accels = gtk_accel_group_new ();
  gtk_accel_group_attach(par_accels, GTK_OBJECT(wpar));
  
  param_vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (param_vbox, "param_vbox");
  gtk_widget_ref (param_vbox);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "param_vbox", param_vbox,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (param_vbox);
  gtk_container_add (GTK_CONTAINER (wpar), param_vbox);

  paramnotebook = gtk_notebook_new ();
  gtk_widget_set_name (paramnotebook, "paramnotebook");
  gtk_widget_ref (paramnotebook);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "paramnotebook", paramnotebook,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (paramnotebook);
  gtk_box_pack_start (GTK_BOX (param_vbox), paramnotebook, TRUE, TRUE, 0);

/* brag */

  table8 = gtk_table_new (17, 2, FALSE);
  gtk_widget_set_name (table8, "table8");
  gtk_widget_ref (table8);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "table8", table8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table8);
  gtk_container_add (GTK_CONTAINER (paramnotebook), table8);
  gtk_table_set_row_spacings (GTK_TABLE (table8), 1);
  gtk_table_set_col_spacings (GTK_TABLE (table8), 3);

  label_brag = gtk_label_new (_("Station Data"));
  gtk_widget_set_name (label_brag, "label_brag");
  gtk_widget_ref (label_brag);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_brag", label_brag,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_brag);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (paramnotebook), 
    gtk_notebook_get_nth_page (GTK_NOTEBOOK (paramnotebook), 0), label_brag);

  label65 = gtk_label_new (_("Callsign"));
  gtk_widget_set_name (label65, "label65");
  gtk_widget_ref (label65);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label65", label65,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label65);
  gtk_table_attach (GTK_TABLE (table8), label65, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label65), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label65), 3, 0);

  label66 = gtk_label_new (_("Name / SysOp"));
  gtk_widget_set_name (label66, "label66");
  gtk_widget_ref (label66);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label66", label66,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label66);
  gtk_table_attach (GTK_TABLE (table8), label66, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label66), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label66), 3, 0);

  label67 = gtk_label_new (_("City / QTH"));
  gtk_widget_set_name (label67, "label67");
  gtk_widget_ref (label67);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label67", label67,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label67);
  gtk_table_attach (GTK_TABLE (table8), label67, 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label67), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label67), 3, 0);

  label68 = gtk_label_new (_("Locator"));
  gtk_widget_set_name (label68, "label68");
  gtk_widget_ref (label68);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label68", label68,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label68);
  gtk_table_attach (GTK_TABLE (table8), label68, 0, 1, 6, 7,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label68), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label68), 3, 0);

  label69 = gtk_label_new (_("RX / TX"));
  gtk_widget_set_name (label69, "label69");
  gtk_widget_ref (label69);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label69", label69,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label69);
  gtk_table_attach (GTK_TABLE (table8), label69, 0, 1, 8, 9,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label69), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label69), 3, 0);

  label70 = gtk_label_new (_("Output ( Wtts )"));
  gtk_widget_set_name (label70, "label70");
  gtk_widget_ref (label70);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label70", label70,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label70);
  gtk_table_attach (GTK_TABLE (table8), label70, 0, 1, 10, 11,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label70), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label70), 3, 0);

  label71 = gtk_label_new (_("Antenna"));
  gtk_widget_set_name (label71, "label71");
  gtk_widget_ref (label71);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label71", label71,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label71);
  gtk_table_attach (GTK_TABLE (table8), label71, 0, 1, 12, 13,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label71), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label71), 3, 0);

  label72 = gtk_label_new (_("Homepage / E-Mail"));
  gtk_widget_set_name (label72, "label72");
  gtk_widget_ref (label72);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label72", label72,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label72);
  gtk_table_attach (GTK_TABLE (table8), label72, 0, 1, 14, 15,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label72), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label72), 3, 0);

  hseparator4 = gtk_hseparator_new ();
  gtk_widget_set_name (hseparator4, "hseparator4");
  gtk_widget_ref (hseparator4);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "hseparator4", hseparator4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hseparator4);
  gtk_table_attach (GTK_TABLE (table8), hseparator4, 0, 1, 16, 17,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  hseparator5 = gtk_hseparator_new ();
  gtk_widget_set_name (hseparator5, "hseparator5");
  gtk_widget_ref (hseparator5);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "hseparator5", hseparator5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hseparator5);
  gtk_table_attach (GTK_TABLE (table8), hseparator5, 1, 2, 16, 17,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  entry28 = gtk_entry_new_with_max_length (127);
  gtk_widget_set_name (entry28, "entry28");
  gtk_widget_ref (entry28);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "entry28", entry28,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry28);
  gtk_table_attach (GTK_TABLE (table8), entry28, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  entry29 = gtk_entry_new_with_max_length (127);
  gtk_widget_set_name (entry29, "entry29");
  gtk_widget_ref (entry29);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "entry29", entry29,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry29);
  gtk_table_attach (GTK_TABLE (table8), entry29, 1, 2, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  entry30 = gtk_entry_new_with_max_length (127);
  gtk_widget_set_name (entry30, "entry30");
  gtk_widget_ref (entry30);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "entry30", entry30,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry30);
  gtk_table_attach (GTK_TABLE (table8), entry30, 1, 2, 6, 7,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  entry31 = gtk_entry_new_with_max_length (127);
  gtk_widget_set_name (entry31, "entry31");
  gtk_widget_ref (entry31);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "entry31", entry31,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry31);
  gtk_table_attach (GTK_TABLE (table8), entry31, 1, 2, 8, 9,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  entry32 = gtk_entry_new_with_max_length (127);
  gtk_widget_set_name (entry32, "entry32");
  gtk_widget_ref (entry32);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "entry32", entry32,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry32);
  gtk_table_attach (GTK_TABLE (table8), entry32, 1, 2, 10, 11,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  entry33 = gtk_entry_new_with_max_length (127);
  gtk_widget_set_name (entry33, "entry33");
  gtk_widget_ref (entry33);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "entry33", entry33,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry33);
  gtk_table_attach (GTK_TABLE (table8), entry33, 1, 2, 12, 13,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  entry34 = gtk_entry_new_with_max_length (127);
  gtk_widget_set_name (entry34, "entry34");
  gtk_widget_ref (entry34);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "entry34", entry34,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry34);
  gtk_table_attach (GTK_TABLE (table8), entry34, 1, 2, 14, 15,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  entry27 = gtk_entry_new_with_max_length (15);
  gtk_widget_set_name (entry27, "entry27");
  gtk_widget_ref (entry27);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "entry27", entry27,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry27);
  gtk_table_attach (GTK_TABLE (table8), entry27, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

/* General Settings */
  table_general = gtk_table_new (17, 2, FALSE);
  gtk_widget_set_name (table_general, "table_general");
  gtk_widget_ref (table_general);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "table_general", table_general,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table_general);
  gtk_container_add (GTK_CONTAINER (paramnotebook), table_general);

  label_general = gtk_label_new (_("General"));
  gtk_widget_set_name (label_general, "label_general");
  gtk_widget_ref (label_general);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_general", label_general,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_general);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (paramnotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (paramnotebook), 1), label_general);

  
  gtk_tooltips_set_tip (tooltips, label_general, _("Note that you have to restart hfterm to make these basic settings work (except beacon pause and squelch percent)."), NULL);
  gtk_tooltips_set_tip (tooltips, table_general, _("Note that you have to restart hfterm to make these basic settings work (except beacon pause and squelch percent)."), NULL);

  
  label_soundcorr = gtk_label_new (_("sound correction"));
  gtk_widget_set_name (label_soundcorr, "label_soundcorr");
  gtk_widget_ref (label_soundcorr);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_soundcorr", label_soundcorr,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_soundcorr);
  gtk_table_attach (GTK_TABLE (table_general), label_soundcorr, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_soundcorr), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_soundcorr), 5, 0);

  soundcorr = gtk_entry_new_with_max_length (10);
  gtk_widget_set_name (soundcorr, "soundcorr");
  gtk_widget_ref (soundcorr);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "soundcorr", soundcorr,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (soundcorr);
  gtk_table_attach (GTK_TABLE (table_general), soundcorr, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_entry_set_text (GTK_ENTRY (soundcorr), _("1.0"));
  gtk_tooltips_set_tip (tooltips, soundcorr, _("Correction factor for the sample rate of your soundcard. 0.9-1.1. You can get it with one the 3 calibration programs dcf77rx, reffreq or ratecal, included in hf, see the hf-howto. You should calibrate for good pactor results."), NULL);
    
  
  label_timecorr = gtk_label_new (_("time correction"));
  gtk_widget_set_name (label_timecorr, "label_timecorr");
  gtk_widget_ref (label_timecorr);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_timecorr", label_timecorr,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_timecorr);
  gtk_table_attach (GTK_TABLE (table_general), label_timecorr, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_timecorr), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_timecorr), 5, 0);

  timecorr = gtk_entry_new_with_max_length (10);
  gtk_widget_set_name (timecorr, "timecorr");
  gtk_widget_ref (timecorr);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "timecorr", timecorr,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (timecorr);
  gtk_table_attach (GTK_TABLE (table_general), timecorr, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_entry_set_text (GTK_ENTRY (timecorr), _("1.0"));
  gtk_tooltips_set_tip (tooltips, timecorr, _("Correction factor for the gettimeofday system call. This is only seldom needed if your soundcard can not work full-duplex. You can get it with one the calibration programs dcf77rx or reffreq, included in hf, see the hf-howto."), NULL);
  
  
  label_cpumhzcorr = gtk_label_new (_("cpu rate correction"));
  gtk_widget_set_name (label_cpumhzcorr, "label_cpumhzcorr");
  gtk_widget_ref (label_cpumhzcorr);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_cpumhzcorr", label_cpumhzcorr,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_cpumhzcorr);
  gtk_table_attach (GTK_TABLE (table_general), label_cpumhzcorr, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_cpumhzcorr), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_cpumhzcorr), 5, 0);

  

  cpumhzcorr = gtk_entry_new_with_max_length (10);
  gtk_widget_set_name (cpumhzcorr, "cpumhzcorr");
  gtk_widget_ref (cpumhzcorr);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "cpumhzcorr", cpumhzcorr,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (cpumhzcorr);
  gtk_table_attach (GTK_TABLE (table_general), cpumhzcorr, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_entry_set_text (GTK_ENTRY (cpumhzcorr), _("0"));
  gtk_tooltips_set_tip (tooltips, cpumhzcorr, _("Enter here your real cpu MHZ rate. This is only very very seldom needed if a) your soundcard can not work full-duplex and b) your gettimeofday system call does not work. You can get it with one the calibration programs dcf77rx or reffreq, included in hf, see the hf-howto."), NULL);
  
  label_soundcard = gtk_label_new (_("Sound Device"));
  gtk_widget_set_name (label_soundcard, "label_soundcard");
  gtk_widget_ref (label_soundcard);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_soundcard", label_soundcard,(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_soundcard);
  gtk_table_attach (GTK_TABLE (table_general),label_soundcard, 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_soundcard), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_soundcard), 5, 0);

  soundcard = gtk_combo_new ();
  gtk_widget_set_name (soundcard, "soundcard");
  gtk_widget_ref (soundcard);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "soundcard", soundcard,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (soundcard);
  gtk_table_attach (GTK_TABLE (table_general), soundcard, 1, 2, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  soundcard_items = g_list_append (soundcard_items, (gpointer) _("/dev/dsp0"));
  soundcard_items = g_list_append (soundcard_items, (gpointer) _("/dev/dsp1"));  
  soundcard_items = g_list_append (soundcard_items, (gpointer) _("/dev/dsp2"));  
  soundcard_items = g_list_append (soundcard_items, (gpointer) _("/dev/dsp3"));  
  soundcard_items = g_list_append (soundcard_items, (gpointer) _("hw:0"));  
  soundcard_items = g_list_append (soundcard_items, (gpointer) _("hw:1"));
  soundcard_items = g_list_append (soundcard_items, (gpointer) _("hw:2"));
  soundcard_items = g_list_append (soundcard_items, (gpointer) _("hw:3"));
  gtk_combo_set_popdown_strings (GTK_COMBO (soundcard), soundcard_items);
  g_list_free (soundcard_items);

  soundcardvalue = GTK_COMBO (soundcard)->entry;
  gtk_widget_set_name (soundcardvalue, "soundcardvalue");
  gtk_widget_ref (soundcardvalue);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "soundcardvalue", soundcardvalue, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (soundcardvalue);
  gtk_entry_set_text (GTK_ENTRY (soundcardvalue), _("/dev/dsp0"));
  gtk_tooltips_set_tip (tooltips, soundcardvalue, _("/dev/dsp0 is default, first soundcard, and I will use default OSS API code. You can also test the new ALSA code by using hw:0 for the first sound card and so on. See the hf-howto."), NULL);
  
  label_serial = gtk_label_new (_("Serial Device"));
  gtk_widget_set_name (label_serial, "label_serial");
  gtk_widget_ref (label_serial);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_serial", label_serial,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_serial);
  gtk_table_attach (GTK_TABLE (table_general),label_serial, 0, 1, 5, 6,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_serial), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_serial), 5, 0);

  serial = gtk_combo_new ();
  gtk_widget_set_name (serial, "serial");
  gtk_widget_ref (serial);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "serial", serial,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (serial);
  gtk_table_attach (GTK_TABLE (table_general), serial, 1, 2, 5, 6,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  serial_items = g_list_append (serial_items, (gpointer) _("/dev/ttyS0"));
  serial_items = g_list_append (serial_items, (gpointer) _("/dev/ttyS1"));  
  serial_items = g_list_append (serial_items, (gpointer) _("/dev/ttyS2"));  
  serial_items = g_list_append (serial_items, (gpointer) _("/dev/ttyS3"));  
  gtk_combo_set_popdown_strings (GTK_COMBO (serial), serial_items);
  g_list_free (serial_items);

  serialvalue = GTK_COMBO (serial)->entry;
  gtk_widget_set_name (serialvalue, "serialvalue");
  gtk_widget_ref (serialvalue);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "serialvalue", serialvalue,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (serialvalue);
  gtk_entry_set_text (GTK_ENTRY (serialvalue), _("none"));
  gtk_tooltips_set_tip (tooltips, serialvalue, _("for ptt. None is default to prevent errors at first start."), NULL);
  
  label_kerneloptions = gtk_label_new (_("hfkernel options"));
  gtk_widget_set_name (label_kerneloptions, "label_kerneloptions");
  gtk_widget_ref (label_kerneloptions);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_kerneloptions", label_kerneloptions,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_kerneloptions);
  gtk_table_attach (GTK_TABLE (table_general), label_kerneloptions, 0, 1, 6, 7,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_kerneloptions), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_kerneloptions), 5, 0);

  kerneloptions = gtk_entry_new_with_max_length (256);
  gtk_widget_set_name (kerneloptions, "kerneloptions");
  gtk_widget_ref (kerneloptions);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "kerneloptions", kerneloptions,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (kerneloptions);
  gtk_table_attach (GTK_TABLE (table_general), kerneloptions, 1, 2, 6, 7,
                    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_entry_set_text (GTK_ENTRY (kerneloptions), _("1.0"));
  gtk_tooltips_set_tip (tooltips, kerneloptions, _("Normally no entries are needed here. For special options to hfkernel (the real-time sound driver). E.g. -n = no-memory-mapping, -h = halfduplex, -R = no RDTSC. Should be seldom needed, as hfkernel has now a better hardware detection. If hfkernel does not start, experiment with the options at a console first, see the instructions in the hf-howto."), NULL);
  
  label_beaconpause = gtk_label_new (_("Beacon Pause (sec)"));
  gtk_widget_set_name (label_beaconpause, "label_beaconpause");
  gtk_widget_ref (label_beaconpause);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_beaconpause", label_beaconpause,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_beaconpause);
  gtk_table_attach (GTK_TABLE (table_general),label_beaconpause, 0, 1, 8, 9,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_beaconpause), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_beaconpause), 5, 0);

  beaconpause = gtk_combo_new ();
  gtk_widget_set_name (beaconpause, "beaconpause");
  gtk_widget_ref (beaconpause);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "beaconpause", beaconpause,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (beaconpause);
  gtk_table_attach (GTK_TABLE (table_general), beaconpause, 1, 2, 8, 9,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  beaconpause_items = g_list_append (beaconpause_items, (gpointer) _("  5"));
  beaconpause_items = g_list_append (beaconpause_items, (gpointer) _(" 10"));  
  beaconpause_items = g_list_append (beaconpause_items, (gpointer) _(" 15"));  
  beaconpause_items = g_list_append (beaconpause_items, (gpointer) _(" 20"));  
  beaconpause_items = g_list_append (beaconpause_items, (gpointer) _(" 30"));  
  beaconpause_items = g_list_append (beaconpause_items, (gpointer) _(" 60"));
  beaconpause_items = g_list_append (beaconpause_items, (gpointer) _(" 90"));
  beaconpause_items = g_list_append (beaconpause_items, (gpointer) _("180"));
  beaconpause_items = g_list_append (beaconpause_items, (gpointer) _("300"));
  gtk_combo_set_popdown_strings (GTK_COMBO (beaconpause), beaconpause_items);
  g_list_free (beaconpause_items);

  beaconpausevalue = GTK_COMBO (beaconpause)->entry;
  gtk_widget_set_name (beaconpausevalue, "beaconpausevalue");
  gtk_widget_ref (beaconpausevalue);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "beaconpausevalue", beaconpausevalue, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (beaconpausevalue);
  gtk_entry_set_text (GTK_ENTRY (beaconpausevalue), _("10"));

  label_squelch = gtk_label_new (_("squelch percent"));
  gtk_widget_set_name (label_squelch, "label_squelch");
  gtk_widget_ref (label_squelch);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_squelch", label_squelch,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_squelch);
  gtk_table_attach (GTK_TABLE (table_general), label_squelch, 0, 1, 9, 10,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_squelch), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_squelch), 5, 0);

  squelchpercent = gtk_entry_new_with_max_length (2);
  gtk_widget_set_name (squelchpercent, "squelchpercent");
  gtk_widget_ref (squelchpercent);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "squelchpercent", squelchpercent, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (squelchpercent);
  gtk_table_attach (GTK_TABLE (table_general), squelchpercent, 1, 2, 9, 10, (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
     (GtkAttachOptions) (0), 3, 3);
  gtk_entry_set_text (GTK_ENTRY (squelchpercent), _("7"));

/* FSK mark + space + squelch */
  table_fsk = gtk_table_new (2, 2, FALSE);
  gtk_widget_set_name (table_fsk, "table_fsk");
  gtk_widget_ref (table_fsk);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "table_fsk", table_fsk,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table_fsk);
  gtk_container_add (GTK_CONTAINER (paramnotebook), table_fsk);

  label_FSK = gtk_label_new (_("FSK "));
  gtk_widget_set_name (label_FSK, "label_FSK");
  gtk_widget_ref (label_FSK);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_FSK", label_FSK,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_FSK);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (paramnotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (paramnotebook), 2), label_FSK);

  label_mark = gtk_label_new (_("FSK Mark Frequency"));
  gtk_widget_set_name (label_mark, "label_mark");
  gtk_widget_ref (label_mark);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_mark", label_mark,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_mark);
  gtk_table_attach (GTK_TABLE (table_fsk), label_mark, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_mark), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_mark), 5, 0);

  label_space = gtk_label_new (_("FSK Space Frequency"));
  gtk_widget_set_name (label_space, "label_space");
  gtk_widget_ref (label_space);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_space", label_space,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_space);
  gtk_table_attach (GTK_TABLE (table_fsk), label_space, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_space), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_space), 5, 0);

  fskmark = gtk_combo_new ();
  gtk_widget_set_name (fskmark, "fskmark");
  gtk_widget_ref (fskmark);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "fskmark", fskmark,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fskmark);
  gtk_table_attach (GTK_TABLE (table_fsk), fskmark, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  fskmark_items = g_list_append (fskmark_items, (gpointer) _("1800"));
  fskmark_items = g_list_append (fskmark_items, (gpointer) _("2400"));
  gtk_combo_set_popdown_strings (GTK_COMBO (fskmark), fskmark_items);
  g_list_free (fskmark_items);

  fskmarkfreq = GTK_COMBO (fskmark)->entry;
  gtk_widget_set_name (fskmarkfreq, "fskmarkfreq");
  gtk_widget_ref (fskmarkfreq);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "fskmarkfreq", fskmarkfreq,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fskmarkfreq);
  gtk_entry_set_text (GTK_ENTRY (fskmarkfreq), _("1800"));

  fskspace = gtk_combo_new ();
  gtk_widget_set_name (fskspace, "fskspace");
  gtk_widget_ref (fskspace);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "fskspace", fskspace,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fskspace);
  gtk_table_attach (GTK_TABLE (table_fsk), fskspace, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  fskspace_items = g_list_append (fskspace_items, (gpointer) _("1600"));
  fskspace_items = g_list_append (fskspace_items, (gpointer) _("2200"));
  gtk_combo_set_popdown_strings (GTK_COMBO (fskspace), fskspace_items);
  g_list_free (fskspace_items);

  fskspacefreq = GTK_COMBO (fskspace)->entry;
  gtk_widget_set_name (fskspacefreq, "fskspacefreq");
  gtk_widget_ref (fskspacefreq);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "fskspacefreq", fskspacefreq,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fskspacefreq);
  gtk_entry_set_text (GTK_ENTRY (fskspacefreq), _("1600"));

/* CW */

  table_cw = gtk_table_new (3, 2, FALSE);
  gtk_widget_set_name (table_cw, "table_cw");
  gtk_widget_ref (table_cw);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "table_cw", table_cw,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table_cw);
  gtk_container_add (GTK_CONTAINER (paramnotebook), table_cw);

  label_wpm = gtk_label_new (_("Speed (words per minute)"));
  gtk_widget_set_name (label_wpm, "label_wpm");
  gtk_widget_ref (label_wpm);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_wpm", label_wpm,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_wpm);
  gtk_table_attach (GTK_TABLE (table_cw), label_wpm, 0, 1, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_wpm), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_wpm), 5, 0);

  wpm = gtk_entry_new_with_max_length (5);
  gtk_widget_set_name (wpm, "wpm");
  gtk_widget_ref (wpm);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "wpm", wpm,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (wpm);
  gtk_table_attach (GTK_TABLE (table_cw), wpm, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_entry_set_text (GTK_ENTRY (wpm), _("7"));

  label_tone = gtk_label_new (_("CW tone Hz"));
  gtk_widget_set_name (label_tone, "label_tone");
  gtk_widget_ref (label_tone);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_tone", label_tone,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_tone);
  gtk_table_attach (GTK_TABLE (table_cw), label_tone, 0, 1, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_tone), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_tone), 5, 0);

  tone = gtk_entry_new_with_max_length (5);
  gtk_widget_set_name (tone, "tone");
  gtk_widget_ref (tone);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "tone", tone,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (tone);
  gtk_table_attach (GTK_TABLE (table_cw), tone, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_entry_set_text (GTK_ENTRY (tone), _("880"));
  gtk_tooltips_set_tip (tooltips, tone, _("pc speaker tone for elbug. 0 for silent."), NULL);

  farnsworth = gtk_check_button_new_with_label (_("Farnsworth Spacing"));
  gtk_widget_set_name (farnsworth, "farnsworth");
  gtk_widget_ref (farnsworth);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "farnsworth", farnsworth,
                            (GtkDestroyNotify) gtk_widget_unref);
  //gtk_widget_show (farnsworth);
  gtk_table_attach (GTK_TABLE (table_cw), farnsworth, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_tooltips_set_tip (tooltips, farnsworth, _("not yet implemented..."), NULL);

  dtr = gtk_check_button_new_with_label (_("DTR pin for elbug"));
  gtk_widget_set_name (dtr, "dtr");
  gtk_widget_ref (dtr);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "dtr", dtr,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dtr);
  gtk_table_attach (GTK_TABLE (table_cw), dtr, 1, 2, 3, 4, 
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);

  label_CW = gtk_label_new (_("CW"));
  gtk_widget_set_name (label_CW, "label_CW");
  gtk_widget_ref (label_CW);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_CW", label_CW,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_CW);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (paramnotebook), 
    gtk_notebook_get_nth_page (GTK_NOTEBOOK (paramnotebook), 3), label_CW);

/* RTTY */
  table_rtty = gtk_table_new (3, 2, FALSE);
  gtk_widget_set_name (table_rtty, "table_rtty");
  gtk_widget_ref (table_rtty);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "table_rtty", table_rtty,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table_rtty);
  gtk_container_add (GTK_CONTAINER (paramnotebook), table_rtty);

  label_baudrate = gtk_label_new (_("Baud Rate"));
  gtk_widget_set_name (label_baudrate, "label_baudrate");
  gtk_widget_ref (label_baudrate);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_baudrate", label_baudrate,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_baudrate);
  gtk_table_attach (GTK_TABLE (table_rtty), label_baudrate, 0, 1, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_baudrate), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_baudrate), 5, 0);

  rttyinvert = gtk_check_button_new_with_label (_("Invert"));
  gtk_widget_set_name (rttyinvert, "rttyinvert");
  gtk_widget_ref (rttyinvert);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "rttyinvert", rttyinvert,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (rttyinvert);
  gtk_table_attach (GTK_TABLE (table_rtty), rttyinvert, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);

  rttyrxtxinvert = gtk_check_button_new_with_label (_("RX/TX invert"));
  gtk_widget_set_name (rttyrxtxinvert, "rttyrxtxinvert");
  gtk_widget_ref (rttyrxtxinvert);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "rttyrxtxinvert", rttyrxtxinvert,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (rttyrxtxinvert);
  gtk_table_attach (GTK_TABLE (table_rtty), rttyrxtxinvert, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);

  rttybaudrate = gtk_entry_new_with_max_length (6);
  gtk_widget_set_name (rttybaudrate, "rttybaudrate");
  gtk_widget_ref (rttybaudrate);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "rttybaudrate", rttybaudrate,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (rttybaudrate);
  gtk_table_attach (GTK_TABLE (table_rtty), rttybaudrate, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_entry_set_text (GTK_ENTRY (rttybaudrate), _("45.45"));

  label_RTTY = gtk_label_new (_("RTTY"));
  gtk_widget_set_name (label_RTTY, "label_RTTY");
  gtk_widget_ref (label_RTTY);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_RTTY", label_RTTY,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_RTTY);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (paramnotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (paramnotebook), 4), label_RTTY);

  /* AMTOR */
  table_amtor = gtk_table_new (7, 2, FALSE);
  gtk_widget_set_name (table_amtor, "table_amtor");
  gtk_widget_ref (table_amtor);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "table_amtor", table_amtor,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table_amtor);
  gtk_container_add (GTK_CONTAINER (paramnotebook), table_amtor);

  label_amtor_retries = gtk_label_new (_("Retries"));
  gtk_widget_set_name (label_amtor_retries, "label_amtor_retries");
  gtk_widget_ref (label_amtor_retries);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_amtor_retries", label_amtor_retries,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_amtor_retries);
  gtk_table_attach (GTK_TABLE (table_amtor), label_amtor_retries, 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_amtor_retries), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_amtor_retries), 5, 0);

  label_amtor_txdelay = gtk_label_new (_("TX Delay (ms)"));
  gtk_widget_set_name (label_amtor_txdelay, "label_amtor_txdelay");
  gtk_widget_ref (label_amtor_txdelay);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_amtor_txdelay", label_amtor_txdelay,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_amtor_txdelay);
  gtk_table_attach (GTK_TABLE (table_amtor), label_amtor_txdelay, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_amtor_txdelay), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_amtor_txdelay), 5, 0);

  label_amtor_mycall = gtk_label_new (_("Mycall"));
  gtk_widget_set_name (label_amtor_mycall, "label_amtor_mycall");
  gtk_widget_ref (label_amtor_mycall);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_amtor_mycall", label_amtor_mycall,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_amtor_mycall);
  gtk_table_attach (GTK_TABLE (table_amtor), label_amtor_mycall, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_amtor_mycall), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_amtor_mycall), 5, 0);

  label_amtor_selfeccall = gtk_label_new (_("Selective FEC Call"));
  gtk_widget_set_name (label_amtor_selfeccall, "label_amtor_selfeccall");
  gtk_widget_ref (label_amtor_selfeccall);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_amtor_selfeccall", label_amtor_selfeccall,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_amtor_selfeccall);
  gtk_table_attach (GTK_TABLE (table_amtor), label_amtor_selfeccall, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_amtor_selfeccall), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_amtor_selfeccall), 5, 0);

  label_amtor_destcall = gtk_label_new (_("Destination Call"));
  gtk_widget_set_name (label_amtor_destcall, "label_amtor_destcall");
  gtk_widget_ref (label_amtor_destcall);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_amtor_destcall", label_amtor_destcall,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_amtor_destcall);
  gtk_table_attach (GTK_TABLE (table_amtor), label_amtor_destcall, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_amtor_destcall), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_amtor_destcall), 5, 0);

  amtorrxtxinvert = gtk_check_button_new_with_label (_("RX/TX invert"));
  gtk_widget_set_name (amtorrxtxinvert, "amtorrxtxinvert");
  gtk_widget_ref (amtorrxtxinvert);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "amtorrxtxinvert", amtorrxtxinvert,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (amtorrxtxinvert);
  gtk_table_attach (GTK_TABLE (table_amtor), amtorrxtxinvert, 1, 2, 6, 7,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);

  amtorinvert = gtk_check_button_new_with_label (_("Invert"));
  gtk_widget_set_name (amtorinvert, "amtorinvert");
  gtk_widget_ref (amtorinvert);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "amtorinvert", amtorinvert,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (amtorinvert);
  gtk_table_attach (GTK_TABLE (table_amtor), amtorinvert, 1, 2, 5, 6,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);

  amtordestcall = gtk_entry_new_with_max_length (4);
  gtk_widget_set_name (amtordestcall, "amtordestcall");
  gtk_widget_ref (amtordestcall);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "amtordestcall", amtordestcall,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (amtordestcall);
  gtk_table_attach (GTK_TABLE (table_amtor), amtordestcall, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_entry_set_text (GTK_ENTRY (amtordestcall), _("NONE"));

  amtorselfeccall = gtk_entry_new_with_max_length (4);
  gtk_widget_set_name (amtorselfeccall, "amtorselfeccall");
  gtk_widget_ref (amtorselfeccall);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "amtorselfeccall", amtorselfeccall,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (amtorselfeccall);
  gtk_table_attach (GTK_TABLE (table_amtor), amtorselfeccall, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_entry_set_text (GTK_ENTRY (amtorselfeccall), _("NONE"));

  amtormycall = gtk_entry_new_with_max_length (4);
  gtk_widget_set_name (amtormycall, "amtormycall");
  gtk_widget_ref (amtormycall);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "amtormycall", amtormycall,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (amtormycall);
  gtk_table_attach (GTK_TABLE (table_amtor), amtormycall, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_entry_set_text (GTK_ENTRY (amtormycall), _("NONE"));

  amtortxdelay = gtk_entry_new ();
  gtk_widget_set_name (amtortxdelay, "amtortxdelay");
  gtk_widget_ref (amtortxdelay);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "amtortxdelay", amtortxdelay,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (amtortxdelay);
  gtk_table_attach (GTK_TABLE (table_amtor), amtortxdelay, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_entry_set_text (GTK_ENTRY (amtortxdelay), _("30"));

  amtorretry = gtk_entry_new ();
  gtk_widget_set_name (amtorretry, "amtorretry");
  gtk_widget_ref (amtorretry);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "amtorretry", amtorretry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (amtorretry);
  gtk_table_attach (GTK_TABLE (table_amtor), amtorretry, 1, 2, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_entry_set_text (GTK_ENTRY (amtorretry), _("30"));

  label_amtor = gtk_label_new (_("Amtor"));
  gtk_widget_set_name (label_amtor, "label_amtor");
  gtk_widget_ref (label_amtor);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_amtor", label_amtor,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_amtor);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (paramnotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (paramnotebook), 5), label_amtor);

/* Gtor */
  table_Gtor = gtk_table_new (4, 2, FALSE);
  gtk_widget_set_name (table_Gtor, "table_Gtor");
  gtk_widget_ref (table_Gtor);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "table_Gtor", table_Gtor,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table_Gtor);
  gtk_container_add (GTK_CONTAINER (paramnotebook), table_Gtor);

  gtordestcall = gtk_entry_new_with_max_length (12);
  gtk_widget_set_name (gtordestcall, "gtordestcall");
  gtk_widget_ref (gtordestcall);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "gtordestcall", gtordestcall,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (gtordestcall);
  gtk_table_attach (GTK_TABLE (table_Gtor), gtordestcall, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_entry_set_text (GTK_ENTRY (gtordestcall), _("NOCALL"));

/*
  label_gtor_mycall = gtk_label_new (_("Mycall"));
  gtk_widget_set_name (label_gtor_mycall, "label_gtor_mycall");
  gtk_widget_ref (label_gtor_mycall);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_gtor_mycall", label_gtor_mycall,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_gtor_mycall);
  gtk_table_attach (GTK_TABLE (table_Gtor), label_gtor_mycall, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_gtor_mycall), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_gtor_mycall), 5, 0);

  gtormycall = gtk_entry_new_with_max_length (12);
  gtk_widget_set_name (gtormycall, "gtormycall");
  gtk_widget_ref (gtormycall);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "gtormycall", gtormycall,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (gtormycall);
  gtk_table_attach (GTK_TABLE (table_Gtor), gtormycall, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_entry_set_text (GTK_ENTRY (gtormycall), _("NOCALL"));
*/

  gtortxdelay = gtk_entry_new ();
  gtk_widget_set_name (gtortxdelay, "gtortxdelay");
  gtk_widget_ref (gtortxdelay);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "gtortxdelay", gtortxdelay,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (gtortxdelay);
  gtk_table_attach (GTK_TABLE (table_Gtor), gtortxdelay, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_entry_set_text (GTK_ENTRY (gtortxdelay), _("30"));

  gtorretry = gtk_entry_new ();
  gtk_widget_set_name (gtorretry, "gtorretry");
  gtk_widget_ref (gtorretry);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "gtorretry", gtorretry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (gtorretry);
  gtk_table_attach (GTK_TABLE (table_Gtor), gtorretry, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_entry_set_text (GTK_ENTRY (gtorretry), _("30"));

  label_gtor_destcall = gtk_label_new (_("Destination Call"));
  gtk_widget_set_name (label_gtor_destcall, "label_gtor_destcall");
  gtk_widget_ref (label_gtor_destcall);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_gtor_destcall", label_gtor_destcall,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_gtor_destcall);
  gtk_table_attach (GTK_TABLE (table_Gtor), label_gtor_destcall, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_gtor_destcall), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_gtor_destcall), 5, 0);


  label_gtor_txdelay = gtk_label_new (_("TX Delay (ms)"));
  gtk_widget_set_name (label_gtor_txdelay, "label_gtor_txdelay");
  gtk_widget_ref (label_gtor_txdelay);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_gtor_txdelay", label_gtor_txdelay,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_gtor_txdelay);
  gtk_table_attach (GTK_TABLE (table_Gtor), label_gtor_txdelay, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_gtor_txdelay), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_gtor_txdelay), 5, 0);

  label_gtor_retries = gtk_label_new (_("Retries"));
  gtk_widget_set_name (label_gtor_retries, "label_gtor_retries");
  gtk_widget_ref (label_gtor_retries);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_gtor_retries", label_gtor_retries,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_gtor_retries);
  gtk_table_attach (GTK_TABLE (table_Gtor), label_gtor_retries, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_gtor_retries), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_gtor_retries), 5, 0);

  label_Gtor = gtk_label_new (_("GTOR"));
  gtk_widget_set_name (label_Gtor, "label_Gtor");
  gtk_widget_ref (label_Gtor);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_Gtor", label_Gtor,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_Gtor);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (paramnotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (paramnotebook), 6), label_Gtor);

/* Pactor */
  hbox_pactor_standard = gtk_hbox_new (TRUE, 2);
  gtk_widget_set_name (hbox_pactor_standard, "hbox_pactor_standard");
  gtk_widget_ref (hbox_pactor_standard);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "hbox_pactor_standard", hbox_pactor_standard,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox_pactor_standard);
  gtk_container_add (GTK_CONTAINER (paramnotebook), hbox_pactor_standard);

  frame_pactor_standard = gtk_frame_new (_("Standard"));
  gtk_widget_set_name (frame_pactor_standard, "frame_pactor_standard");
  gtk_widget_ref (frame_pactor_standard);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "frame_pactor_standard", frame_pactor_standard,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame_pactor_standard);
  gtk_box_pack_start (GTK_BOX (hbox_pactor_standard), frame_pactor_standard, TRUE, TRUE, 0);

  label_Pactor = gtk_label_new (_("Pactor"));
  gtk_widget_set_name (label_Pactor, "label_Pactor");
  gtk_widget_ref (label_Pactor);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_Pactor", label_Pactor,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_Pactor);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (paramnotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (paramnotebook), 7), label_Pactor);

  table_pactor_standard = gtk_table_new (5, 2, FALSE);
  gtk_widget_set_name (table_pactor_standard, "table_pactor_standard");
  gtk_widget_ref (table_pactor_standard);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "table_pactor_standard", table_pactor_standard,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table_pactor_standard);
  gtk_container_add (GTK_CONTAINER (frame_pactor_standard), table_pactor_standard);

/*
  label_pactor_mycall = gtk_label_new (_("Mycall"));
  gtk_widget_set_name (label_pactor_mycall, "label_pactor_mycall");
  gtk_widget_ref (label_pactor_mycall);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_pactor_mycall", label_pactor_mycall,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_pactor_mycall);
  gtk_table_attach (GTK_TABLE (table_pactor_standard), label_pactor_mycall, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_pactor_mycall), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_pactor_mycall), 5, 0);

  pactormycall = gtk_entry_new_with_max_length (7);
  gtk_widget_set_name (pactormycall, "pactormycall");
  gtk_widget_ref (pactormycall);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "pactormycall", pactormycall,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (pactormycall);
  gtk_table_attach (GTK_TABLE (table_pactor_standard), pactormycall, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_widget_set_usize (pactormycall, 10, -2);
  gtk_entry_set_text (GTK_ENTRY (pactormycall), _("NOCALL"));
*/

  label_pactor_txdelay = gtk_label_new (_("TX Delay (ms)"));
  gtk_widget_set_name (label_pactor_txdelay, "label_pactor_txdelay");
  gtk_widget_ref (label_pactor_txdelay);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_pactor_txdelay", label_pactor_txdelay,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_pactor_txdelay);
  gtk_table_attach (GTK_TABLE (table_pactor_standard), label_pactor_txdelay, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_pactor_txdelay), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_pactor_txdelay), 5, 0);

  label_pactor_retries = gtk_label_new (_("Retries"));
  gtk_widget_set_name (label_pactor_retries, "label_pactor_retries");
  gtk_widget_ref (label_pactor_retries);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_pactor_retries", label_pactor_retries,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_pactor_retries);
  gtk_table_attach (GTK_TABLE (table_pactor_standard), label_pactor_retries, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_pactor_retries), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_pactor_retries), 5, 0);

  pactorlongpath = gtk_check_button_new_with_label (_("Longpath"));
  gtk_widget_set_name (pactorlongpath, "pactorlongpath");
  gtk_widget_ref (pactorlongpath);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "pactorlongpath", pactorlongpath,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (pactorlongpath);
  gtk_table_attach (GTK_TABLE (table_pactor_standard), pactorlongpath, 1, 2, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);

  pactorcall = gtk_entry_new_with_max_length (7);
  gtk_widget_set_name (pactorcall, "pactorcall");
  gtk_widget_ref (pactorcall);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "pactorcall", pactorcall,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (pactorcall);
  gtk_table_attach (GTK_TABLE (table_pactor_standard), pactorcall, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_widget_set_usize (pactorcall, 10, -2);
  gtk_entry_set_text (GTK_ENTRY (pactorcall), _("NOCALL"));

 
  pactortxdelay = gtk_entry_new ();
  gtk_widget_set_name (pactortxdelay, "pactortxdelay");
  gtk_widget_ref (pactortxdelay);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "pactortxdelay", pactortxdelay,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (pactortxdelay);
  gtk_table_attach (GTK_TABLE (table_pactor_standard), pactortxdelay, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_widget_set_usize (pactortxdelay, 10, -2);
  gtk_entry_set_text (GTK_ENTRY (pactortxdelay), _("30"));

  pactorretry = gtk_entry_new ();
  gtk_widget_set_name (pactorretry, "pactorretry");
  gtk_widget_ref (pactorretry);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "pactorretry", pactorretry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (pactorretry);
  gtk_table_attach (GTK_TABLE (table_pactor_standard), pactorretry, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_widget_set_usize (pactorretry, 10, -2);
  gtk_entry_set_text (GTK_ENTRY (pactorretry), _("30"));

  label_pactor_destcall = gtk_label_new (_("Destination Call"));
  gtk_widget_set_name (label_pactor_destcall, "label_pactor_destcall");
  gtk_widget_ref (label_pactor_destcall);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_pactor_destcall", label_pactor_destcall,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_pactor_destcall);
  gtk_table_attach (GTK_TABLE (table_pactor_standard), label_pactor_destcall, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_pactor_destcall), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_pactor_destcall), 5, 0);

  label_pactor_splp = gtk_label_new (_("SP/LP"));
  gtk_widget_set_name (label_pactor_splp, "label_pactor_splp");
  gtk_widget_ref (label_pactor_splp);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_pactor_splp", label_pactor_splp,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_pactor_splp);
  gtk_table_attach (GTK_TABLE (table_pactor_standard), label_pactor_splp, 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_pactor_splp), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_pactor_splp), 5, 0);


  frame_pactor_advanced = gtk_frame_new (_("Advanced"));
  gtk_widget_set_name (frame_pactor_advanced, "frame_pactor_advanced");
  gtk_widget_ref (frame_pactor_advanced);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "frame_pactor_advanced", frame_pactor_advanced,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame_pactor_advanced);
  gtk_box_pack_start (GTK_BOX (hbox_pactor_standard), frame_pactor_advanced, TRUE, TRUE, 0);

  table_pactor_advanced = gtk_table_new (4, 2, FALSE);
  gtk_widget_set_name (table_pactor_advanced, "table_pactor_advanced");
  gtk_widget_ref (table_pactor_advanced);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "table_pactor_advanced", table_pactor_advanced,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table_pactor_advanced);
  gtk_container_add (GTK_CONTAINER (frame_pactor_advanced), table_pactor_advanced);

  pactorcrc0 = gtk_entry_new ();
  gtk_widget_set_name (pactorcrc0, "pactorcrc0");
  gtk_widget_ref (pactorcrc0);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "pactorcrc0", pactorcrc0,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (pactorcrc0);
  gtk_table_attach (GTK_TABLE (table_pactor_advanced), pactorcrc0, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_widget_set_usize (pactorcrc0, 10, -2);

  pactorcrc1 = gtk_entry_new ();
  gtk_widget_set_name (pactorcrc1, "pactorcrc1");
  gtk_widget_ref (pactorcrc1);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "pactorcrc1", pactorcrc1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (pactorcrc1);
  gtk_table_attach (GTK_TABLE (table_pactor_advanced), pactorcrc1, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_widget_set_usize (pactorcrc1, 10, -2);

  pactorcrc3 = gtk_entry_new ();
  gtk_widget_set_name (pactorcrc3, "pactorcrc3");
  gtk_widget_ref (pactorcrc3);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "pactorcrc3", pactorcrc3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (pactorcrc3);
  gtk_table_attach (GTK_TABLE (table_pactor_advanced), pactorcrc3, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_widget_set_usize (pactorcrc3, 10, -2);

  pactorcrc2 = gtk_entry_new ();
  gtk_widget_set_name (pactorcrc2, "pactorcrc2");
  gtk_widget_ref (pactorcrc2);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "pactorcrc2", pactorcrc2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (pactorcrc2);
  gtk_table_attach (GTK_TABLE (table_pactor_advanced), pactorcrc2, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_widget_set_usize (pactorcrc2, 10, -2);

  label_pactor_crc100chg = gtk_label_new (_("CRC 100 Chg"));
  gtk_widget_set_name (label_pactor_crc100chg, "label_pactor_crc100chg");
  gtk_widget_ref (label_pactor_crc100chg);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_pactor_crc100chg", label_pactor_crc100chg,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_pactor_crc100chg);
  gtk_table_attach (GTK_TABLE (table_pactor_advanced), label_pactor_crc100chg, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_pactor_crc100chg), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_pactor_crc100chg), 5, 0);

  label_pactor_crc100 = gtk_label_new (_("CRC 100"));
  gtk_widget_set_name (label_pactor_crc100, "label_pactor_crc100");
  gtk_widget_ref (label_pactor_crc100);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_pactor_crc100", label_pactor_crc100,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_pactor_crc100);
  gtk_table_attach (GTK_TABLE (table_pactor_advanced), label_pactor_crc100, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label_pactor_crc100), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label_pactor_crc100), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_pactor_crc100), 5, 0);

  label_pactor_crc200chg = gtk_label_new (_("CRC 200 Chg"));
  gtk_widget_set_name (label_pactor_crc200chg, "label_pactor_crc200chg");
  gtk_widget_ref (label_pactor_crc200chg);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_pactor_crc200chg", label_pactor_crc200chg,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_pactor_crc200chg);
  gtk_table_attach (GTK_TABLE (table_pactor_advanced), label_pactor_crc200chg, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_pactor_crc200chg), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_pactor_crc200chg), 5, 0);

  label_pactor_crc200 = gtk_label_new (_("CRC 200"));
  gtk_widget_set_name (label_pactor_crc200, "label_pactor_crc200");
  gtk_widget_ref (label_pactor_crc200);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_pactor_crc200", label_pactor_crc200,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_pactor_crc200);
  gtk_table_attach (GTK_TABLE (table_pactor_advanced), label_pactor_crc200, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_pactor_crc200), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_pactor_crc200), 5, 0);


/* mt63 */
  table_mt63 = gtk_table_new (4, 2, FALSE);
  gtk_widget_set_name (table_mt63, "table_mt63");
  gtk_widget_ref (table_mt63);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "table_mt63", table_mt63,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table_mt63);
  gtk_container_add (GTK_CONTAINER (paramnotebook), table_mt63);

  label_mt63_bandwidth = gtk_label_new (_("Bandwidth"));
  gtk_widget_set_name (label_mt63_bandwidth, "label_mt63_bandwidth");
  gtk_widget_ref (label_mt63_bandwidth);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_mt63_bandwidth", label_mt63_bandwidth,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_mt63_bandwidth);
  gtk_table_attach (GTK_TABLE (table_mt63), label_mt63_bandwidth, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_mt63_bandwidth), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_mt63_bandwidth), 5, 0);

  mt63_bandwidth = gtk_combo_new ();
  gtk_widget_set_name (mt63_bandwidth, "mt63_bandwidth");
  gtk_widget_ref (mt63_bandwidth);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "mt63_bandwidth", mt63_bandwidth,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (mt63_bandwidth);
  gtk_table_attach (GTK_TABLE (table_mt63), mt63_bandwidth, 1, 2, 0, 1, 
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  mt63_bandwidth_items = g_list_append (mt63_bandwidth_items, (gpointer) _(" 500"));
  mt63_bandwidth_items = g_list_append (mt63_bandwidth_items, (gpointer) _("1000"));
  mt63_bandwidth_items = g_list_append (mt63_bandwidth_items, (gpointer) _("2000"));
  gtk_combo_set_popdown_strings (GTK_COMBO (mt63_bandwidth), mt63_bandwidth_items);
  g_list_free (mt63_bandwidth_items);

  mt63_bandwidth_value = GTK_COMBO (mt63_bandwidth)->entry;
  gtk_widget_set_name (mt63_bandwidth_value, "mt63_bandwidth_value");
  gtk_widget_ref (mt63_bandwidth_value);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "mt63_bandwidth_value", 
                              mt63_bandwidth_value,
                    	    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (mt63_bandwidth_value);
  gtk_entry_set_editable (GTK_ENTRY (mt63_bandwidth_value), FALSE);
  gtk_entry_set_text (GTK_ENTRY (mt63_bandwidth_value), _("1000"));

  label_mt63_integration = gtk_label_new (_("Integration Period"));
  gtk_widget_set_name (label_mt63_integration, "label_mt63_integration");
  gtk_widget_ref (label_mt63_integration);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_mt63_integration", label_mt63_integration,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_mt63_integration);
  gtk_table_attach (GTK_TABLE (table_mt63), label_mt63_integration, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_mt63_integration), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_mt63_integration), 5, 0);

  mt63_integration = gtk_entry_new_with_max_length (3);
  gtk_widget_set_name (mt63_integration, "mt63_integration");
  gtk_widget_ref (mt63_integration);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "mt63_integration", mt63_integration,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (mt63_integration);
  gtk_table_attach (GTK_TABLE (table_mt63), mt63_integration, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_entry_set_text (GTK_ENTRY (mt63_integration), _("32"));

  label_mt63_cwcall = gtk_label_new (_("CW ID (none if empty)"));
  gtk_widget_set_name (label_mt63_cwcall, "label_mt63_cwcall");
  gtk_widget_ref (label_mt63_cwcall);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_mt63_cwcall", label_mt63_cwcall,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_mt63_cwcall);
  gtk_table_attach (GTK_TABLE (table_mt63), label_mt63_cwcall, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_mt63_cwcall), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_mt63_cwcall), 5, 0);

  mt63_cwcall = gtk_entry_new_with_max_length (10);
  gtk_widget_set_name (mt63_cwcall, "mt63_cwcall");
  gtk_widget_ref (mt63_cwcall);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "mt63_cwcall", mt63_cwcall,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (mt63_cwcall);
  gtk_table_attach (GTK_TABLE (table_mt63), mt63_cwcall, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_entry_set_text (GTK_ENTRY (mt63_cwcall), _("MT63"));

  mt63_doubleinterleave = gtk_check_button_new_with_label (_("Double Interleave"));
  gtk_widget_set_name (mt63_doubleinterleave, "mt63_doubleinterleave");
  gtk_widget_ref (mt63_doubleinterleave);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "mt63_doubleinterleave", mt63_doubleinterleave,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (mt63_doubleinterleave);
  gtk_table_attach (GTK_TABLE (table_mt63), mt63_doubleinterleave, 1, 2, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);

  label_mt63 = gtk_label_new (_("MT63"));
  gtk_widget_set_name (label_mt63, "label_mt63");
  gtk_widget_ref (label_mt63);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_mt63", label_mt63,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_mt63);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (paramnotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (paramnotebook), 8), label_mt63);

/* mailbox */
/*
  GtkWidget *table_mailbox;
  GtkWidget *label_mailbox;
  GtkWidget *label_mailbox_host;
  GtkWidget *mailbox_host;
  GtkWidget *label_mailbox_port;
  GtkWidget *mailbox_port;
*/
  table_mailbox = gtk_table_new (4, 2, FALSE);
  gtk_widget_set_name (table_mailbox, "table_mailbox");
  gtk_widget_ref (table_mailbox);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "table_mailbox", table_mailbox,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table_mailbox);
  gtk_container_add (GTK_CONTAINER (paramnotebook), table_mailbox);

  label_mailbox = gtk_label_new (_("mailbox"));
  gtk_widget_set_name (label_mailbox, "label_mailbox");
  gtk_widget_ref (label_mailbox);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_mailbox", label_mailbox,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_mailbox);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (paramnotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (paramnotebook), 9), label_mailbox);

  label_mailbox_host = gtk_label_new (_("host"));
  gtk_widget_set_name (label_mailbox_host, "label_mailbox_host");
  gtk_widget_ref (label_mailbox_host);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_mailbox_host", label_mailbox_host,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_mailbox_host);
  gtk_table_attach (GTK_TABLE (table_mailbox), label_mailbox_host, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_mailbox_host), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_mailbox_host), 5, 0);

  mailbox_host = gtk_entry_new_with_max_length (15);
  gtk_widget_set_name (mailbox_host, "mailbox_host");
  gtk_widget_ref (mailbox_host);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "mailbox_host", mailbox_host,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (mailbox_host);
  gtk_table_attach (GTK_TABLE (table_mailbox), mailbox_host, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_entry_set_text (GTK_ENTRY (mailbox_host), _("127.0.0.1"));

  label_mailbox_port = gtk_label_new (_("port"));
  gtk_widget_set_name (label_mailbox_port, "label_mailbox_port");
  gtk_widget_ref (label_mailbox_port);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "label_mailbox_port", label_mailbox_port,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label_mailbox_port);
  gtk_table_attach (GTK_TABLE (table_mailbox), label_mailbox_port, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_mailbox_port), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label_mailbox_port), 5, 0);

  mailbox_port = gtk_entry_new_with_max_length (5);
  gtk_widget_set_name (mailbox_port, "mailbox_port");
  gtk_widget_ref (mailbox_port);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "mailbox_port", mailbox_port,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (mailbox_port);
  gtk_table_attach (GTK_TABLE (table_mailbox), mailbox_port, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_entry_set_text (GTK_ENTRY (mailbox_port), _("6300"));

/* end mailbox */

  toolbar1 = gtk_toolbar_new (GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_TEXT);
  gtk_widget_set_name (toolbar1, "toolbar1");
  gtk_widget_ref (toolbar1);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "toolbar1", toolbar1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (toolbar1);
  gtk_box_pack_start (GTK_BOX (param_vbox), toolbar1, FALSE, FALSE, 0);

  parok = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar1),
                                GTK_TOOLBAR_CHILD_BUTTON,
                                NULL,
                                _("OK"),
                                NULL, NULL,
                                NULL, NULL, NULL);
  gtk_widget_set_name (parok, "parok");
  gtk_widget_ref (parok);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "parok", parok,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (parok);
  gtk_container_set_border_width (GTK_CONTAINER (parok), 3);

  parcancel = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_set_name (parcancel, "parcancel");
  gtk_widget_ref (parcancel);
  gtk_object_set_data_full (GTK_OBJECT (wpar), "parcancel", parcancel,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (parcancel);
  gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar1), parcancel, NULL, NULL);

  gtk_signal_connect (GTK_OBJECT (wpar), "delete_event",
                      GTK_SIGNAL_FUNC (gtk_widget_hide),
                      NULL);
  gtk_widget_add_accelerator (wpar, "hide",
  			par_accels,
			GDK_p, GDK_SHIFT_MASK,
      	                GTK_ACCEL_VISIBLE);

  
  gtk_signal_connect (GTK_OBJECT (parok), "clicked",
                      GTK_SIGNAL_FUNC (on_parok_clicked),
                      NULL);
  
  gtk_widget_add_accelerator (parok, "clicked",
  			par_accels,
			GDK_Return, 0, 
      	                GTK_ACCEL_VISIBLE);
 
  gtk_signal_connect (GTK_OBJECT (parcancel), "clicked",
                      GTK_SIGNAL_FUNC (on_parcancel_clicked),
                      NULL);

  gtk_widget_add_accelerator (parcancel, "clicked",
  			par_accels,
			GDK_Escape, 0, 
      	                GTK_ACCEL_VISIBLE);
 

  return wpar;
}

