/*
 *  interface for main window
 */
#include "hft.h"

GtkWidget*
create_wmain (void)
{
  GtkWidget *wmain;
  GtkWidget *vbox_main;
  GtkWidget *vbox_textwindows;
  GtkWidget *handlebox1;
  GtkWidget *menubar1;
  GtkWidget *file;
  GtkWidget *file_menu;
  GtkAccelGroup *file_menu_accels;
  GtkWidget *quit;
  GtkWidget *rx_file;
  GtkWidget *tx_file;  
  GtkWidget *beacon_file;
  GtkWidget *state;
  GtkWidget *state_menu;
  GtkAccelGroup *state_menu_accels;
  GtkWidget *become_irs;
  GtkWidget *become_iss;
  GtkWidget *qrt;
  GtkWidget *speedup;
  GtkWidget *uppercase;
  GtkWidget *lowercase;
  GtkWidget *figurecase;
  GtkWidget *mailbox;
  GtkWidget *mode;
  GtkWidget *mode_menu;
  GtkAccelGroup *mode_menu_accels;
//  GtkWidget *helpoption;
//  GtkWidget *help_line;
  GtkWidget *cw;  
//  GtkWidget *cw_line;
  GtkWidget *rtty_line;
  GtkWidget *standby_line;
  GtkWidget *standby;  
  GtkWidget *standby_pactor;
  GtkWidget *amtor_line;
  GSList *mode_menu_group = NULL;
  GtkWidget *pactor_arq;
  GtkWidget *pactor_fec;
  GtkWidget *gtor_arq1;
  GtkWidget *amtor_arq;
  GtkWidget *amtor_collective_fec;
  GtkWidget *amtor_selective_fec;
  GtkWidget *rtty_receive;
  GtkWidget *rtty_transmit;
  GtkWidget *menulinemt63;
  GtkWidget *mt63_rx;
  GtkWidget *mt63_tx;
  GtkWidget *spectrum;
  GtkWidget *parameters;
  GtkWidget *monitor;
  GtkWidget *Wfixtext;
  GtkWidget *line10;
  GtkWidget *log_window_show;
  GtkWidget *logbook1;
  GtkWidget *logbook1_menu;
  GtkAccelGroup *logbook1_menu_accels;
  GtkWidget *save_logentry;
  GtkWidget *new_logentry;
  GtkWidget *clear_logentry;
  GtkWidget *archivate;
  GtkWidget *search_entry1;
  GtkWidget *line6;
  GtkWidget *list_all_entrys1;
  GtkWidget *line7;
//  GtkWidget *map1;
  GtkWidget *help;
  GtkWidget *help_menu;
  GtkAccelGroup *help_menu_accels;
  GtkWidget *about;
  GtkWidget *index1;
  GtkWidget *logbook_frame;
  GtkWidget *table7;
  GtkWidget *entry_rstin;
  GtkWidget *Call_label;
  GtkWidget *QTH_label;
  GtkWidget *Rcvd_label;
  GtkWidget *Mode_label;
  GtkWidget *Band_label;
  GtkWidget *Note_label;
  GtkWidget *Send_label;
  GtkWidget *entry_call;
  GtkWidget *entry_name;
  GtkWidget *combo1;
  GList *combo1_items = NULL;
  GtkWidget *combo_entry_mode;
  GtkWidget *combo2;
  GList *combo2_items = NULL;
  GtkWidget *combo_entry_band;
  GtkWidget *entry_notes;
  GtkWidget *entry_rstout;
  GtkWidget *Name_label;
  GtkWidget *entry_qth;
  GtkWidget *log_win_ok_button;
  GtkWidget *log_win_cancel_button;

  GtkWidget *rightbuttonframe;
  GtkWidget *vboxfix;
  GtkWidget *Fix_button1;
  GtkWidget *Fix_button2;
  GtkWidget *Fix_button3;
  GtkWidget *Fix_button4;
  GtkWidget *Fix_button5;
  GtkWidget *Fix_button6;
  GtkWidget *Fix_button7;
  GtkWidget *Fix_button8;
  GtkWidget *Fix_button9;
  GtkWidget *Fix_button10;
  GtkWidget *Fix_button11;
  GtkWidget *Fix_button12;

  GtkWidget *hbox_big_downside;

  GtkWidget *leftbuttonframe;
  GtkWidget *vboxmode;
  GtkWidget *standbybutton;
  GtkWidget *helpbutton;
  GtkWidget *spectrumbutton;  
//  GtkWidget *standby_pactorbutton;
  GtkWidget *CW_button;
  GtkWidget *P_ARQ_button;
  GtkWidget *P_FEC_button;
  GtkWidget *GTOR_button;
  GtkWidget *A_ARQ_button;
  GtkWidget *A_CFEC_button;
  GtkWidget *A_SFEC_button;
  GtkWidget *RTTY_RX_button;
//  GtkWidget *RTTY_TX_button;
  GtkWidget *MT63_button;
//  GtkWidget *parabutton;

  GtkWidget *scrolledwindow40;
  GtkWidget *frame8;
  GtkWidget *textmain;
  GtkWidget *frame4;
  GtkWidget *scrolledwindow38;
  GtkWidget *textedit;
  GtkWidget *frame11;
  GtkWidget *scrolledwindow41;
  GtkWidget *textstatus;
  GtkAccelGroup *accel_group;
  GtkTooltips *tooltips;
   char versiontitle[128];

  tooltips = gtk_tooltips_new ();
  accel_group = gtk_accel_group_new ();

  wmain = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (wmain, "wmain");
  gtk_object_set_data (GTK_OBJECT (wmain), "wmain", wmain);
  
  sprintf(versiontitle, "hf %s", PACKAGE_VERSION);

  gtk_window_set_title (GTK_WINDOW (wmain), _(versiontitle));
  gtk_window_set_position (GTK_WINDOW (wmain), GTK_WIN_POS_CENTER);
  gtk_window_set_policy (GTK_WINDOW (wmain), TRUE, TRUE, FALSE);
  gtk_window_set_wmclass (GTK_WINDOW (wmain), "hfterm", "hfterm");

  vbox_main = gtk_vbox_new (FALSE, 3);
  gtk_widget_set_name (vbox_main, "vbox_main");
  gtk_widget_ref (vbox_main);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "vbox_main", vbox_main,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox_main);
  gtk_container_add (GTK_CONTAINER (wmain), vbox_main);
  gtk_container_set_border_width (GTK_CONTAINER (vbox_main), 3);

  handlebox1 = gtk_handle_box_new ();
  gtk_widget_set_name (handlebox1, "handlebox1");
  gtk_widget_ref (handlebox1);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "handlebox1", handlebox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (handlebox1);
  gtk_box_pack_start (GTK_BOX (vbox_main), handlebox1, FALSE, TRUE, 0);

  menubar1 = gtk_menu_bar_new ();
  gtk_widget_set_name (menubar1, "menubar1");
  gtk_widget_ref (menubar1);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "menubar1", menubar1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (menubar1);
  gtk_container_add (GTK_CONTAINER (handlebox1), menubar1);

  file = gtk_menu_item_new_with_label (_("File"));
  gtk_widget_set_name (file, "file");
  gtk_widget_ref (file);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "file", file,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (file);
  gtk_container_add (GTK_CONTAINER (menubar1), file);

  file_menu = gtk_menu_new ();
  gtk_widget_set_name (file_menu, "file_menu");
  gtk_widget_ref (file_menu);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "file_menu", file_menu,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (file), file_menu);
  file_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (file_menu));

  quit = gtk_menu_item_new_with_label (_("Quit"));
  gtk_widget_set_name (quit, "quit");
  gtk_widget_ref (quit);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "quit", quit,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (quit);
  gtk_container_add (GTK_CONTAINER (file_menu), quit);
  gtk_widget_add_accelerator (quit, "activate", accel_group,
                              GDK_x, GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);

  rx_file = gtk_menu_item_new_with_label (_("Input  -> file"));
  gtk_widget_set_name (rx_file, "rx_file");
  gtk_widget_ref (rx_file);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "rx_file", rx_file,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (rx_file);
  gtk_container_add (GTK_CONTAINER (file_menu), rx_file);
  gtk_widget_add_accelerator (rx_file, "activate", accel_group,
                              GDK_i, GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);

  tx_file = gtk_menu_item_new_with_label (_("Output <- file"));
  gtk_widget_set_name (tx_file, "tx_file");
  gtk_widget_ref (tx_file);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "tx_file", tx_file,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (tx_file);
  gtk_container_add (GTK_CONTAINER (file_menu), tx_file);
  gtk_widget_add_accelerator (tx_file, "activate", accel_group,
                              GDK_o, GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);

  beacon_file = gtk_menu_item_new_with_label (_("Beacon"));
  gtk_widget_set_name (beacon_file, "beacon_file");
  gtk_widget_ref (beacon_file);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "beacon_file", beacon_file,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (beacon_file);
  gtk_container_add (GTK_CONTAINER (file_menu), beacon_file);
  gtk_widget_add_accelerator (beacon_file, "activate", accel_group,
                              GDK_b, GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);
  
  state = gtk_menu_item_new_with_label (_("State"));
  gtk_widget_set_name (state, "state");
  gtk_widget_ref (state);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "state", state,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (state);
  gtk_container_add (GTK_CONTAINER (menubar1), state);

  state_menu = gtk_menu_new ();
  gtk_widget_set_name (state_menu, "state_menu");
  gtk_widget_ref (state_menu);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "state_menu", state_menu,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (state), state_menu);
  state_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (state_menu));

  become_irs = gtk_menu_item_new_with_label (_("Become IRS"));
  gtk_widget_set_name (become_irs, "become_irs");
  gtk_widget_ref (become_irs);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "become_irs", become_irs,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (become_irs);
  gtk_container_add (GTK_CONTAINER (state_menu), become_irs);
  gtk_widget_add_accelerator (become_irs, "activate", accel_group,
                              GDK_r, GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);

  become_iss = gtk_menu_item_new_with_label (_("Become ISS"));
  gtk_widget_set_name (become_iss, "become_iss");
  gtk_widget_ref (become_iss);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "become_iss", become_iss,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (become_iss);
  gtk_container_add (GTK_CONTAINER (state_menu), become_iss);
  gtk_widget_add_accelerator (become_iss, "activate", accel_group,
                              GDK_t, GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);

  qrt = gtk_menu_item_new_with_label (_("QRT"));
  gtk_widget_set_name (qrt, "qrt");
  gtk_widget_ref (qrt);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "qrt", qrt,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (qrt);
  gtk_container_add (GTK_CONTAINER (state_menu), qrt);
  gtk_widget_add_accelerator (qrt, "activate", accel_group,
                              GDK_q, GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);

  speedup = gtk_menu_item_new_with_label (_("Speed Up"));
  gtk_widget_set_name (speedup, "speedup");
  gtk_widget_ref (speedup);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "speedup", speedup,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (speedup);
  gtk_container_add (GTK_CONTAINER (state_menu), speedup);
  gtk_widget_add_accelerator (speedup, "activate", accel_group,
                              GDK_s, GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);

  uppercase = gtk_menu_item_new_with_label (_("Uppercase"));
  gtk_widget_set_name (uppercase, "uppercase");
  gtk_widget_ref (uppercase);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "uppercase", uppercase,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (uppercase);
  gtk_container_add (GTK_CONTAINER (state_menu), uppercase);
  gtk_widget_add_accelerator (uppercase, "activate", accel_group,
                              GDK_u, GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);

  lowercase = gtk_menu_item_new_with_label (_("Lowercase"));
  gtk_widget_set_name (lowercase, "lowercase");
  gtk_widget_ref (lowercase);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "lowercase", lowercase,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (lowercase);
  gtk_container_add (GTK_CONTAINER (state_menu), lowercase);
  gtk_widget_add_accelerator (lowercase, "activate", accel_group,
                              GDK_l, GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);

  figurecase = gtk_menu_item_new_with_label (_("Figurecase"));
  gtk_widget_set_name (figurecase, "figurecase");
  gtk_widget_ref (figurecase);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "figurecase", figurecase,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (figurecase);
  gtk_container_add (GTK_CONTAINER (state_menu), figurecase);
  gtk_widget_add_accelerator (figurecase, "activate", accel_group,
                              GDK_f, GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);

  mode = gtk_menu_item_new_with_label (_("Mode"));
  gtk_widget_set_name (mode, "mode");
  gtk_widget_ref (mode);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "mode", mode,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (mode);
  gtk_container_add (GTK_CONTAINER (menubar1), mode);

  mode_menu = gtk_menu_new ();
  gtk_widget_set_name (mode_menu, "mode_menu");
  gtk_widget_ref (mode_menu);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "mode_menu", mode_menu,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (mode), mode_menu);
  mode_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (mode_menu));

