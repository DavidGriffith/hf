/* 
 * Spectrum Widget
 * Copyright (C) 1999 Thomas Sailer <sailer@ife.ee.ethz.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __SPECTRUM_H__
#define __SPECTRUM_H__

#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>

#define SPEC_WIDTH   460 // 512
#define SPEC_HEIGHT 330 //384

extern void spec_samples(short *data, unsigned int len);
void spec_samples(short *data, unsigned int len);
unsigned int get_freq_shift(void);
void set_freq_shift(int shift);
void set_fsk_freq(unsigned int mark, unsigned int space);
void scope_window(int on) ;
void scope_draw(float *data);

/*
static void spectrum_class_init(SpectrumClass *klass);
static void spectrum_init(Spectrum *spec);
static void spectrum_finalize(GtkObject *object);
static gint spectrum_expose(GtkWidget *widget, GdkEventExpose *event);
static void spectrum_realize(GtkWidget *widget);
static void spectrum_unrealize(GtkWidget *widget);
static void spectrum_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static void spectrum_send_configure (Spectrum *spec);
static gint idle_callback(gpointer data);
*/

extern int scope_on;
extern guint squelch;
extern int squelchpercent;
extern int squelch_passed_fsk;
extern int squelch_passed_cw;
extern int squelch_passed_mt36_500_1000;
extern int squelch_passed_mt36_1000_1500;
extern int squelch_passed_mt36_1500_2500;
/* last 2 prepared for mt63_bandwidth_auto_detect() to come in sha allah ! */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SPECTRUM(obj)          GTK_CHECK_CAST(obj, spectrum_get_type(), Spectrum)
#define SPECTRUM_CLASS(klass)  GTK_CHECK_CLASS_CAST(klass, spectrum_get_type(), SpectrumClass)
#define IS_SPECTRUM(obj)       GTK_CHECK_TYPE(obj, spectrum_get_type())

typedef struct _Spectrum        Spectrum;
typedef struct _SpectrumClass   SpectrumClass;

#define SPECTRUM_NUMSAMPLES  1024

#define SPECTRUM_WIDTH    ((SPECTRUM_NUMSAMPLES) >> 1)
#define SPECTRUM_HEIGHT   384

struct _Spectrum 
{
	GtkWidget widget;

	guint idlefunc;
	GdkGC *trace_gc;
	GdkGC *grid_gc;
	GdkGC *squelch_gc;
	GdkGC *space_gc;
	GdkGC *mark_gc;
	GdkGC *pointer_gc;
	GdkColor tracecol;
        GdkColor gridcol;
	GdkColor squelchcol;
        GdkColor spacecol;
	GdkColor markcol;
	GdkColor pointercol;
	GdkPixmap *pixmap;

	/* marker */
	unsigned int space;
	unsigned int mark;
	unsigned int pointer;

	unsigned short y[SPECTRUM_WIDTH];
	float window[SPECTRUM_NUMSAMPLES];
};

struct _SpectrumClass
{
	GtkWidgetClass parent_class;
};


guint spectrum_get_type(void);

GtkWidget* spectrum_new(const char *name, void *dummy0, void *dummy1, unsigned int dummy2, unsigned int dummy3);
void spectrum_setdata(Spectrum *spec, short *samples);
void spectrum_setmarker(Spectrum *spec, int space, int mark, int pointer);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __SPECTRUM_H__ */
