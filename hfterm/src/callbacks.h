#include <gtk/gtk.h>


gboolean
on_wspec_delete_event                  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_spec_button_press_event             (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_spec_motion_event                   (GtkWidget       *widget,
                                        GdkEventMotion  *event,
                                        gpointer         user_data);

GtkWidget*
spectrum_new (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

void
on_parok_clicked                       (GtkButton       *button,
                                        gpointer         user_data);

void
on_parcancel_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_wmain_delete_event                  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_wmain_destroy_event                 (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_quit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_become_irs_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_become_iss_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_qrt_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_speedup_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_uppercase_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_lowercase_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_figurecase_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_standby_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_pactor_arq_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_pactor_fec_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_gtor_arq1_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_amtor_arq_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_amtor_collective_fec_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_amtor_selective_fec_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rtty_receive_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rtty_transmit_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_frequency_spectrum_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_parameters_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_monitor_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_text_keypress_event                 (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data);

gboolean
on_text_keyrelease_event               (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data);

void
on_aboutok_clicked                     (GtkButton       *button,
                                        gpointer         user_data);