/*
  helpoption = gtk_menu_item_new_with_label (_("Help"));
  gtk_widget_set_name (helpoption, "helpoption");
  gtk_widget_ref (helpoption);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "helpoption", helpoption,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (helpoption);
  gtk_container_add (GTK_CONTAINER (mode_menu), helpoption);
  gtk_widget_add_accelerator (helpoption, "activate", accel_group,
                              GDK_F1, 0,
                              GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (helpoption, "activate", accel_group,
                              GDK_h,   GDK_Control_R, 
                              GTK_ACCEL_VISIBLE);

  help_line = gtk_menu_item_new ();
  gtk_widget_set_name (help_line, "help_line");
  gtk_widget_ref (help_line);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "help_line", help_line,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (help_line);
  gtk_container_add (GTK_CONTAINER (mode_menu), help_line);
  gtk_widget_set_sensitive (help_line, FALSE);

  cw_line = gtk_menu_item_new ();
  gtk_widget_set_name (cw_line, "cw_line");
  gtk_widget_ref (cw_line);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "cw_line", cw_line,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (cw_line);
  gtk_container_add (GTK_CONTAINER (mode_menu), cw_line);
  gtk_widget_set_sensitive (cw_line, FALSE);
*/
  cw = gtk_menu_item_new_with_label (_("CW Elbug"));
  gtk_widget_set_name (cw, "cw");
  gtk_widget_ref (cw);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "cw", cw,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (cw);
  gtk_container_add (GTK_CONTAINER (mode_menu), cw);
  gtk_widget_add_accelerator (cw, "activate", accel_group,
                              GDK_F3, 0,
                              GTK_ACCEL_VISIBLE);

  rtty_line = gtk_menu_item_new ();
  gtk_widget_set_name (rtty_line, "rtty_line");
  gtk_widget_ref (rtty_line);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "rtty_line", rtty_line,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (rtty_line);
  gtk_container_add (GTK_CONTAINER (mode_menu), rtty_line);
  gtk_widget_set_sensitive (rtty_line, FALSE);

  rtty_receive = gtk_radio_menu_item_new_with_label 
      (mode_menu_group, _("RTTY Receive"));
  mode_menu_group = gtk_radio_menu_item_group 
      (GTK_RADIO_MENU_ITEM (rtty_receive));
  gtk_widget_set_name (rtty_receive, "rtty_receive");
  gtk_widget_ref (rtty_receive);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "rtty_receive", rtty_receive,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (rtty_receive);
  gtk_container_add (GTK_CONTAINER (mode_menu), rtty_receive);
  gtk_widget_add_accelerator (rtty_receive, "activate", accel_group,
      GDK_F4, 0,  GTK_ACCEL_VISIBLE);
  gtk_check_menu_item_set_show_toggle 
      (GTK_CHECK_MENU_ITEM (rtty_receive), TRUE);

  rtty_transmit = gtk_radio_menu_item_new_with_label 
      (mode_menu_group, _("RTTY Transmit"));
  mode_menu_group = gtk_radio_menu_item_group 
      (GTK_RADIO_MENU_ITEM (rtty_transmit));
  gtk_widget_set_name (rtty_transmit, "rtty_transmit");
  gtk_widget_ref (rtty_transmit);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "rtty_transmit", rtty_transmit,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (rtty_transmit);
  gtk_container_add (GTK_CONTAINER (mode_menu), rtty_transmit);
/* now needed for mt63
  gtk_widget_add_accelerator (rtty_transmit, "activate", accel_group,
      GDK_F12, 0, GTK_ACCEL_VISIBLE);
*/
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (rtty_transmit), TRUE);

  standby_line = gtk_menu_item_new ();
  gtk_widget_set_name (standby_line, "standby_line");
  gtk_widget_ref (standby_line);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "standby_line", standby_line,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (standby_line);
  gtk_container_add (GTK_CONTAINER (mode_menu), standby_line);
  gtk_widget_set_sensitive (standby_line, FALSE);

  standby = gtk_menu_item_new_with_label (_("Amtor Gtor Pactor Standby"));
  gtk_widget_set_name (standby, "standby");
  gtk_widget_ref (standby);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "standby", standby,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (standby);
  gtk_container_add (GTK_CONTAINER (mode_menu), standby);
  gtk_widget_add_accelerator (standby, "activate", accel_group,
                              GDK_F5, 0,
                              GTK_ACCEL_VISIBLE);

  standby_pactor = gtk_menu_item_new_with_label (_("Pactor Standby"));
  gtk_widget_set_name (standby_pactor, "standby_pactor");
  gtk_widget_ref (standby_pactor);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "standby_pactor", standby_pactor,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (standby_pactor);
  gtk_container_add (GTK_CONTAINER (mode_menu), standby_pactor);
