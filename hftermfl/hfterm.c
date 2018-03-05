/* Form definition file generated with fdesign. */

#include "forms.h"
#include <stdlib.h>
#include "hfterm.h"

FD_menubar *create_form_menubar(void)
{
  FL_OBJECT *obj;
  FD_menubar *fdui = (FD_menubar *) fl_calloc(1, sizeof(*fdui));

  fdui->menubar = fl_bgn_form(FL_NO_BOX, 310, 200);
  obj = fl_add_box(FL_FLAT_BOX,0,0,310,200,"");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_frame(FL_UP_FRAME,0,0,310,30,"");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->file = obj = fl_add_menu(FL_PULLDOWN_MENU,0,0,70,30,"File");
    fl_set_object_boxtype(obj,FL_FLAT_BOX);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,cb_filemenu,0);
  fdui->status = obj = fl_add_browser(FL_NORMAL_BROWSER,10,40,290,140,"");
    fl_set_object_lstyle(obj,FL_FIXED_STYLE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->mode = obj = fl_add_menu(FL_PULLDOWN_MENU,140,0,70,30,"Mode");
    fl_set_object_boxtype(obj,FL_FLAT_BOX);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,cb_modemenu,0);
  obj = fl_add_text(FL_NORMAL_TEXT,10,180,290,20,"(C) 1997 by Thomas Sailer, HB9JNX/AE4WA");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->util = obj = fl_add_menu(FL_PULLDOWN_MENU,210,0,70,30,"Utils");
    fl_set_object_boxtype(obj,FL_FLAT_BOX);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,cb_utilmenu,0);
  fdui->state = obj = fl_add_menu(FL_PULLDOWN_MENU,70,0,70,30,"State");
    fl_set_object_boxtype(obj,FL_FLAT_BOX);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,cb_statemenu,0);
  fl_end_form();

  fdui->menubar->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_spec *create_form_spec(void)
{
  FL_OBJECT *obj;
  FD_spec *fdui = (FD_spec *) fl_calloc(1, sizeof(*fdui));

  fdui->spec = fl_bgn_form(FL_NO_BOX, 530, 430);
  obj = fl_add_box(FL_UP_BOX,0,0,530,430,"");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,280,10,240,20,"");
  fdui->shift170 = obj = fl_add_checkbutton(FL_RADIO_BUTTON,280,10,60,20,"170Hz");
    fl_set_object_callback(obj,cb_shift,170);
  fdui->scdisp = obj = fl_add_canvas(FL_NORMAL_CANVAS,10,40,510,380,"");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->shift200 = obj = fl_add_checkbutton(FL_RADIO_BUTTON,340,10,60,20,"200Hz");
    fl_set_object_callback(obj,cb_shift,200);
    fl_set_button(obj, 1);
  fdui->shift425 = obj = fl_add_checkbutton(FL_RADIO_BUTTON,400,10,60,20,"425Hz");
    fl_set_object_callback(obj,cb_shift,425);
  fdui->shift850 = obj = fl_add_checkbutton(FL_RADIO_BUTTON,460,10,60,20,"850Hz");
    fl_set_object_callback(obj,cb_shift,850);
  obj = fl_add_text(FL_NORMAL_TEXT,180,10,40,20,"Space");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,10,10,50,20,"Pointer");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,100,10,40,20,"Mark");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->freq_mark = obj = fl_add_text(FL_NORMAL_TEXT,130,10,50,20,"Hz");
    fl_set_object_boxtype(obj,FL_EMBOSSED_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->freq_space = obj = fl_add_text(FL_NORMAL_TEXT,220,10,50,20,"Hz");
    fl_set_object_boxtype(obj,FL_EMBOSSED_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->freq_pointer = obj = fl_add_text(FL_NORMAL_TEXT,50,10,50,20,"Hz");
    fl_set_object_boxtype(obj,FL_EMBOSSED_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fl_end_form();

  fdui->spec->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_pactor_params *create_form_pactor_params(void)
{
  FL_OBJECT *obj;
  FD_pactor_params *fdui = (FD_pactor_params *) fl_calloc(1, sizeof(*fdui));

  fdui->pactor_params = fl_bgn_form(FL_NO_BOX, 210, 470);
  obj = fl_add_box(FL_UP_BOX,0,0,210,470,"");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_frame(FL_DOWN_FRAME,10,10,190,450,"");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,20,180,30,"Pactor Parameters");
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE+FL_SHADOW_STYLE);
  fdui->destcall = obj = fl_add_input(FL_NORMAL_INPUT,120,60,70,30,"Destination Callsign");
  fdui->mycall = obj = fl_add_input(FL_NORMAL_INPUT,120,100,70,30,"Mycall");
  fdui->txdelay = obj = fl_add_input(FL_INT_INPUT,120,140,70,30,"TX Delay (ms)");
  fdui->retry = obj = fl_add_input(FL_INT_INPUT,120,180,70,30,"Retries");
  fdui->longpath = obj = fl_add_checkbutton(FL_PUSH_BUTTON,140,220,30,30,"Longpath");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
  obj = fl_add_button(FL_RETURN_BUTTON,20,420,80,30,"OK");
    fl_set_object_callback(obj,cb_pactorparams,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,110,420,80,30,"Cancel");
    fl_set_object_callback(obj,cb_pactorparams,1);
  fdui->crc100chg = obj = fl_add_input(FL_NORMAL_INPUT,120,260,70,30,"CRC 100 Baud Chg");
  fdui->crc200chg = obj = fl_add_input(FL_NORMAL_INPUT,120,340,70,30,"CRC 200 Baud Chg");
  fdui->crc100 = obj = fl_add_input(FL_NORMAL_INPUT,120,300,70,30,"CRC 100 Baud");
  fdui->crc200 = obj = fl_add_input(FL_NORMAL_INPUT,120,380,70,30,"CRC 200 Baud");
  fl_end_form();

  fdui->pactor_params->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_amtor_params *create_form_amtor_params(void)
{
  FL_OBJECT *obj;
  FD_amtor_params *fdui = (FD_amtor_params *) fl_calloc(1, sizeof(*fdui));

  fdui->amtor_params = fl_bgn_form(FL_NO_BOX, 210, 350);
  obj = fl_add_box(FL_UP_BOX,0,0,210,350,"");
  obj = fl_add_frame(FL_DOWN_FRAME,10,10,190,330,"");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,20,180,30,"Amtor Parameters");
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE+FL_SHADOW_STYLE);
  fdui->destcall = obj = fl_add_input(FL_NORMAL_INPUT,120,60,70,30,"Destination Callsign");
  fdui->mycall = obj = fl_add_input(FL_NORMAL_INPUT,120,140,70,30,"Mycall");
  fdui->txdelay = obj = fl_add_input(FL_INT_INPUT,120,180,70,30,"TX Delay (ms)");
  fdui->retry = obj = fl_add_input(FL_INT_INPUT,120,220,70,30,"Retries");
  fdui->inv = obj = fl_add_checkbutton(FL_PUSH_BUTTON,60,260,30,30,"Invert");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
  obj = fl_add_button(FL_RETURN_BUTTON,20,300,80,30,"OK");
    fl_set_object_callback(obj,cb_amtorparams,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,110,300,80,30,"Cancel");
    fl_set_object_callback(obj,cb_amtorparams,1);
  fdui->selfeccall = obj = fl_add_input(FL_NORMAL_INPUT,120,100,70,30,"Selective FEC call");
  fdui->rxtxinv = obj = fl_add_checkbutton(FL_PUSH_BUTTON,160,260,30,30,"RX/TX invert");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
  fl_end_form();

  fdui->amtor_params->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_rtty_params *create_form_rtty_params(void)
{
  FL_OBJECT *obj;
  FD_rtty_params *fdui = (FD_rtty_params *) fl_calloc(1, sizeof(*fdui));

  fdui->rtty_params = fl_bgn_form(FL_NO_BOX, 210, 190);
  obj = fl_add_box(FL_UP_BOX,0,0,210,190,"");
  obj = fl_add_frame(FL_DOWN_FRAME,10,10,190,170,"");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,20,180,30,"RTTY Parameters");
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE+FL_SHADOW_STYLE);
  fdui->baud = obj = fl_add_input(FL_INT_INPUT,120,60,70,30,"Baud Rate");
  obj = fl_add_button(FL_RETURN_BUTTON,20,140,80,30,"OK");
    fl_set_object_callback(obj,cb_rttyparams,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,110,140,80,30,"Cancel");
    fl_set_object_callback(obj,cb_rttyparams,1);
  fdui->inv = obj = fl_add_checkbutton(FL_PUSH_BUTTON,60,100,30,30,"Invert");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
  fdui->rxtxinv = obj = fl_add_checkbutton(FL_PUSH_BUTTON,160,100,30,30,"RX/TX invert");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
  fl_end_form();

  fdui->rtty_params->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_fsk_params *create_form_fsk_params(void)
{
  FL_OBJECT *obj;
  FD_fsk_params *fdui = (FD_fsk_params *) fl_calloc(1, sizeof(*fdui));

  fdui->fsk_params = fl_bgn_form(FL_NO_BOX, 210, 190);
  obj = fl_add_box(FL_UP_BOX,0,0,210,190,"");
  obj = fl_add_frame(FL_DOWN_FRAME,10,10,190,170,"");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,20,160,30,"FSK Parameters");
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE+FL_SHADOW_STYLE);
  fdui->freqspace = obj = fl_add_input(FL_INT_INPUT,120,60,70,30,"Space Frequency");
  obj = fl_add_button(FL_RETURN_BUTTON,20,140,80,30,"OK");
    fl_set_object_callback(obj,cb_fskparams,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,110,140,80,30,"Cancel");
    fl_set_object_callback(obj,cb_fskparams,1);
  fdui->freqmark = obj = fl_add_input(FL_INT_INPUT,120,100,70,30,"Mark Frequency");
  fl_end_form();

  fdui->fsk_params->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_mixer_params *create_form_mixer_params(void)
{
  FL_OBJECT *obj;
  FD_mixer_params *fdui = (FD_mixer_params *) fl_calloc(1, sizeof(*fdui));

  fdui->mixer_params = fl_bgn_form(FL_NO_BOX, 240, 250);
  obj = fl_add_box(FL_UP_BOX,0,0,240,250,"");
  obj = fl_add_frame(FL_DOWN_FRAME,10,10,220,230,"");

  fdui->insrc = fl_bgn_group();
  fdui->insrc_aux = obj = fl_add_checkbutton(FL_RADIO_BUTTON,20,130,20,20,"CD/Aux");
  fdui->insrc_mic = obj = fl_add_checkbutton(FL_RADIO_BUTTON,20,110,20,20,"Mic");
  fdui->insrc_line = obj = fl_add_checkbutton(FL_RADIO_BUTTON,20,90,20,20,"Line");
    fl_set_button(obj, 1);
  fl_end_group();

  fdui->igain = obj = fl_add_valslider(FL_VERT_SLIDER,150,20,30,200,"input gain");
    fl_set_slider_bounds(obj, 1.00, 0.00);
     fl_set_slider_return(obj, FL_RETURN_END);
  fdui->ogain = obj = fl_add_valslider(FL_VERT_SLIDER,190,20,30,200,"onput gain");
    fl_set_slider_bounds(obj, 1.00, 0.00);
     fl_set_slider_return(obj, FL_RETURN_END);
  obj = fl_add_button(FL_RETURN_BUTTON,20,160,80,30,"OK");
    fl_set_object_callback(obj,cb_mixerparams,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,20,200,80,30,"Cancel");
    fl_set_object_callback(obj,cb_mixerparams,1);
  obj = fl_add_text(FL_NORMAL_TEXT,20,20,60,30,"Mixer");
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE+FL_SHADOW_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,50,110,30,"Parameters");
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE+FL_SHADOW_STYLE);
  fl_end_form();

  fdui->mixer_params->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_gtor_params *create_form_gtor_params(void)
{
  FL_OBJECT *obj;
  FD_gtor_params *fdui = (FD_gtor_params *) fl_calloc(1, sizeof(*fdui));

  fdui->gtor_params = fl_bgn_form(FL_NO_BOX, 210, 270);
  obj = fl_add_box(FL_UP_BOX,0,0,210,270,"");
  obj = fl_add_frame(FL_DOWN_FRAME,10,10,190,250,"");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,20,170,30,"GTOR Parameters");
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE+FL_SHADOW_STYLE);
  fdui->destcall = obj = fl_add_input(FL_NORMAL_INPUT,120,60,70,30,"Destination Callsign");
  fdui->mycall = obj = fl_add_input(FL_NORMAL_INPUT,120,100,70,30,"Mycall");
  fdui->txdelay = obj = fl_add_input(FL_INT_INPUT,120,140,70,30,"TX Delay (ms)");
  fdui->retry = obj = fl_add_input(FL_INT_INPUT,120,180,70,30,"Retries");
  obj = fl_add_button(FL_RETURN_BUTTON,20,220,80,30,"OK");
    fl_set_object_callback(obj,cb_gtorparams,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,110,220,80,30,"Cancel");
    fl_set_object_callback(obj,cb_gtorparams,1);
  fl_end_form();

  fdui->gtor_params->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

