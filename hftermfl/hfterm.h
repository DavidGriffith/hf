#ifndef FD_menubar_h_
#define FD_menubar_h_
/* Header file generated with fdesign. */

/**** Callback routines ****/

extern void cb_filemenu(FL_OBJECT *, long);
extern void cb_modemenu(FL_OBJECT *, long);
extern void cb_utilmenu(FL_OBJECT *, long);
extern void cb_statemenu(FL_OBJECT *, long);

extern void cb_shift(FL_OBJECT *, long);

extern void cb_pactorparams(FL_OBJECT *, long);

extern void cb_amtorparams(FL_OBJECT *, long);

extern void cb_rttyparams(FL_OBJECT *, long);

extern void cb_fskparams(FL_OBJECT *, long);

extern void cb_mixerparams(FL_OBJECT *, long);

extern void cb_gtorparams(FL_OBJECT *, long);


/**** Forms and Objects ****/

typedef struct {
	FL_FORM *menubar;
	void *vdata;
	long ldata;
	FL_OBJECT *file;
	FL_OBJECT *status;
	FL_OBJECT *mode;
	FL_OBJECT *util;
	FL_OBJECT *state;
} FD_menubar;

extern FD_menubar * create_form_menubar(void);
typedef struct {
	FL_FORM *spec;
	void *vdata;
	long ldata;
	FL_OBJECT *shift170;
	FL_OBJECT *scdisp;
	FL_OBJECT *shift200;
	FL_OBJECT *shift425;
	FL_OBJECT *shift850;
	FL_OBJECT *freq_mark;
	FL_OBJECT *freq_space;
	FL_OBJECT *freq_pointer;
} FD_spec;

extern FD_spec * create_form_spec(void);
typedef struct {
	FL_FORM *pactor_params;
	void *vdata;
	long ldata;
	FL_OBJECT *destcall;
	FL_OBJECT *mycall;
	FL_OBJECT *txdelay;
	FL_OBJECT *retry;
	FL_OBJECT *longpath;
	FL_OBJECT *crc100chg;
	FL_OBJECT *crc200chg;
	FL_OBJECT *crc100;
	FL_OBJECT *crc200;
} FD_pactor_params;

extern FD_pactor_params * create_form_pactor_params(void);
typedef struct {
	FL_FORM *amtor_params;
	void *vdata;
	long ldata;
	FL_OBJECT *destcall;
	FL_OBJECT *mycall;
	FL_OBJECT *txdelay;
	FL_OBJECT *retry;
	FL_OBJECT *inv;
	FL_OBJECT *selfeccall;
	FL_OBJECT *rxtxinv;
} FD_amtor_params;

extern FD_amtor_params * create_form_amtor_params(void);
typedef struct {
	FL_FORM *rtty_params;
	void *vdata;
	long ldata;
	FL_OBJECT *baud;
	FL_OBJECT *inv;
	FL_OBJECT *rxtxinv;
} FD_rtty_params;

extern FD_rtty_params * create_form_rtty_params(void);
typedef struct {
	FL_FORM *fsk_params;
	void *vdata;
	long ldata;
	FL_OBJECT *freqspace;
	FL_OBJECT *freqmark;
} FD_fsk_params;

extern FD_fsk_params * create_form_fsk_params(void);
typedef struct {
	FL_FORM *mixer_params;
	void *vdata;
	long ldata;
	FL_OBJECT *insrc;
	FL_OBJECT *insrc_aux;
	FL_OBJECT *insrc_mic;
	FL_OBJECT *insrc_line;
	FL_OBJECT *igain;
	FL_OBJECT *ogain;
} FD_mixer_params;

extern FD_mixer_params * create_form_mixer_params(void);
typedef struct {
	FL_FORM *gtor_params;
	void *vdata;
	long ldata;
	FL_OBJECT *destcall;
	FL_OBJECT *mycall;
	FL_OBJECT *txdelay;
	FL_OBJECT *retry;
} FD_gtor_params;

extern FD_gtor_params * create_form_gtor_params(void);

#endif /* FD_menubar_h_ */