/*
  not so often needed -> removed for cw
  gtk_widget_add_accelerator (standby_pactor, "activate", accel_group,
                              GDK_F4, 0,
                              GTK_ACCEL_VISIBLE);
*/
  amtor_line = gtk_menu_item_new ();
  gtk_widget_set_name (amtor_line, "amtor_line");
  gtk_widget_ref (amtor_line);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "amtor_line", amtor_line,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (amtor_line);
  gtk_container_add (GTK_CONTAINER (mode_menu), amtor_line);
  gtk_widget_set_sensitive (amtor_line, FALSE);

  amtor_collective_fec = gtk_radio_menu_item_new_with_label (mode_menu_group, _("Amtor Collective FEC"));
  mode_menu_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (amtor_collective_fec));
  gtk_widget_set_name (amtor_collective_fec, "amtor_collective_fec");
  gtk_widget_ref (amtor_collective_fec);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "amtor_collective_fec", amtor_collective_fec,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (amtor_collective_fec);
  gtk_container_add (GTK_CONTAINER (mode_menu), amtor_collective_fec);
  gtk_widget_add_accelerator (amtor_collective_fec, "activate", accel_group,
                              GDK_F6, 0,
                              GTK_ACCEL_VISIBLE);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (amtor_collective_fec), TRUE);

  amtor_selective_fec = gtk_radio_menu_item_new_with_label (mode_menu_group, _("Amtor Selective FEC"));
  mode_menu_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (amtor_selective_fec));
  gtk_widget_set_name (amtor_selective_fec, "amtor_selective_fec");
  gtk_widget_ref (amtor_selective_fec);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "amtor_selective_fec", amtor_selective_fec,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (amtor_selective_fec);
  gtk_container_add (GTK_CONTAINER (mode_menu), amtor_selective_fec);
  gtk_widget_add_accelerator (amtor_selective_fec, "activate", accel_group,
                              GDK_F7, 0,
                              GTK_ACCEL_VISIBLE);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (amtor_selective_fec), TRUE);

  amtor_arq = gtk_radio_menu_item_new_with_label (mode_menu_group, _("Amtor ARQ"));
  mode_menu_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (amtor_arq));
  gtk_widget_set_name (amtor_arq, "amtor_arq");
  gtk_widget_ref (amtor_arq);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "amtor_arq", amtor_arq,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (amtor_arq);
  gtk_container_add (GTK_CONTAINER (mode_menu), amtor_arq);
  gtk_widget_add_accelerator (amtor_arq, "activate", accel_group,
                              GDK_F8, 0,
                              GTK_ACCEL_VISIBLE);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (amtor_arq), TRUE);

  gtor_arq1 = gtk_radio_menu_item_new_with_label (mode_menu_group, _("GTOR ARQ"));
  mode_menu_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (gtor_arq1));
  gtk_widget_set_name (gtor_arq1, "gtor_arq1");
  gtk_widget_ref (gtor_arq1);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "gtor_arq1", gtor_arq1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (gtor_arq1);
  gtk_container_add (GTK_CONTAINER (mode_menu), gtor_arq1);
  gtk_widget_add_accelerator (gtor_arq1, "activate", accel_group,
                              GDK_F9, 0,
                              GTK_ACCEL_VISIBLE);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (gtor_arq1), TRUE);

  pactor_fec = gtk_radio_menu_item_new_with_label (mode_menu_group, _("Pactor FEC"));
  mode_menu_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (pactor_fec));
  gtk_widget_set_name (pactor_fec, "pactor_fec");
  gtk_widget_ref (pactor_fec);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "pactor_fec", pactor_fec,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (pactor_fec);
  gtk_container_add (GTK_CONTAINER (mode_menu), pactor_fec);
  gtk_widget_add_accelerator (pactor_fec, "activate", accel_group,
                              GDK_F10, 0,
                              GTK_ACCEL_VISIBLE);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (pactor_fec), TRUE);

  pactor_arq = gtk_radio_menu_item_new_with_label (mode_menu_group, _("Pactor ARQ"));
  mode_menu_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (pactor_arq));
  gtk_widget_set_name (pactor_arq, "pactor_arq");
  gtk_widget_ref (pactor_arq);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "pactor_arq", pactor_arq,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (pactor_arq);
  gtk_container_add (GTK_CONTAINER (mode_menu), pactor_arq);
  gtk_widget_add_accelerator (pactor_arq, "activate", accel_group,
                              GDK_F11, 0,
                              GTK_ACCEL_VISIBLE);
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (pactor_arq), TRUE);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (pactor_arq), TRUE);

  menulinemt63 = gtk_menu_item_new ();
  gtk_widget_set_name (menulinemt63, "menulinemt63");
  gtk_widget_ref (menulinemt63);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "menulinemt63", menulinemt63,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (menulinemt63);
  gtk_container_add (GTK_CONTAINER (mode_menu), menulinemt63);
  gtk_widget_set_sensitive (menulinemt63, FALSE);

  mt63_rx = gtk_radio_menu_item_new_with_label 
      (mode_menu_group, _("MT63 Receive"));
  mode_menu_group = gtk_radio_menu_item_group 
      (GTK_RADIO_MENU_ITEM (mt63_rx));
  gtk_widget_set_name (mt63_rx, "mt63_rx");
  gtk_widget_ref (mt63_rx);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "mt63_rx", mt63_rx,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (mt63_rx);
  gtk_container_add (GTK_CONTAINER (mode_menu), mt63_rx);
  gtk_widget_add_accelerator (mt63_rx, "activate", accel_group,
      GDK_F12, 0,  GTK_ACCEL_VISIBLE);
  gtk_check_menu_item_set_show_toggle 
      (GTK_CHECK_MENU_ITEM (mt63_rx), TRUE);

  mt63_tx = gtk_radio_menu_item_new_with_label 
      (mode_menu_group, _("MT63 Transmit"));
  mode_menu_group = gtk_radio_menu_item_group 
      (GTK_RADIO_MENU_ITEM (mt63_tx));
  gtk_widget_set_name (mt63_tx, "mt63_tx");
  gtk_widget_ref (mt63_tx);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "mt63_tx", mt63_tx,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (mt63_tx);
  gtk_container_add (GTK_CONTAINER (mode_menu), mt63_tx);
/* 
  gtk_widget_add_accelerator (mt63_tx, "activate", accel_group,
      GDK_F12, 0, GTK_ACCEL_VISIBLE);
*/
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (mt63_tx), TRUE);
  
  spectrum = gtk_menu_item_new_with_label (_("Spectrum"));
  gtk_widget_set_name (spectrum, "spectrum");
  gtk_widget_ref (spectrum);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "spectrum", spectrum,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (spectrum);
  gtk_container_add (GTK_CONTAINER (menubar1), spectrum);
  gtk_tooltips_set_tip (tooltips, spectrum, _("Left mousekey: Mark, Right: Squelch. Quit with Escape."), NULL);
  gtk_widget_add_accelerator (spectrum, "activate", accel_group,
                              GDK_F2, 0,
                              GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (spectrum, "activate", accel_group,
                              GDK_f, GDK_Control_R ,
                              GTK_ACCEL_VISIBLE);

  monitor = gtk_menu_item_new_with_label (_("Monitor"));
  gtk_widget_set_name (monitor, "monitor");
  gtk_widget_ref (monitor);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "monitor", monitor,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (monitor);
  gtk_container_add (GTK_CONTAINER (menubar1), monitor);
  gtk_tooltips_set_tip (tooltips, monitor, _("<strg>+M, verbose output of de- / encoders"), NULL);
  gtk_widget_add_accelerator (monitor, "activate", accel_group,
                              GDK_m, GDK_Control_R ,
                              GTK_ACCEL_VISIBLE);

  Wfixtext = gtk_menu_item_new_with_label (_("Fixtexts"));
  gtk_widget_set_name (Wfixtext, "Wfixtext");
  gtk_widget_ref (Wfixtext);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "Wfixtext", Wfixtext,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (Wfixtext);
  gtk_container_add (GTK_CONTAINER (menubar1), Wfixtext);
  gtk_tooltips_set_tip (tooltips, Wfixtext, _("<strg>+T, You can edit 12 text macros here."), NULL);
  gtk_widget_add_accelerator (Wfixtext, "activate", accel_group,
                              GDK_t, GDK_Control_R ,
                              GTK_ACCEL_VISIBLE);

  parameters = gtk_menu_item_new_with_label (_("Config"));
  gtk_widget_set_name (parameters, "parameters");
  gtk_widget_ref (parameters);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "parameters", parameters,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (parameters);
  gtk_container_add (GTK_CONTAINER (menubar1), parameters);
  gtk_tooltips_set_tip (tooltips, parameters, _("<strg>+P, Global and per-mode settings e.g. baudrates..."), NULL);
  gtk_widget_add_accelerator (parameters, "activate", accel_group,
                              GDK_p, GDK_Control_R ,
                              GTK_ACCEL_VISIBLE);

/*
  personal_edit1 = gtk_menu_item_new_with_label (_("My Station"));
  gtk_widget_set_name (personal_edit1, "personal_edit1");
  gtk_widget_ref (personal_edit1);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "personal_edit1", personal_edit1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (personal_edit1);
  gtk_container_add (GTK_CONTAINER (menubar1), personal_edit1);
  gtk_tooltips_set_tip (tooltips, personal_edit1, _("<strg>+B, \"brag\", Your station's call, qth, antenna, rig, power and so on"), NULL);
  gtk_widget_add_accelerator (personal_edit1, "activate", accel_group,
                              GDK_b, GDK_Control_R ,
                              GTK_ACCEL_VISIBLE);

*/
  logbook1 = gtk_menu_item_new_with_label (_("Logbook"));
  gtk_widget_set_name (logbook1, "logbook1");
  gtk_widget_ref (logbook1);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "logbook1", logbook1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (logbook1);
  gtk_container_add (GTK_CONTAINER (menubar1), logbook1);

  logbook1_menu = gtk_menu_new ();
  gtk_widget_set_name (logbook1_menu, "logbook1_menu");
  gtk_widget_ref (logbook1_menu);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "logbook1_menu", logbook1_menu,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (logbook1), logbook1_menu);
  logbook1_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (logbook1_menu));

  new_logentry = gtk_menu_item_new_with_label (_("New Entry"));
  gtk_widget_set_name (new_logentry, "new_logentry");
  gtk_widget_ref (new_logentry);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "new_logentry", new_logentry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (new_logentry);
  gtk_container_add (GTK_CONTAINER (logbook1_menu), new_logentry);
  gtk_widget_add_accelerator (new_logentry, "activate", accel_group,
                              GDK_n, GDK_Control_R ,
                              GTK_ACCEL_VISIBLE);

  save_logentry = gtk_menu_item_new_with_label (_("Save Entry"));
  gtk_widget_set_name (save_logentry, "save_logentry");
  gtk_widget_ref (save_logentry);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "save_logentry", save_logentry,
                            (GtkDestroyNotify) gtk_widget_unref);
//  gtk_widget_show (save_logentry);
//  nur bei log open allways  
  gtk_container_add (GTK_CONTAINER (logbook1_menu), save_logentry);
  gtk_widget_add_accelerator (save_logentry, "activate", accel_group,
                              GDK_s, GDK_Control_R ,
                              GTK_ACCEL_VISIBLE);

  clear_logentry = gtk_menu_item_new_with_label (_("Clear Entry"));
  gtk_widget_set_name (clear_logentry, "clear_logentry");
  gtk_widget_ref (clear_logentry);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "clear_logentry", clear_logentry,
                            (GtkDestroyNotify) gtk_widget_unref);
//  gtk_widget_show (clear_logentry);
//  nur bei log open allways
  gtk_container_add (GTK_CONTAINER (logbook1_menu), clear_logentry);
  gtk_widget_add_accelerator (clear_logentry, "activate", accel_group,
                              GDK_c, GDK_Control_R ,
                              GTK_ACCEL_VISIBLE);

  search_entry1 = gtk_menu_item_new_with_label (_("Old Entry"));
  gtk_widget_set_name (search_entry1, "search_entry1");
  gtk_widget_ref (search_entry1);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "search_entry1", search_entry1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (search_entry1);
  gtk_container_add (GTK_CONTAINER (logbook1_menu), search_entry1);
  gtk_widget_add_accelerator (search_entry1, "activate", accel_group,
                              GDK_o, GDK_Control_R ,
                              GTK_ACCEL_VISIBLE);

  list_all_entrys1 = gtk_menu_item_new_with_label (_("List all Entries"));
  gtk_widget_set_name (list_all_entrys1, "list_all_entrys1");
  gtk_widget_ref (list_all_entrys1);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "list_all_entrys1", list_all_entrys1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (list_all_entrys1);
  gtk_container_add (GTK_CONTAINER (logbook1_menu), list_all_entrys1);
  gtk_widget_add_accelerator (list_all_entrys1, "activate", accel_group,
                              GDK_l, GDK_Control_R ,
                              GTK_ACCEL_VISIBLE);

  line6 = gtk_menu_item_new ();
  gtk_widget_set_name (line6, "line6");
  gtk_widget_ref (line6);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "line6", line6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (line6);
  gtk_container_add (GTK_CONTAINER (logbook1_menu), line6);
  gtk_widget_set_sensitive (line6, FALSE);
  
  archivate = gtk_menu_item_new_with_label (_("Archivate Log"));
  gtk_widget_set_name (archivate, "archivate");
  gtk_widget_ref (archivate);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "archivate", archivate,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (archivate);
  gtk_container_add (GTK_CONTAINER (logbook1_menu), archivate);
  gtk_widget_add_accelerator (archivate, "activate", accel_group,
                              GDK_a, GDK_Control_R ,
                              GTK_ACCEL_VISIBLE);

  line7 = gtk_menu_item_new ();
  gtk_widget_set_name (line7, "line7");
  gtk_widget_ref (line7);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "line7", line7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (line7);
  gtk_container_add (GTK_CONTAINER (logbook1_menu), line7);
  gtk_widget_set_sensitive (line7, FALSE);

  log_window_show = gtk_menu_item_new_with_label (_("QSO Window"));
  gtk_widget_set_name (log_window_show, "log_window_show");
  gtk_widget_ref (log_window_show);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "log_window_show", log_window_show,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (log_window_show);
  gtk_container_add (GTK_CONTAINER (logbook1_menu), log_window_show);
  gtk_widget_add_accelerator (log_window_show, "activate", accel_group,
                              GDK_q, GDK_Control_R ,
                              GTK_ACCEL_VISIBLE);

  line10 = gtk_menu_item_new ();
  gtk_widget_set_name (line10, "line10");
  gtk_widget_ref (line10);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "line10", line10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (line10);
  gtk_container_add (GTK_CONTAINER (logbook1_menu), line10);
  gtk_widget_set_sensitive (line10, FALSE);

  mailbox = gtk_menu_item_new_with_label (_("Mailbox"));
  gtk_widget_set_name (mailbox, "mailbox");
  gtk_widget_ref (mailbox);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "mailbox", mailbox,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (mailbox);
  gtk_container_add (GTK_CONTAINER (menubar1), mailbox);
  gtk_tooltips_set_tip (tooltips, mailbox, _("<alt>+M, Toggles mailbox mode on/off"), NULL);
  gtk_widget_add_accelerator (mailbox, "activate", accel_group,
                              GDK_m, GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);

/* not yet implemented */
/*
  map1 = gtk_menu_item_new_with_label (_("World Map"));
  gtk_widget_set_name (map1, "map1");
  gtk_widget_ref (map1);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "map1", map1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (map1);
  gtk_container_add (GTK_CONTAINER (logbook1_menu), map1);
  gtk_widget_add_accelerator (map1, "activate", accel_group,
                              GDK_w, GDK_Control_R,
                              GTK_ACCEL_VISIBLE);
*/

  help = gtk_menu_item_new_with_label (_("?"));
  gtk_widget_set_name (help, "help");
  gtk_widget_ref (help);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "help", help,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (help);
  gtk_container_add (GTK_CONTAINER (menubar1), help);
  gtk_menu_item_right_justify (GTK_MENU_ITEM (help));

  help_menu = gtk_menu_new ();
  gtk_widget_set_name (help_menu, "help_menu");
  gtk_widget_ref (help_menu);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "help_menu", help_menu,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (help), help_menu);
  help_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (help_menu));

  about = gtk_menu_item_new_with_label (_("About"));
  gtk_widget_set_name (about, "about");
  gtk_widget_ref (about);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "about", about,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (about);
  gtk_container_add (GTK_CONTAINER (help_menu), about);

  index1 = gtk_menu_item_new_with_label (_("Help"));
  gtk_widget_set_name (index1, "index1");
  gtk_widget_ref (index1);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "index1", index1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (index1);
  gtk_container_add (GTK_CONTAINER (help_menu), index1);
  gtk_widget_add_accelerator (index1, "activate", accel_group,
                              GDK_h,   GDK_Control_R, 
                              GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (index1, "activate", accel_group,
                              GDK_F1, 0,
                              GTK_ACCEL_VISIBLE);

  logbook_frame = gtk_frame_new (_("Log Book"));
  gtk_widget_set_name (logbook_frame, "logbook_frame");
  gtk_widget_ref (logbook_frame);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "logbook_frame", logbook_frame,
                            (GtkDestroyNotify) gtk_widget_unref);
//  gtk_widget_show (logbook_frame);
//  in Men verlagert!
  gtk_box_pack_start (GTK_BOX (vbox_main), logbook_frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (logbook_frame), 3);

  table7 = gtk_table_new (2, 8, FALSE);
  gtk_widget_set_name (table7, "table7");
  gtk_widget_ref (table7);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "table7", table7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table7);
  gtk_container_add (GTK_CONTAINER (logbook_frame), table7);
  gtk_container_set_border_width (GTK_CONTAINER (table7), 3);

  entry_rstin = gtk_entry_new_with_max_length (6);
  gtk_widget_set_name (entry_rstin, "entry_rstin");
  gtk_widget_ref (entry_rstin);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "entry_rstin", entry_rstin,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry_rstin);
  gtk_table_attach (GTK_TABLE (table7), entry_rstin, 7, 8, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_widget_set_usize (entry_rstin, 40, -2);
  gtk_tooltips_set_tip (tooltips, entry_rstin, _("Enter the RST you get ..."), NULL);

  Call_label = gtk_label_new (_("Call"));
  gtk_widget_set_name (Call_label, "Call_label");
  gtk_widget_ref (Call_label);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "Call_label", Call_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (Call_label);
  gtk_table_attach (GTK_TABLE (table7), Call_label, 0, 1, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_misc_set_alignment (GTK_MISC (Call_label), 0, 0.5);

  QTH_label = gtk_label_new (_("QTH"));
  gtk_widget_set_name (QTH_label, "QTH_label");
  gtk_widget_ref (QTH_label);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "QTH_label", QTH_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (QTH_label);
  gtk_table_attach (GTK_TABLE (table7), QTH_label, 4, 5, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_misc_set_alignment (GTK_MISC (QTH_label), 0, 0.5);

  Rcvd_label = gtk_label_new (_("Rcvd"));
  gtk_widget_set_name (Rcvd_label, "Rcvd_label");
  gtk_widget_ref (Rcvd_label);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "Rcvd_label", Rcvd_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (Rcvd_label);
  gtk_table_attach (GTK_TABLE (table7), Rcvd_label, 6, 7, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_misc_set_alignment (GTK_MISC (Rcvd_label), 0, 0.5);

  Mode_label = gtk_label_new (_("Mode"));
  gtk_widget_set_name (Mode_label, "Mode_label");
  gtk_widget_ref (Mode_label);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "Mode_label", Mode_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (Mode_label);
  gtk_table_attach (GTK_TABLE (table7), Mode_label, 2, 3, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_misc_set_alignment (GTK_MISC (Mode_label), 0, 0.5);

  Band_label = gtk_label_new (_("Band"));
  gtk_widget_set_name (Band_label, "Band_label");
  gtk_widget_ref (Band_label);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "Band_label", Band_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (Band_label);
  gtk_table_attach (GTK_TABLE (table7), Band_label, 2, 3, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_misc_set_alignment (GTK_MISC (Band_label), 0, 0.5);

  Note_label = gtk_label_new (_("Notes"));
  gtk_widget_set_name (Note_label, "Note_label");
  gtk_widget_ref (Note_label);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "Note_label", Note_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (Note_label);
  gtk_table_attach (GTK_TABLE (table7), Note_label, 4, 5, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_misc_set_alignment (GTK_MISC (Note_label), 0, 0.5);

  Send_label = gtk_label_new (_("Send"));
  gtk_widget_set_name (Send_label, "Send_label");
  gtk_widget_ref (Send_label);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "Send_label", Send_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (Send_label);
  gtk_table_attach (GTK_TABLE (table7), Send_label, 6, 7, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_misc_set_alignment (GTK_MISC (Send_label), 0, 0.5);

  entry_call =  gtk_entry_new_with_max_length (15);
  gtk_widget_set_name (entry_call, "entry_call");
  gtk_widget_ref (entry_call);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "entry_call", entry_call,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry_call);
  gtk_table_attach (GTK_TABLE (table7), entry_call, 1, 2, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_widget_set_usize (entry_call, 91, -2);
  gtk_tooltips_set_tip (tooltips, entry_call, _("Call of your QSO-Partner"), NULL);

  entry_name =   gtk_entry_new_with_max_length (31);
  gtk_widget_set_name (entry_name, "entry_name");
  gtk_widget_ref (entry_name);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "entry_name", entry_name,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry_name);
  gtk_table_attach (GTK_TABLE (table7), entry_name, 1, 2, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_widget_set_usize (entry_name, 91, -2);
  gtk_tooltips_set_tip (tooltips, entry_name, _("Enter the Name of your QSO-Partner"), NULL);

  combo1 = gtk_combo_new ();
  gtk_widget_set_name (combo1, "combo1");
  gtk_widget_ref (combo1);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "combo1", combo1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (combo1);
  gtk_table_attach (GTK_TABLE (table7), combo1, 3, 4, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_widget_set_usize (combo1, 91, -2);
  combo1_items = g_list_append (combo1_items, (gpointer) _("CW"));
  combo1_items = g_list_append (combo1_items, (gpointer) _("RTTY"));
  combo1_items = g_list_append (combo1_items, (gpointer) _("Amtor"));
  combo1_items = g_list_append (combo1_items, (gpointer) _("Gtor"));
  combo1_items = g_list_append (combo1_items, (gpointer) _("Pactor"));
  combo1_items = g_list_append (combo1_items, (gpointer) _("MT63"));
  gtk_combo_set_popdown_strings (GTK_COMBO (combo1), combo1_items);
  g_list_free (combo1_items);

  combo_entry_mode = GTK_COMBO (combo1)->entry;
  gtk_widget_set_name (combo_entry_mode, "combo_entry_mode");
  gtk_widget_ref (combo_entry_mode);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "combo_entry_mode", combo_entry_mode,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (combo_entry_mode);
  gtk_entry_set_editable (GTK_ENTRY (combo_entry_mode), TRUE);
  gtk_entry_set_text (GTK_ENTRY (combo_entry_mode), _("Pactor"));

  combo2 = gtk_combo_new ();
  gtk_widget_set_name (combo2, "combo2");
  gtk_widget_ref (combo2);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "combo2", combo2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (combo2);
  gtk_table_attach (GTK_TABLE (table7), combo2, 3, 4, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_widget_set_usize (combo2, 91, -2);
  combo2_items = g_list_append (combo2_items, (gpointer) _("160m"));
  combo2_items = g_list_append (combo2_items, (gpointer) _("80m"));
  combo2_items = g_list_append (combo2_items, (gpointer) _("40m"));
  combo2_items = g_list_append (combo2_items, (gpointer) _("30m"));
  combo2_items = g_list_append (combo2_items, (gpointer) _("20m"));
  combo2_items = g_list_append (combo2_items, (gpointer) _("17m"));
  combo2_items = g_list_append (combo2_items, (gpointer) _("15m"));
  combo2_items = g_list_append (combo2_items, (gpointer) _("12m"));
  combo2_items = g_list_append (combo2_items, (gpointer) _("10m"));
  combo2_items = g_list_append (combo2_items, (gpointer) _("6m"));
  combo2_items = g_list_append (combo2_items, (gpointer) _("2m"));
  combo2_items = g_list_append (combo2_items, (gpointer) _("70cm"));
  combo2_items = g_list_append (combo2_items, (gpointer) _("23cm"));
  gtk_combo_set_popdown_strings (GTK_COMBO (combo2), combo2_items);
  g_list_free (combo2_items);

  combo_entry_band = GTK_COMBO (combo2)->entry;
  gtk_widget_set_name (combo_entry_band, "combo_entry_band");
  gtk_widget_ref (combo_entry_band);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "combo_entry_band", combo_entry_band,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (combo_entry_band);
  gtk_entry_set_editable (GTK_ENTRY (combo_entry_band), TRUE);
  gtk_entry_set_text (GTK_ENTRY (combo_entry_band), _("20 m"));

  entry_notes =  gtk_entry_new_with_max_length (63);
  gtk_widget_set_name (entry_notes, "entry_notes");
  gtk_widget_ref (entry_notes);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "entry_notes", entry_notes,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry_notes);
  gtk_table_attach (GTK_TABLE (table7), entry_notes, 5, 6, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 3, 3);
//  gtk_widget_set_usize (entry_notes, 300, -2);
  gtk_tooltips_set_tip (tooltips, entry_notes, _("... perhaps Locator or anything alse"), NULL);

  entry_rstout = gtk_entry_new_with_max_length (6);
  gtk_widget_set_name (entry_rstout, "entry_rstout");
  gtk_widget_ref (entry_rstout);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "entry_rstout", entry_rstout,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry_rstout);
  gtk_table_attach (GTK_TABLE (table7), entry_rstout, 7, 8, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_widget_set_usize (entry_rstout, 40, -2);
  gtk_tooltips_set_tip (tooltips, entry_rstout, _("Enter the RST you give"), NULL);

  Name_label = gtk_label_new (_("Name"));
  gtk_widget_set_name (Name_label, "Name_label");
  gtk_widget_ref (Name_label);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "Name_label", Name_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (Name_label);
  gtk_table_attach (GTK_TABLE (table7), Name_label,  0, 1, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_misc_set_alignment (GTK_MISC (Name_label), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (Name_label), 3, 3);

  entry_qth =  gtk_entry_new_with_max_length (31);
  gtk_widget_set_name (entry_qth, "entry_qth");
  gtk_widget_ref (entry_qth);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "entry_qth", entry_qth,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry_qth);
  gtk_table_attach (GTK_TABLE (table7), entry_qth, 5, 6, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 3, 3);
//  gtk_widget_set_usize (entry_qth, 300, -2);
  gtk_tooltips_set_tip (tooltips, entry_qth, _("QTH of your QSO-Partner"), NULL);
  
  log_win_ok_button = gtk_button_new_with_label (_("OK & Save"));
  gtk_widget_set_name (log_win_ok_button, "log_win_ok_button");
  gtk_widget_ref (log_win_ok_button);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "log_win_ok_button", log_win_ok_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (log_win_ok_button);
  gtk_table_attach (GTK_TABLE (table7), log_win_ok_button, 9, 10, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_widget_set_usize (log_win_ok_button, 90, -2);
  gtk_tooltips_set_tip (tooltips, log_win_ok_button, _("Save log entry"), NULL);

  log_win_cancel_button = gtk_button_new_with_label (_("Clear"));
  gtk_widget_set_name (log_win_cancel_button, "log_win_cancel_button");
  gtk_widget_ref (log_win_cancel_button);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "log_win_cancel_button", log_win_cancel_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (log_win_cancel_button);
  gtk_table_attach (GTK_TABLE (table7), log_win_cancel_button, 9, 10, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_widget_set_usize (log_win_cancel_button, 90, -2);
  gtk_tooltips_set_tip (tooltips, log_win_cancel_button, _("Clear this log entry"), NULL);
  
  hbox_big_downside = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (hbox_big_downside, "hbox_big_downside");
  gtk_widget_ref (hbox_big_downside);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "hbox_big_downside", hbox_big_downside,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox_big_downside);
  gtk_box_pack_start (GTK_BOX (vbox_main), hbox_big_downside, TRUE, TRUE, 0);
  
  leftbuttonframe = gtk_frame_new (_("Mode: (F...)"));
  gtk_widget_set_name (leftbuttonframe, "leftbuttonframe");
  gtk_widget_ref (leftbuttonframe);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "leftbuttonframe", leftbuttonframe,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (leftbuttonframe);
  gtk_box_pack_start (GTK_BOX (hbox_big_downside), leftbuttonframe, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (leftbuttonframe), 3);

  vboxmode = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (vboxmode, "vboxmode");
  gtk_widget_ref (vboxmode);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "vboxmode", vboxmode,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vboxmode);
  gtk_container_add (GTK_CONTAINER (leftbuttonframe), vboxmode); 
  gtk_container_set_border_width (GTK_CONTAINER (vboxmode), 3);

  helpbutton = gtk_button_new_with_label (_(" 1: Help"));
  gtk_widget_set_name (helpbutton, "helpbutton");
  gtk_widget_ref (helpbutton);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "helpbutton", helpbutton,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (helpbutton);
  gtk_box_pack_start (GTK_BOX (vboxmode), helpbutton, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (helpbutton), 3);
  gtk_tooltips_set_tip (tooltips, helpbutton, _("<strg>+H, Help: translations welcome !!!! "), NULL);
  
  spectrumbutton = gtk_button_new_with_label (_(" 2: Spectrum"));
  gtk_widget_set_name (spectrumbutton, "spectrumbutton");
  gtk_widget_ref (spectrumbutton);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "spectrumbutton", spectrumbutton,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (spectrumbutton);
  gtk_box_pack_start (GTK_BOX (vboxmode), spectrumbutton, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (spectrumbutton), 3);
  gtk_tooltips_set_tip (tooltips, spectrumbutton, _("<strg>+F, Left mousekey: Mark, Right: Squelch. Quit with Escape."), NULL);

  CW_button = gtk_button_new_with_label (_(" 3: CW-ELBUG"));
  gtk_widget_set_name (CW_button, "CW_button");
  gtk_widget_ref (CW_button);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "CW_button", CW_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (CW_button);
  gtk_box_pack_start (GTK_BOX (vboxmode), CW_button, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (CW_button), 3);
  gtk_tooltips_set_tip (tooltips, CW_button, _("CW Mouse Elbug. Unique by DL4MGE. Click into the TX text window..."), NULL);

  RTTY_RX_button = gtk_button_new_with_label (_(" 4: RTTY"));
  gtk_widget_set_name (RTTY_RX_button, "RTTY_RX_button");
  gtk_widget_ref (RTTY_RX_button);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "RTTY_RX_button", RTTY_RX_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (RTTY_RX_button);
  gtk_box_pack_start (GTK_BOX (vboxmode), RTTY_RX_button, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (RTTY_RX_button), 3);
  gtk_tooltips_set_tip (tooltips, RTTY_RX_button, _("RTTY RX. With Alt-U, -L, -F you can toggle upper, lower, figure case.\n45 baud is default for ham radio. You can also try other rates (see parameter menu <Strg>+<P>) for wx stations etc."), NULL);

/*
  RTTY_TX_button = gtk_button_new_with_label (_("12: RTTY TX"));
  gtk_widget_set_name (RTTY_TX_button, "RTTY_TX_button");
  gtk_widget_ref (RTTY_TX_button);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "RTTY_TX_button", RTTY_TX_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (RTTY_TX_button);
  gtk_box_pack_start (GTK_BOX (vboxmode), RTTY_TX_button, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (RTTY_TX_button), 3);
  gtk_tooltips_set_tip (tooltips, RTTY_TX_button, _("RTTY Transmit. 45 baud is default. \nIn Parameter menu (<Strg>+<P>) you can change baudrate and mark&space and invert."), NULL);
*/

  standbybutton = gtk_button_new_with_label (_(" 5: Standby"));
  gtk_widget_set_name (standbybutton, "standbybutton");
  gtk_widget_ref (standbybutton);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "standbybutton", standbybutton,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (standbybutton);
  gtk_box_pack_start (GTK_BOX (vboxmode), standbybutton, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (standbybutton), 3);
  gtk_tooltips_set_tip (tooltips, standbybutton, _("Standby for Pactor, Amtor, Gtor 100-300 Baud."), NULL);

/*
  standby_pactorbutton = gtk_button_new_with_label (_(" 4: Pactor Standby"));
  gtk_widget_set_name (standby_pactorbutton, "standby_pactorbutton");
  gtk_widget_ref (standby_pactorbutton);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "standby_pactorbutton", standby_pactorbutton,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (standby_pactorbutton);
  gtk_box_pack_start (GTK_BOX (vboxmode), standby_pactorbutton, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (standby_pactorbutton), 3);
  gtk_tooltips_set_tip (tooltips, standby_pactorbutton, _("Standby for Pactor only. To see still more, see monitor with Shft-M."), NULL);
*/

  A_CFEC_button = gtk_button_new_with_label (_(" 6: Amtor-C-FEC"));
  gtk_widget_set_name (A_CFEC_button, "A_CFEC_button");
  gtk_widget_ref (A_CFEC_button);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "A_CFEC_button", A_CFEC_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (A_CFEC_button);
  gtk_box_pack_start (GTK_BOX (vboxmode), A_CFEC_button, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (A_CFEC_button), 3);
  gtk_tooltips_set_tip (tooltips, A_CFEC_button, _("Amtor Collective FEC, e.g. for calling CQ in Amtor."), NULL);
  
  A_SFEC_button = gtk_button_new_with_label (_(" 7: Amtor S-FEC"));
  gtk_widget_set_name (A_SFEC_button, "A_SFEC_button");
  gtk_widget_ref (A_SFEC_button);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "A_SFEC_button", A_SFEC_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (A_SFEC_button);
  gtk_box_pack_start (GTK_BOX (vboxmode), A_SFEC_button, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (A_SFEC_button), 3);
  gtk_tooltips_set_tip (tooltips, A_SFEC_button, _("calls the Amtor selective FEC call (see Parameter Menu, <Strg>+<P>)"), NULL);

  A_ARQ_button = gtk_button_new_with_label (_(" 8: Amtor ARQ"));
  gtk_widget_set_name (A_ARQ_button, "A_ARQ_button");
  gtk_widget_ref (A_ARQ_button);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "A_ARQ_button", A_ARQ_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start (GTK_BOX (vboxmode), A_ARQ_button, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (A_ARQ_button), 3);
  gtk_tooltips_set_tip (tooltips, A_ARQ_button, _("Amtor ARQ. Hope you entered your and destiny call in Parameter menu (<Strg>+<P>)."), NULL);
  gtk_widget_show (A_ARQ_button);

  GTOR_button = gtk_button_new_with_label (_(" 9: GTOR"));
  gtk_widget_set_name (GTOR_button, "GTOR_button");
  gtk_widget_ref (GTOR_button);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "GTOR_button", GTOR_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (GTOR_button);
  gtk_box_pack_start (GTK_BOX (vboxmode), GTOR_button, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (GTOR_button), 3);
  gtk_tooltips_set_tip (tooltips, GTOR_button, _("please, send me docs about this mode, thanks, Gnther!"), NULL);

  P_FEC_button = gtk_button_new_with_label (_("10: Pactor FEC"));
  gtk_widget_set_name (P_FEC_button, "P_FEC_button");
  gtk_widget_ref (P_FEC_button);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "P_FEC_button", P_FEC_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (P_FEC_button);
  gtk_box_pack_start (GTK_BOX (vboxmode), P_FEC_button, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (P_FEC_button), 3);
  gtk_tooltips_set_tip (tooltips, P_FEC_button, _("Pactor FEC means without confirm packets. So you can call CQ in Pactor"), NULL);

  P_ARQ_button = gtk_button_new_with_label (_("11: Pactor ARQ"));
  gtk_widget_set_name (P_ARQ_button, "P_ARQ_button");
  gtk_widget_ref (P_ARQ_button);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "P_ARQ_button", P_ARQ_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (P_ARQ_button);
  gtk_box_pack_start (GTK_BOX (vboxmode), P_ARQ_button, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (P_ARQ_button), 3);
  gtk_tooltips_set_tip (tooltips, P_ARQ_button, _("Pactor ARQ. Hope you entered your call and destiny call into parameter menu (<Strg>+<P>) !"), NULL);

  MT63_button = gtk_button_new_with_label (_("12: MT63"));
  gtk_widget_set_name (MT63_button, "MT63_button");
  gtk_widget_ref (MT63_button);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "MT63_button", MT63_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (MT63_button);
  gtk_box_pack_start (GTK_BOX (vboxmode), MT63_button, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (MT63_button), 3);
  gtk_tooltips_set_tip (tooltips, MT63_button, _("Pawel Jalocha's MT63.\nIn Parameter menu (<Strg>+<P>) you can change bandwidth, double-interleave, integration. "), NULL);
/*  
   took it out because of symmetry and so that F1 - F12 is consistent
   parabutton = gtk_button_new_with_label (_("Parameters"));
  gtk_widget_set_name (parabutton, "parabutton");
  gtk_widget_ref (parabutton);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "parabutton", parabutton,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (parabutton);
  gtk_box_pack_start (GTK_BOX (vboxmode), parabutton, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (parabutton), 3);

*/
  vbox_textwindows = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (vbox_textwindows, "vbox_textwindows");
  gtk_widget_ref (vbox_textwindows);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "vbox_textwindows", vbox_textwindows,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox_textwindows);
  gtk_box_pack_start (GTK_BOX (hbox_big_downside), vbox_textwindows, TRUE, TRUE, 0);

  frame8 = gtk_frame_new (_("RECEIVED TEXT"));
  gtk_widget_set_name (frame8, "frame8");
  gtk_widget_ref (frame8);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "frame8", frame8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame8);
  gtk_box_pack_start (GTK_BOX (vbox_textwindows), frame8, TRUE, TRUE, 0);

  scrolledwindow40 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow40, "scrolledwindow40");
  gtk_widget_ref (scrolledwindow40);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "scrolledwindow40", scrolledwindow40,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow40);
  gtk_container_add (GTK_CONTAINER (frame8), scrolledwindow40);
//  gtk_box_pack_start (GTK_BOX (vbox_textwindows), scrolledwindow40, TRUE, TRUE , 0);
  gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow40), 3);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow40), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  textmain = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (textmain, "textmain");
  gtk_widget_ref (textmain);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "textmain", textmain,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (textmain);
  gtk_container_add (GTK_CONTAINER (scrolledwindow40), textmain);
  gtk_widget_set_usize (textmain, 350, 120);
  gtk_text_set_editable (GTK_TEXT (textmain), TRUE);
  //GTK_WIDGET_UNSET_FLAGS (textmain, GTK_CAN_FOCUS);
  //gtk_widget_set_events (textmain, GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);

  rightbuttonframe = gtk_frame_new (_("Texts: (Shift-F...)"));
  gtk_widget_set_name (rightbuttonframe, "rightbuttonframe");
  gtk_widget_ref (rightbuttonframe);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "rightbuttonframe", rightbuttonframe,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (rightbuttonframe);
  gtk_box_pack_start (GTK_BOX (hbox_big_downside), rightbuttonframe, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (rightbuttonframe), 3);

  vboxfix = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (vboxfix, "vboxfix");
  gtk_widget_ref (vboxfix);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "vboxfix", vboxfix,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vboxfix);
  gtk_container_add (GTK_CONTAINER (rightbuttonframe), vboxfix);
  gtk_container_set_border_width (GTK_CONTAINER (vboxfix), 3);

  Fix_button1 = gtk_button_new_with_label (_(" 1: CQ de..."));
  gtk_widget_set_name (Fix_button1, "Fix_button1");
  gtk_widget_ref (Fix_button1);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "Fix_button1", Fix_button1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (Fix_button1);
  gtk_box_pack_start (GTK_BOX (vboxfix), Fix_button1, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (Fix_button1), 3);
  gtk_tooltips_set_tip (tooltips, Fix_button1, _("To edit, press \"Shift T\". \nTo transmit, press \"Shift F1\"."), NULL);
  gtk_widget_add_accelerator (Fix_button1, "clicked", accel_group,
                              GDK_F1, GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);

 
  Fix_button2 = gtk_button_new_with_label (_(" 2: QRZ? de..."));
  gtk_widget_set_name (Fix_button2, "Fix_button2");
  gtk_widget_ref (Fix_button2);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "Fix_button2", Fix_button2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (Fix_button2);
  gtk_box_pack_start (GTK_BOX (vboxfix), Fix_button2, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (Fix_button2), 3);
  gtk_tooltips_set_tip (tooltips, Fix_button2, _("To edit the fixtexts, press \"Shift T\". \nTo transmit, press \"Shift F2\""), NULL);
  gtk_widget_add_accelerator (Fix_button2, "clicked", accel_group,
                              GDK_F2, GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);

  Fix_button3 = gtk_button_new_with_label (_(" 3: ... de..."));
  gtk_widget_set_name (Fix_button3, "Fix_button3");
  gtk_widget_ref (Fix_button3);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "Fix_button3", Fix_button3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (Fix_button3);
  gtk_box_pack_start (GTK_BOX (vboxfix), Fix_button3, FALSE, FALSE, 0);
  
  gtk_container_set_border_width (GTK_CONTAINER (Fix_button3), 3);
  gtk_tooltips_set_tip (tooltips, Fix_button3, _("To edit the fixtexts, press \"Shift T\". \nTo transmit, press \"Shift F3\""), NULL);
  gtk_widget_add_accelerator (Fix_button3, "clicked", accel_group,
                              GDK_F3, GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);

  Fix_button4 = gtk_button_new_with_label (_(" 4: station brag"));
  gtk_widget_set_name (Fix_button4, "Fix_button4");
  gtk_widget_ref (Fix_button4);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "Fix_button4", Fix_button4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (Fix_button4);
  gtk_box_pack_start (GTK_BOX (vboxfix), Fix_button4, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (Fix_button4), 3);
  gtk_tooltips_set_tip (tooltips, Fix_button4, _("To edit the fixtexts, press \"Shift T\". \nTo transmit, press \"Shift F4\""), NULL);
  gtk_widget_add_accelerator (Fix_button4, "clicked", accel_group,
                              GDK_F4, GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);

  Fix_button5 = gtk_button_new_with_label (_(" 5: prog brag"));
  gtk_widget_set_name (Fix_button5, "Fix_button5");
  gtk_widget_ref (Fix_button5);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "Fix_button5", Fix_button5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (Fix_button5);
  gtk_box_pack_start (GTK_BOX (vboxfix), Fix_button5, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (Fix_button5), 3);
  gtk_tooltips_set_tip (tooltips, Fix_button5, _("To edit the fixtexts, press \"Shift T\". \nTo transmit, press \"Shift F5\""), NULL);
  gtk_widget_add_accelerator (Fix_button5, "clicked", accel_group,
                              GDK_F5, GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);

  Fix_button6 = gtk_button_new_with_label (_(" 6: qsl via..."));
  gtk_widget_set_name (Fix_button6, "Fix_button6");
  gtk_widget_ref (Fix_button6);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "Fix_button6", Fix_button6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (Fix_button6);
  gtk_box_pack_start (GTK_BOX (vboxfix), Fix_button6, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (Fix_button6), 3);
  gtk_tooltips_set_tip (tooltips, Fix_button6, _("To edit the fixtexts, press \"Shift T\". \nTo transmit, press \"Shift F6\""), NULL);
  gtk_widget_add_accelerator (Fix_button6, "clicked", accel_group,
                              GDK_F6, GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);

  Fix_button7 = gtk_button_new_with_label (_(" 7: test de.."));
  gtk_widget_set_name (Fix_button7, "Fix_button7");
  gtk_widget_ref (Fix_button7);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "Fix_button7", Fix_button7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (Fix_button7);
  gtk_box_pack_start (GTK_BOX (vboxfix), Fix_button7, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (Fix_button7), 3);
  gtk_tooltips_set_tip (tooltips, Fix_button7, _("To edit the fixtexts, press \"Shift T\". \nTo transmit, press \"Shift F7\""), NULL);
  gtk_widget_add_accelerator (Fix_button7, "clicked", accel_group,
                              GDK_F7, GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);

  Fix_button8 = gtk_button_new_with_label (_(" 8: time"));
  gtk_widget_set_name (Fix_button8, "Fix_button8");
  gtk_widget_ref (Fix_button8);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "Fix_button8", Fix_button8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (Fix_button8);
  gtk_box_pack_start (GTK_BOX (vboxfix), Fix_button8, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (Fix_button8), 3);
  gtk_tooltips_set_tip (tooltips, Fix_button8, _("To edit the fixtexts, press \"Shift T\". \nTo transmit, press \"Shift F8\""), NULL);
  gtk_widget_add_accelerator (Fix_button8, "clicked", accel_group,
                              GDK_F8, GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);

  Fix_button9 = gtk_button_new_with_label (_(" 9: contest"));
  gtk_widget_set_name (Fix_button9, "Fix_button9");
  gtk_widget_ref (Fix_button9);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "Fix_button9", Fix_button9,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (Fix_button9);
  gtk_box_pack_start (GTK_BOX (vboxfix), Fix_button9, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (Fix_button9), 3);
  gtk_tooltips_set_tip (tooltips, Fix_button9, _("To edit the fixtexts, press \"Shift T\". \nTo transmit, press \"Shift F9\""), NULL);
  gtk_widget_add_accelerator (Fix_button9, "clicked", accel_group,
                              GDK_F9, GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);

  Fix_button10 = gtk_button_new_with_label (_("10: mailbox"));
  gtk_widget_set_name (Fix_button10, "Fix_button10");
  gtk_widget_ref (Fix_button10);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "Fix_button10", Fix_button10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (Fix_button10);
  gtk_box_pack_start (GTK_BOX (vboxfix), Fix_button10, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (Fix_button10), 3);
  gtk_tooltips_set_tip (tooltips, Fix_button10, _("Just a beacon announcing the mailbox. "
    "To start the mailbox, press <alt>+M, if a program like F6FBB is prepared. "
    "See HOWTO!"), NULL);
  gtk_widget_add_accelerator (Fix_button10, "clicked", accel_group,
                              GDK_F10, GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);

  Fix_button11 = gtk_button_new_with_label (_("11: bye"));
  gtk_widget_set_name (Fix_button11, "Fix_button11");
  gtk_widget_ref (Fix_button11);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "Fix_button11", Fix_button11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (Fix_button11);
  gtk_box_pack_start (GTK_BOX (vboxfix), Fix_button11, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (Fix_button11), 3);
  gtk_tooltips_set_tip (tooltips, Fix_button11, _("To edit the fixtexts, press \"Shift T\". \nTo transmit, press \"Shift F11\""), NULL);
  gtk_widget_add_accelerator (Fix_button11, "clicked", accel_group,
                              GDK_F11, GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);

  Fix_button12 = gtk_button_new_with_label (_("12: littleman"));
  gtk_widget_set_name (Fix_button12, "Fix_button12");
  gtk_widget_ref (Fix_button12);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "Fix_button12", Fix_button12,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (Fix_button12);
  gtk_box_pack_start (GTK_BOX (vboxfix), Fix_button12, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (Fix_button12), 3);
  gtk_tooltips_set_tip (tooltips, Fix_button12, _("To edit the fixtexts, press \"Shift T\". \nTo transmit, press \"Shift F12\""), NULL);
  gtk_widget_add_accelerator (Fix_button12, "clicked", accel_group,
                              GDK_F12, GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);

  frame4 = gtk_frame_new (_("TRANSMIT TEXT"));
  gtk_widget_set_name (frame4, "frame4");
  gtk_widget_ref (frame4);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "frame4", frame4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame4);
  gtk_box_pack_start (GTK_BOX (vbox_textwindows), frame4, TRUE, TRUE, 0);

  scrolledwindow38 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow38, "scrolledwindow38");
  gtk_widget_ref (scrolledwindow38);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "scrolledwindow38", scrolledwindow38,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow38);
  gtk_container_add (GTK_CONTAINER (frame4), scrolledwindow38);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow38), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  textedit = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (textedit, "textedit");
  gtk_widget_ref (textedit);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "textedit", textedit,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (textedit);
  gtk_container_add (GTK_CONTAINER (scrolledwindow38), textedit);
  gtk_widget_set_usize (textedit, 350, 60);
  gtk_text_set_editable (GTK_TEXT (textedit), TRUE);

  frame11 = gtk_frame_new (_("STATUS DISPLAY"));
  gtk_widget_set_name (frame11, "frame11");
  gtk_widget_ref (frame11);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "frame11", frame11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame11);
  gtk_box_pack_start (GTK_BOX (vbox_textwindows), frame11, TRUE, TRUE, 0);

  scrolledwindow41 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow41, "scrolledwindow41");
  gtk_widget_ref (scrolledwindow41);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "scrolledwindow41", scrolledwindow41,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow41);
  gtk_container_add (GTK_CONTAINER (frame11), scrolledwindow41);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow41), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  textstatus = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (textstatus, "textstatus");
  gtk_widget_ref (textstatus);
  gtk_object_set_data_full (GTK_OBJECT (wmain), "textstatus", textstatus,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (textstatus);
  gtk_container_add (GTK_CONTAINER (scrolledwindow41), textstatus);
  gtk_widget_set_usize (textstatus, 350, 30);
  GTK_WIDGET_UNSET_FLAGS (textstatus, GTK_CAN_FOCUS);
  gtk_tooltips_set_tip (tooltips, textstatus, _("informs you what hfterm is doing, e.g. switching modes..."), NULL);

  gtk_signal_connect (GTK_OBJECT (wmain), "delete_event",
                      GTK_SIGNAL_FUNC (on_wmain_delete_event),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (wmain), "destroy_event",
                      GTK_SIGNAL_FUNC (on_wmain_destroy_event),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (quit), "activate",
                      GTK_SIGNAL_FUNC (on_quit_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (rx_file), "activate",
                      GTK_SIGNAL_FUNC (rx_demand_store),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (tx_file), "activate",
                      GTK_SIGNAL_FUNC (on_tx_file_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (beacon_file), "activate",
                      GTK_SIGNAL_FUNC (on_beacon_file_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (become_irs), "activate",
                     GTK_SIGNAL_FUNC (on_become_irs_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (become_iss), "activate",
                      GTK_SIGNAL_FUNC (on_become_iss_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (qrt), "activate",
                      GTK_SIGNAL_FUNC (on_qrt_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (speedup), "activate",
                      GTK_SIGNAL_FUNC (on_speedup_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (uppercase), "activate",
                      GTK_SIGNAL_FUNC (on_uppercase_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (lowercase), "activate",
                      GTK_SIGNAL_FUNC (on_lowercase_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (figurecase), "activate",
                      GTK_SIGNAL_FUNC (on_figurecase_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (mailbox), "activate",
                      GTK_SIGNAL_FUNC (on_mailbox_activate),
                      NULL);
		      
  gtk_signal_connect (GTK_OBJECT (cw), "activate",
                      GTK_SIGNAL_FUNC (on_cw_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (standby), "activate",
                      GTK_SIGNAL_FUNC (on_standby_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (standby_pactor), "activate",
                      GTK_SIGNAL_FUNC (on_standby_pactor_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (pactor_arq), "activate",
                      GTK_SIGNAL_FUNC (on_pactor_arq_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (pactor_fec), "activate",
                      GTK_SIGNAL_FUNC (on_pactor_fec_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (gtor_arq1), "activate",
                      GTK_SIGNAL_FUNC (on_gtor_arq1_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (amtor_arq), "activate",
                      GTK_SIGNAL_FUNC (on_amtor_arq_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (amtor_collective_fec), "activate",
                      GTK_SIGNAL_FUNC (on_amtor_collective_fec_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (amtor_selective_fec), "activate",
                      GTK_SIGNAL_FUNC (on_amtor_selective_fec_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (rtty_receive), "activate",
                      GTK_SIGNAL_FUNC (on_rtty_receive_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (rtty_transmit), "activate",
                      GTK_SIGNAL_FUNC (on_rtty_transmit_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (mt63_rx), "activate",
                      GTK_SIGNAL_FUNC (on_mt63_rx_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (mt63_tx), "activate",
                      GTK_SIGNAL_FUNC (on_mt63_tx_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (spectrum), "activate",
                      GTK_SIGNAL_FUNC (on_frequency_spectrum_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (parameters), "activate",
                      GTK_SIGNAL_FUNC (on_parameters_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (monitor), "activate",
                      GTK_SIGNAL_FUNC (on_monitor_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (Wfixtext), "activate",
                      GTK_SIGNAL_FUNC (on_Wfixtext_activate),
                      NULL);
/*
  gtk_signal_connect (GTK_OBJECT (personal_edit1), "activate",
                      GTK_SIGNAL_FUNC (on_brag_activate),
                      NULL);
  */
  gtk_signal_connect (GTK_OBJECT (log_window_show), "activate",
                      GTK_SIGNAL_FUNC (logbook_window_show),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (new_logentry), "activate",
                      GTK_SIGNAL_FUNC (on_new_logentry_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (save_logentry), "activate",
                      GTK_SIGNAL_FUNC (on_save_logentry_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (log_win_ok_button), "clicked",
                      GTK_SIGNAL_FUNC (on_save_logentry_activate),
                      NULL);
  gtk_widget_add_accelerator (log_win_ok_button, "clicked",
  			accel_group,
			GDK_Return, 0, 
      	                GTK_ACCEL_VISIBLE);
  gtk_signal_connect (GTK_OBJECT (clear_logentry), "activate",
                      GTK_SIGNAL_FUNC (on_clear_logentry_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (log_win_cancel_button), "clicked",
                      GTK_SIGNAL_FUNC (on_clear_logentry_activate),
                      NULL);

  gtk_signal_connect (GTK_OBJECT (archivate), "activate",
                      GTK_SIGNAL_FUNC (on_archivate_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (search_entry1), "activate",
                      GTK_SIGNAL_FUNC (on_search_entry1_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (list_all_entrys1), "activate",
                      GTK_SIGNAL_FUNC (on_list_all_entrys1_activate),
                      NULL);
/*  gtk_signal_connect (GTK_OBJECT (map1), "activate",
                      GTK_SIGNAL_FUNC (on_map1_activate),
                      NULL);
 */
  gtk_signal_connect (GTK_OBJECT (about), "activate",
                      GTK_SIGNAL_FUNC (on_about_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (index1), "activate",
                      GTK_SIGNAL_FUNC (on_index1_activate),
                      NULL);
/*
  gtk_signal_connect (GTK_OBJECT (helpoption), "activate",
                      GTK_SIGNAL_FUNC (on_index1_activate),
                      NULL);
 */
  gtk_signal_connect (GTK_OBJECT (helpbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_index1_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (spectrumbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_frequency_spectrum_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (CW_button), "clicked",
                      GTK_SIGNAL_FUNC (on_cw_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (standbybutton), "clicked",
                      GTK_SIGNAL_FUNC (on_standby_activate),
                      NULL);
/*
  gtk_signal_connect (GTK_OBJECT (standby_pactorbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_standby_pactor_activate),
                      NULL);
 */
  gtk_signal_connect (GTK_OBJECT (P_ARQ_button), "clicked",
                      GTK_SIGNAL_FUNC (on_pactor_arq_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (P_FEC_button), "clicked",
                      GTK_SIGNAL_FUNC (on_pactor_fec_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (GTOR_button), "clicked",
                      GTK_SIGNAL_FUNC (on_gtor_arq1_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (A_ARQ_button), "clicked",
                      GTK_SIGNAL_FUNC (on_amtor_arq_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (A_CFEC_button), "clicked",
                      GTK_SIGNAL_FUNC (on_amtor_collective_fec_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (A_SFEC_button), "clicked",
                      GTK_SIGNAL_FUNC (on_amtor_selective_fec_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (RTTY_RX_button), "clicked",
                      GTK_SIGNAL_FUNC (on_rtty_receive_activate),
                      NULL);
/*		      
  gtk_signal_connect (GTK_OBJECT (RTTY_TX_button), "clicked",
                      GTK_SIGNAL_FUNC (on_rtty_transmit_activate),
                      NULL);
*/
  gtk_signal_connect (GTK_OBJECT (MT63_button), "clicked",
                      GTK_SIGNAL_FUNC (on_mt63_rx_activate),
                      NULL);
		      
  gtk_signal_connect (GTK_OBJECT (Fix_button1), "clicked",
                      GTK_SIGNAL_FUNC (on_Fix_button1_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (Fix_button2), "clicked",
                      GTK_SIGNAL_FUNC (on_Fix_button2_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (Fix_button3), "clicked",
                      GTK_SIGNAL_FUNC (on_Fix_button3_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (Fix_button4), "clicked",
                      GTK_SIGNAL_FUNC (on_Fix_button4_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (Fix_button5), "clicked",
                      GTK_SIGNAL_FUNC (on_Fix_button5_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (Fix_button6), "clicked",
                      GTK_SIGNAL_FUNC (on_Fix_button6_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (Fix_button7), "clicked",
                      GTK_SIGNAL_FUNC (on_Fix_button7_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (Fix_button8), "clicked",
                      GTK_SIGNAL_FUNC (on_Fix_button8_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (Fix_button9), "clicked",
                      GTK_SIGNAL_FUNC (on_Fix_button9_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (Fix_button10), "clicked",
                      GTK_SIGNAL_FUNC (on_Fix_button10_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (Fix_button11), "clicked",
                      GTK_SIGNAL_FUNC (on_Fix_button11_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (Fix_button12), "clicked",
                      GTK_SIGNAL_FUNC (on_Fix_button12_clicked),
                      NULL);


/*  gtk_signal_connect (GTK_OBJECT (parabutton), "clicked",
                      GTK_SIGNAL_FUNC (on_parabutton_clicked),
                      NULL);
*/
  gtk_signal_connect (GTK_OBJECT (textmain), "key_press_event",
                      GTK_SIGNAL_FUNC (on_rx_keypress_event),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (textmain), "key_release_event",
                      GTK_SIGNAL_FUNC (on_rx_keyrelease_event),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (textedit), "key_press_event",
                      GTK_SIGNAL_FUNC (on_tx_keypress_event),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (textedit), "key_release_event",
                      GTK_SIGNAL_FUNC (on_tx_keyrelease_event),
                      NULL);
    /* for cw elbug: */
  gtk_signal_connect (GTK_OBJECT (textedit), "button_press_event",
                      GTK_SIGNAL_FUNC (on_textedit_button_press_event),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (textedit), "button_release_event",
                      GTK_SIGNAL_FUNC (on_textedit_button_release_event),
                      NULL);

  gtk_object_set_data (GTK_OBJECT (wmain), "tooltips", tooltips);

  gtk_window_add_accel_group (GTK_WINDOW (wmain), accel_group);

  return wmain;
}
