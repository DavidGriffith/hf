/* 
 * Soundmodem Spectrum Widget
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "hft.h"
#include "spectrum.h"
#include <gtk/gtkgc.h>
#include <gtk/gtkmain.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

/* --------------------------------------------------------------------- */

#define SQUELCHPOINTS 9
guint squelch = 0;
int squelch_passed_fsk;
int squelch_passed_cw;
int squelch_passed_mt36_500_1000;
int squelch_passed_mt36_1000_1500;
int squelch_passed_mt36_1500_2500;
/* last 2 prepared for mt63_bandwidth_auto_detect() to come in sha allah ! */

/* --------------------------------------------------------------------- */

static inline float hamming(float x)
{
        return 0.54-0.46*cos(2*M_PI*x);
}

/* ---------------------------------------------------------------------- */

/*
 * This fft routine is from ~gabriel/src/filters/fft/fft.c;
 * I am unsure of the original source.  The file contains no
 * copyright notice or description.
 * The declaration is changed to the prototype form but the
 * function body is unchanged.  (J. T. Buck)
 */

#define SWAP(a, b) tempr=(a); (a)=(b); (b)=tempr

/*
 * Replace data by its discrete Fourier transform, if isign is
 * input as 1, or by its inverse discrete Fourier transform, if 
 * "isign" is input as -1.  "data'"is a complex array of length "nn",
 * input as a real array data[0..2*nn-1]. "nn" MUST be an integer
 * power of 2 (this is not checked for!?)
 */

static void fft_rif(float *data, int nn, int isign) 
{
        int n;
        int mmax;
        int m, j, istep, i;
        float wtemp, wr, wpr, wpi, wi, theta;
        float tempr, tempi;

        data--;
        n = nn << 1;
        j = 1;

        for (i = 1; i < n; i += 2) {
                if(j > i) {
                        SWAP(data[j], data[i]);
                        SWAP(data[j+1], data[i+1]);
                }
                m= n >> 1;
                while (m >= 2 && j >m) {
                        j -= m;
                        m >>= 1;
                }
                j += m;
        }
        mmax = 2;
        while (n > mmax) {
                istep = 2*mmax;
                theta = -6.28318530717959/(isign*mmax);
                wtemp = sin(0.5*theta);
                wpr = -2.0*wtemp*wtemp;
                wpi = sin(theta);
                wr = 1.0;
                wi = 0.0;
                for (m = 1; m < mmax; m += 2) {
                        for (i = m; i < n; i += istep) {
                                j = i + mmax;
                                tempr = wr*data[j] - wi*data[j+1];
                                tempi = wr*data[j+1] + wi*data[j];
                                data[j] = data[i] - tempr;
                                data[j+1] = data[i+1] - tempi;
                                data[i] += tempr;
                                data[i+1] += tempi;
                        }
                        wr = (wtemp=wr)*wpr - wi*wpi+wr;
                        wi = wi*wpr + wtemp*wpi + wi;
                }
                mmax = istep;
        }
}
        
#undef SWAP

/* --------------------------------------------------------------------- */

static inline float fsqr(float x) 
{ 
        return x*x;
}

/* --------------------------------------------------------------------- */

#define PRIO G_PRIORITY_LOW
#define DBSPAN 80
#define SRATE 8000

static void spectrum_class_init(SpectrumClass *klass);
static void spectrum_init(Spectrum *spec);
static void spectrum_finalize(GtkObject *object);
static gint spectrum_expose(GtkWidget *widget, GdkEventExpose *event);
static void spectrum_realize(GtkWidget *widget);
static void spectrum_unrealize(GtkWidget *widget);
static void spectrum_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static void spectrum_send_configure (Spectrum *spec);
static gint idle_callback(gpointer data);

static GtkWidgetClass *parent_class = NULL;
static SpectrumClass *spectrum_class = NULL;


guint spectrum_get_type(void)
{
	static guint spectrum_type = 0;

	if (!spectrum_type)
	{
		static const GtkTypeInfo spectrum_info =
		{
			"Spectrum",
			sizeof(Spectrum),
			sizeof(SpectrumClass),
			(GtkClassInitFunc)spectrum_class_init,
			(GtkObjectInitFunc)spectrum_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc)NULL,
		};
		spectrum_type = gtk_type_unique(gtk_widget_get_type(), &spectrum_info);
	}
	return spectrum_type;
}

static void spectrum_class_init(SpectrumClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass*)klass;
	widget_class = (GtkWidgetClass*)klass;

	parent_class = gtk_type_class(gtk_widget_get_type());
	spectrum_class = klass;

	object_class->finalize = spectrum_finalize;
	widget_class->expose_event = spectrum_expose;
	widget_class->realize = spectrum_realize;
	widget_class->unrealize = spectrum_unrealize;
	widget_class->size_allocate = spectrum_size_allocate;
}

static void spectrum_init(Spectrum *spec)
{
	spec->idlefunc = 0;
	/* initialize the colors */
	spec->tracecol.red = 11796;
	spec->tracecol.green = 53740;
	spec->tracecol.blue = 4588;
	spec->gridcol.red = 52429;
	spec->gridcol.green = 52429;
	spec->gridcol.blue = 52429;
	spec->squelchcol.red = 10000; // 0
	spec->squelchcol.green = 45000; // 52429;
	spec->squelchcol.blue = 0;
	spec->spacecol.red = 65535;
	spec->spacecol.green = 0;
	spec->spacecol.blue = 24903;
	spec->markcol.red = 29491;
	spec->markcol.green = 0;
	spec->markcol.blue = 65535;
	spec->pointercol.red = 65535;
	spec->pointercol.green = 0;
	spec->pointercol.blue = 65535;
	spec->trace_gc = spec->squelch_gc = spec->grid_gc = spec->space_gc = 
            spec->mark_gc = spec->pointer_gc = NULL;
	spec->pixmap = NULL;
	/* initialize the data */
	memset(&spec->y, 0, sizeof(spec->y));
	spec->space = spec->mark = spec->pointer = 0;
}

static void spectrum_realize(GtkWidget *widget)
{
	Spectrum *spec;
	GdkWindowAttr attributes;
	gint attributes_mask;
	GdkGCValues gc_values;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(IS_SPECTRUM(widget));

	spec = SPECTRUM(widget);
	GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.visual = gtk_widget_get_visual(widget);
	attributes.colormap = gtk_widget_get_colormap(widget);
	attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;

	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

	widget->window = gdk_window_new(gtk_widget_get_parent_window(widget), &attributes, attributes_mask);
	gdk_window_set_user_data(widget->window, spec);

	widget->style = gtk_style_attach(widget->style, widget->window);
	gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);

	/* gc's if necessary */
	if (!gdk_color_alloc(widget->style->colormap, &spec->tracecol))
		g_warning("unable to allocate color: ( %d %d %d )",
			  spec->tracecol.red, spec->tracecol.green, spec->tracecol.blue);
	gc_values.foreground = spec->tracecol;
	spec->trace_gc = gtk_gc_get(widget->style->depth, 
				    widget->style->colormap,
				    &gc_values, GDK_GC_FOREGROUND);
	if (!gdk_color_alloc(widget->style->colormap, &spec->gridcol))
		g_warning("unable to allocate color: ( %d %d %d )",
			  spec->gridcol.red, spec->gridcol.green, spec->gridcol.blue);
	gc_values.foreground = spec->gridcol;
	spec->grid_gc = gtk_gc_get(widget->style->depth,
				   widget->style->colormap,
				   &gc_values, GDK_GC_FOREGROUND);
	if (!gdk_color_alloc(widget->style->colormap, &spec->squelchcol))
		g_warning("unable to allocate color: ( %d %d %d )",
			  spec->squelchcol.red, spec->squelchcol.green, spec->squelchcol.blue);
	gc_values.foreground = spec->squelchcol;
	spec->squelch_gc = gtk_gc_get(widget->style->depth,
				    widget->style->colormap,
				    &gc_values, GDK_GC_FOREGROUND);
	if (!gdk_color_alloc(widget->style->colormap, &spec->spacecol))
		g_warning("unable to allocate color: ( %d %d %d )",
			  spec->spacecol.red, spec->spacecol.green, spec->spacecol.blue);
	gc_values.foreground = spec->spacecol;
	spec->space_gc = gtk_gc_get(widget->style->depth,
				    widget->style->colormap,
				    &gc_values, GDK_GC_FOREGROUND);
	if (!gdk_color_alloc(widget->style->colormap, &spec->markcol))
		g_warning("unable to allocate color: ( %d %d %d )",
			  spec->markcol.red, spec->markcol.green, spec->markcol.blue);
	gc_values.foreground = spec->markcol;
	spec->mark_gc = gtk_gc_get(widget->style->depth,
				   widget->style->colormap,
				   &gc_values, GDK_GC_FOREGROUND);
	if (!gdk_color_alloc(widget->style->colormap, &spec->pointercol))
		g_warning("unable to allocate color: ( %d %d %d )",
			  spec->pointercol.red, spec->pointercol.green, spec->pointercol.blue);
	gc_values.foreground = spec->pointercol;
	spec->pointer_gc = gtk_gc_get(widget->style->depth,
				      widget->style->colormap,
				      &gc_values, GDK_GC_FOREGROUND);
	/* create backing store */
	spec->pixmap = gdk_pixmap_new(widget->window, SPECTRUM_WIDTH, SPECTRUM_HEIGHT, -1);

	spectrum_send_configure(SPECTRUM(widget));
}

static void spectrum_unrealize(GtkWidget *widget)
{
	Spectrum *spec;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(IS_SPECTRUM(widget));

	spec = SPECTRUM(widget);
	if (spec->idlefunc)
		gtk_idle_remove(spec->idlefunc);
	if (spec->trace_gc)
		gtk_gc_release(spec->trace_gc);
	if (spec->squelch_gc)
		gtk_gc_release(spec->squelch_gc);
	if (spec->grid_gc)
		gtk_gc_release(spec->grid_gc);
	if (spec->space_gc)
		gtk_gc_release(spec->space_gc);
	if (spec->mark_gc)
		gtk_gc_release(spec->mark_gc);
	if (spec->pointer_gc)
		gtk_gc_release(spec->pointer_gc);
	spec->trace_gc = spec->grid_gc = spec->space_gc = 
            spec->squelch_gc = spec->mark_gc = spec->pointer_gc = NULL;
	if (spec->pixmap)
			gdk_pixmap_unref(spec->pixmap);
	spec->pixmap = NULL;
	if (GTK_WIDGET_CLASS(parent_class)->unrealize)
		(*GTK_WIDGET_CLASS(parent_class)->unrealize)(widget);
}

static void spectrum_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(IS_SPECTRUM(widget));
	g_return_if_fail(allocation != NULL);
	
	widget->allocation = *allocation;
	widget->allocation.width = SPECTRUM_WIDTH;
	widget->allocation.height = SPECTRUM_HEIGHT;

	if (GTK_WIDGET_REALIZED(widget)) {
		gdk_window_move_resize (widget->window,
					allocation->x, allocation->y,
					allocation->width, allocation->height);
		spectrum_send_configure(SPECTRUM(widget));
	}
}

static void spectrum_send_configure(Spectrum *spec)
{
	GtkWidget *widget;
	GdkEventConfigure event;

	widget = GTK_WIDGET(spec);

	event.type = GDK_CONFIGURE;
	event.window = widget->window;
	event.send_event = TRUE;
	event.x = widget->allocation.x;
	event.y = widget->allocation.y;
	event.width = widget->allocation.width;
	event.height = widget->allocation.height;
  
	gtk_widget_event(widget, (GdkEvent*)&event);
}


GtkWidget* spectrum_new(const char *name, void *dummy0, void *dummy1, unsigned int dummy2, unsigned int dummy3)
{
	Spectrum *spec;
	unsigned int i;
	
	spec = gtk_type_new(spectrum_get_type());
	memset(&spec->y, 0, sizeof(spec->y));
	for (i = 0; i < SPECTRUM_NUMSAMPLES; i++)
		spec->window[i] = (1.0 / 32767.0 / SPECTRUM_NUMSAMPLES) * 
                    hamming(i * (1.0 / (SPECTRUM_NUMSAMPLES-1)));
	return GTK_WIDGET(spec);
}

static void spectrum_finalize(GtkObject *object)
{
	g_return_if_fail(object != NULL);
	g_return_if_fail(IS_SPECTRUM(object));
	(*GTK_OBJECT_CLASS(parent_class)->finalize)(object);
}

static gint spectrum_expose(GtkWidget *widget, GdkEventExpose *event)
{
	Spectrum *spec;

	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(IS_SPECTRUM(widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);
	if (GTK_WIDGET_DRAWABLE(widget)) {
		spec = SPECTRUM(widget);
		if (!spec->idlefunc)
			spec->idlefunc = gtk_idle_add_priority(PRIO, idle_callback, spec);
	}
	return FALSE;
}

static void draw(Spectrum *spec)
{
	guint segcnt, i;
	GdkPoint pt[SPECTRUM_WIDTH+1];
	GdkSegment seg[100], *segp;
	GtkWidget *widget;
        GdkSegment squelchline[1];
        
// 
	widget = GTK_WIDGET(spec);
	g_return_if_fail(GTK_WIDGET_DRAWABLE(widget));
	g_return_if_fail(spec->pixmap);
	/* calculate grid segments */
        /* vertical: */
	for ( segp = seg, segcnt = i = 0; 
                i < SPECTRUM_WIDTH; 
                i += (SPECTRUM_WIDTH * 500 + SRATE/4) / (SRATE/2)) {
		segp->x1 = segp->x2 = i;
		segp->y1 = 0;
		segp->y2 = SPECTRUM_HEIGHT-1;
		segp++;
		segcnt++;
	}
	/* horizontal: */
        for ( i = 0; 
                    i < SPECTRUM_HEIGHT; 
                    i += (SPECTRUM_HEIGHT * 10 + DBSPAN/2) / DBSPAN) {
		segp->y1 = segp->y2 = i;
		segp->x1 = 0;
		segp->x2 = SPECTRUM_WIDTH-1;
		segp++;
		segcnt++;
	}
        /* squelch line: */
        {
		squelch = SPECTRUM_HEIGHT * (100 - params.general.squelchpercent) / 100; 
                /*
		display_status("spec draw: params.general.squelchpercent = %d", 
		    params.general.squelchpercent);
                display_status("spec draw: squelch = %d", i);
                */
		segp = squelchline;
                segp->y1 = segp->y2 = squelch;
                segp->x1 = 0;
                segp->x2 = SPECTRUM_WIDTH-1;
	}
	/* copy data points */
	for (i = 0; i < SPECTRUM_WIDTH; i++) {
		pt[i].x = i;
		pt[i].y = spec->y[i];
	}
        /* clear window */
	gdk_draw_rectangle(spec->pixmap, widget->style->base_gc[widget->state],
			   TRUE, 0, 0, 
			   widget->allocation.width, 
			   widget->allocation.height);
	/* draw grid */
	gdk_draw_segments(spec->pixmap, spec->grid_gc, seg, segcnt);
        /* draw squelch */
        gdk_draw_segments(spec->pixmap, spec->squelch_gc, squelchline, 1);
        /* draw markers */
	gdk_draw_line(spec->pixmap, spec->space_gc, 
		      spec->space,
		      0, 
		      spec->space, 
		      widget->allocation.height-1);
	gdk_draw_line(spec->pixmap, spec->mark_gc, 
		      spec->mark,
		      0, 
		      spec->mark, 
		      widget->allocation.height-1);
	gdk_draw_line(spec->pixmap, spec->pointer_gc, 
		      spec->pointer,
		      0, 
		      spec->pointer, 
		      widget->allocation.height-1);
	/* draw trace */
	gdk_draw_lines(spec->pixmap, spec->trace_gc, pt, SPECTRUM_WIDTH);
	/* draw to screen */
	gdk_draw_pixmap(widget->window, widget->style->base_gc[widget->state], spec->pixmap, 
			0, 0, 0, 0, widget->allocation.width, widget->allocation.height);
}


static gint idle_callback(gpointer data)
{
	g_return_val_if_fail(data != NULL, FALSE);
	g_return_val_if_fail(IS_SPECTRUM(data), FALSE);
	SPECTRUM(data)->idlefunc = 0;
	if (!GTK_WIDGET_DRAWABLE(GTK_WIDGET(data)))
		return FALSE;
	draw(SPECTRUM(data));
	return FALSE;  /* don't call this callback again */
}

void spectrum_setdata(Spectrum *spec, short *samples)
{
	float fdata[2*SPECTRUM_NUMSAMPLES];
	float f;
	unsigned int i;
	static unsigned int squelch_checkpoints[SQUELCHPOINTS];
	unsigned short squelch_y[SQUELCHPOINTS];
	
	/*
	 * 0 cw
	 * 1 fsk space
	 * 2 fsk mark
	 * 3 mt36_500_1000_a;		 600 Hz
	 * 4 mt36_500_1000_b;		 900 Hz
	 * 5 mt36_1000_1500_a;		1100 Hz
	 * 6 mt36_1000_1500_b;		1400 Hz
	 * 7 mt36_1500_2500_a;		1700 Hz
	 * 8 mt36_1500_2500_b;		2300 Hz
	 *
    	 * 5 - 8 prepared for mt63_bandwidth_auto_detect() 
	 * to come inshallah ! 
	 */
	if (! squelch_checkpoints[0])
	squelch_checkpoints[0] = (SPECTRUM_WIDTH * 440 + SRATE/4) / (SRATE/2); 
	// cw mode planned with 440 Hz default, to be parametrized later
	if (! squelch_checkpoints[1])	
	squelch_checkpoints[1] = spec->space;
	if (! squelch_checkpoints[2])
	squelch_checkpoints[2] = spec->mark;
	if (! squelch_checkpoints[3])
	squelch_checkpoints[3] = (SPECTRUM_WIDTH * 600 + SRATE/4) / (SRATE/2);
	if (! squelch_checkpoints[4])	
	squelch_checkpoints[4] = (SPECTRUM_WIDTH * 900 + SRATE/4) / (SRATE/2);	
	if (! squelch_checkpoints[5])	
	squelch_checkpoints[5] = (SPECTRUM_WIDTH * 1100 + SRATE/4) / (SRATE/2);
	if (! squelch_checkpoints[6])	
	squelch_checkpoints[6] = (SPECTRUM_WIDTH * 1400 + SRATE/4) / (SRATE/2);
	if (! squelch_checkpoints[7])	
	squelch_checkpoints[7] = (SPECTRUM_WIDTH * 1700 + SRATE/4) / (SRATE/2);
	if (! squelch_checkpoints[8])	
	squelch_checkpoints[8] = (SPECTRUM_WIDTH * 2300 + SRATE/4) / (SRATE/2);	

	g_return_if_fail(spec != NULL);
	g_return_if_fail(IS_SPECTRUM(spec));
	
	for (i = 0; i < SPECTRUM_NUMSAMPLES; i++) {
		fdata[2*i] = samples[i] * spec->window[i];
		fdata[2*i+1] = 0;
	}
	fft_rif(fdata, SPECTRUM_NUMSAMPLES, 1);
	for (i = 0; i < SPECTRUM_NUMSAMPLES; i++) {
		f = log10(fsqr(fdata[2*i]) + fsqr(fdata[2*i+1]));
		f *= -(10.0 * SPECTRUM_HEIGHT / DBSPAN);
		if (f < 0)
			f = 0;
		if (f > SPECTRUM_HEIGHT-1)
			f = SPECTRUM_HEIGHT-1;
		spec->y[i] = f;
        }
	/* routines for squelch */
	/* big y means weak signal */
	for (i = 0; i < SQUELCHPOINTS; i++) {
            squelch_y[i] = spec->y[squelch_checkpoints[i]];
	    /*
	    display_status("signal at squelch-checkpoint %d: %d", 
		i, squelch_y[i]);
	    */
	}
	if (squelch_y[0] < squelch) squelch_passed_cw = 1;
	else squelch_passed_cw = 0;
	if ((squelch_y[1] < squelch) || (squelch_y[2] < squelch))
	    squelch_passed_fsk = 1; 
	else  squelch_passed_fsk = 0;
	if ((squelch_y[3] < squelch) || (squelch_y[4] < squelch))
	    squelch_passed_mt36_500_1000 = 1; 
	else squelch_passed_mt36_500_1000 = 0;
	if ((squelch_y[5] < squelch) || (squelch_y[6] < squelch))
	    squelch_passed_mt36_1000_1500 = 1; 
	else squelch_passed_mt36_1000_1500 = 0;
	if ((squelch_y[7] < squelch) || (squelch_y[8] < squelch))
	    squelch_passed_mt36_1500_2500 = 1; 
	else squelch_passed_mt36_1500_2500 = 0;
        /* end of routines for squelch */
	if (GTK_WIDGET_DRAWABLE(GTK_WIDGET(spec))) {
	    if (!spec->idlefunc)
		spec->idlefunc = 
		    gtk_idle_add_priority(PRIO, idle_callback, spec);
	}
}

void spectrum_setmarker(Spectrum *spec, int space, int mark, int pointer)
{
	g_return_if_fail(spec != NULL);
	g_return_if_fail(IS_SPECTRUM(spec));
	if (space >= 0 && space < SRATE/2)
		spec->space = (SPECTRUM_WIDTH * space + SRATE/4) / (SRATE/2);
	if (mark >= 0 && mark < SRATE/2)
		spec->mark = (SPECTRUM_WIDTH * mark + SRATE/4) / (SRATE/2);
	if (pointer >= 0 && pointer < SRATE/2)
		spec->pointer = (SPECTRUM_WIDTH * pointer + SRATE/4) / (SRATE/2);
	if (GTK_WIDGET_DRAWABLE(GTK_WIDGET(spec))) {
		if (!spec->idlefunc)
			spec->idlefunc = gtk_idle_add_priority(PRIO, idle_callback, spec);
	}
}

//static 
void set_fsk_freq(unsigned int mark, unsigned int space)
{
	struct hfapp_msg msg;
	char buf[16];
	GtkEntry *entry;
	Spectrum *spec;
	int shift;

	msg.hdr.type = htonl(HFAPP_MSG_SET_FSKPAR);
	msg.hdr.err = htonl(ERR_NOERR);
	msg.hdr.len = htonl(sizeof(msg.data.fpar));
	if (space < 500)
		space = 500;
	if (space > 3500)
		space = 3500;
	if (mark < 500)
		mark = 500;
	if (mark > 3500)
		mark = 3500;
	params.fsk.freq[0] = space;
	params.fsk.freq[1] = mark;
	msg.data.fpar.freq[0] = htons(params.fsk.freq[0]);
	msg.data.fpar.freq[1] = htons(params.fsk.freq[1]);
	msg_send(&msg);
	snprintf(buf, sizeof(buf), "%d", params.fsk.freq[0]);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "fskspacefreq"));
	gtk_entry_set_text(entry, buf);
	snprintf(buf, sizeof(buf), "%d", params.fsk.freq[1]);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wpar), "fskmarkfreq"));
	gtk_entry_set_text(entry, buf);
	snprintf(buf, sizeof(buf), "%d Hz", params.fsk.freq[0]);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wspec), "specfreqspace"));
	gtk_entry_set_text(entry, buf);
	snprintf(buf, sizeof(buf), "%d Hz", params.fsk.freq[1]);
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(wspec), "specfreqmark"));
	gtk_entry_set_text(entry, buf);
	//
	shift = abs (mark - space);
	//display_status("calculated shift from mark & space: %d", shift);
	set_freq_shift(shift);
	spec = SPECTRUM(gtk_object_get_data(GTK_OBJECT(wspec), "spec"));
	spectrum_setmarker(spec, space, mark, -1);
}

/* ---------------------------------------------------------------------- */


#if 0

//static 
int cb_canvas(FL_OBJECT *ob, Window win, int w, int h, XEvent *ev, void *d)
{
        XWindowAttributes winattrs;     
        GC gc;
        XGCValues gcv;
	XButtonEvent *bev = (XButtonEvent *)ev;
	XMotionEvent *mev = (XMotionEvent *)ev;
	unsigned int freq;
	unsigned char buf[16];

	switch (ev->type) {
	case Expose:  /* GraphicsExpose??*/
		XGetWindowAttributes(fl_get_display(), fl_get_canvas_id(fd_spec->scdisp), &winattrs);
		gcv.line_width = 1;
		gcv.line_style = LineSolid;
		gcv.fill_style = FillSolid;
		gc = XCreateGC(fl_get_display(), pixmap, GCLineWidth | GCLineStyle | GCFillStyle, &gcv);
		XSetState(fl_get_display(), gc, col_background, col_background, GXcopy, AllPlanes);
		XCopyArea(fl_get_display(), pixmap, fl_get_canvas_id(fd_spec->scdisp), gr_context, 
			  0, 0, winattrs.width, winattrs.height, 0, 0);
		return 0;

	case ButtonPress:
		freq = (bev->x * SRATE + SPEC_WIDTH) / (2*SPEC_WIDTH);
		if (bev->button == 1)
			set_fsk_freq(freq+freq_shift, freq);
		else if (bev->button == 3)
			set_fsk_freq(freq, freq-freq_shift);
		snprintf(buf, sizeof(buf), "%d Hz", freq);
		fl_set_object_label(fd_spec->freq_pointer, buf);	
		return 0;

	case MotionNotify:
		freq = (mev->x * SRATE + SPEC_WIDTH) / (2*SPEC_WIDTH);
		snprintf(buf, sizeof(buf), "%d Hz", freq);
		fl_set_object_label(fd_spec->freq_pointer, buf);
		return 0;

	default:
		errprintf (SEV_WARNING,"Canvas: unknown event %d\n", ev->type);
		return 0;
	}
}

#endif

/* --------------------------------------------------------------------- */

#if 0
//static 
void scope_window(int on) 
{
	struct hfapp_msg msg;
	if (on) {
		fl_show_form(fd_spec->spec, FL_PLACE_CENTER, FL_FULLBORDER, 
			     "HF Terminal Spectrum Display");
		if (!(pixmap = XCreatePixmap(fl_get_display(), 
					     fl_get_canvas_id(fd_spec->scdisp), 
					     SPEC_WIDTH, SPEC_HEIGHT, 
					     fl_get_canvas_depth(fd_spec->scdisp))))
			errprintf(SEV_FATAL, "unable to open offscreen pixmap\n");
		fl_add_canvas_handler(fd_spec->scdisp, Expose, cb_canvas, NULL);
		fl_add_canvas_handler(fd_spec->scdisp, MotionNotify, cb_canvas, NULL);
		fl_add_canvas_handler(fd_spec->scdisp, ButtonPress, cb_canvas, NULL);
		gr_context = XCreateGC(fl_get_display(), fl_state[fl_vmode].trailblazer, 0, 0);
		col_zeroline = fl_get_flcolor(FL_RED);
		col_background = fl_get_flcolor(FL_WHITE);
		col_trace = fl_get_flcolor(FL_BLACK);
		col_demodbars = fl_get_flcolor(FL_BLUE);
		
		msg.hdr.type = htonl(HFAPP_MSG_REQ_SAMPLES);
		msg.hdr.len = htonl(sizeof(msg.data.u));
		msg.hdr.err = htonl(ERR_NOERR);
		msg.data.u = htonl(SPEC_WIDTH*2);
		msg_send(&msg);
		scope_on = 1;
		return;
	}
	fl_remove_canvas_handler(fd_spec->scdisp, Expose, cb_canvas);
	fl_remove_canvas_handler(fd_spec->scdisp, MotionNotify, cb_canvas);
	fl_remove_canvas_handler(fd_spec->scdisp, ButtonPress, cb_canvas);
	fl_hide_form(fd_spec->spec);
	XFreeGC(fl_get_display(), gr_context);
	XFreePixmap(fl_get_display(), pixmap);
	scope_on = 0;
}
#endif

/* --------------------------------------------------------------------- */

void scope_draw(float *data)
{
#if 0
        int cnt;
        GC gc;
        XGCValues gcv;
	int yc = 0, ycz;

	if (!scope_on)
		return;
#if 0
        XWindowAttributes winattrs;
        XGetWindowAttributes(fl_get_display(), fl_get_canvas_id(fd_spec->scdisp), &winattrs);
	winattrs.width, winattrs.height;
#endif 
        gcv.line_width = 1;
        gcv.line_style = LineSolid;
        gc = XCreateGC(fl_get_display(), pixmap, GCLineWidth | GCLineStyle, &gcv);
        XSetState(fl_get_display(), gc, col_background, col_background, GXcopy, AllPlanes);
	XFillRectangle(fl_get_display(), pixmap, gc, 0, 0, SPEC_WIDTH, SPEC_HEIGHT);
	/*
	 * draw grid
	 */
        XSetForeground(fl_get_display(), gc, col_zeroline);
	for (cnt = 0; cnt < SPEC_HEIGHT; cnt += (10 * SPEC_HEIGHT + DBSPAN / 2) / DBSPAN)
		XDrawLine(fl_get_display(), pixmap, gc, 0, cnt, SPEC_WIDTH-1, cnt);
	for (cnt = 0; cnt < SPEC_WIDTH; cnt += (500 * (2*SPEC_WIDTH) + SRATE / 2) / SRATE)
		XDrawLine(fl_get_display(), pixmap, gc, cnt, 0, cnt, SPEC_HEIGHT-1);
	/*
	 * draw demodulator frequency bars
	 */
        XSetForeground(fl_get_display(), gc, col_demodbars);
	cnt = (fsk_freq[0] * (2*SPEC_WIDTH) + SRATE / 2) / SRATE;
	XDrawLine(fl_get_display(), pixmap, gc, cnt, 0, cnt, SPEC_HEIGHT-1);
	cnt = (fsk_freq[1] * (2*SPEC_WIDTH) + SRATE / 2) / SRATE;
	XDrawLine(fl_get_display(), pixmap, gc, cnt, 0, cnt, SPEC_HEIGHT-1);	
	/*
	 * draw curve
	 */
        XSetForeground(fl_get_display(), gc, col_trace);
        for (cnt = 0; cnt < SPEC_WIDTH; cnt++) {
		ycz = yc;
		yc = -data[cnt] / DBSPAN * SPEC_HEIGHT;
		if (yc < 0) 
			yc = 0;
		if (yc >= SPEC_HEIGHT)
			yc = SPEC_HEIGHT-1;
		if (cnt > 0)
			XDrawLine(fl_get_display(), pixmap, gc, cnt-1, ycz, cnt, yc);
	}
        XCopyArea(fl_get_display(), pixmap, fl_get_canvas_id(fd_spec->scdisp), gr_context, 0, 0, 
                  SPEC_WIDTH, SPEC_HEIGHT, 0, 0);
        XFreeGC(fl_get_display(), gc);
        XSync(fl_get_display(), 0);
#endif
}

void spec_samples(short *data, unsigned int len)
{
	struct hfapp_msg msg;
	short tmp[SPECTRUM_NUMSAMPLES];
	unsigned int i;
	Spectrum *spec;

	if (!scope_on)
		return;
	if (len < SPECTRUM_NUMSAMPLES)
		errprintf(SEV_WARNING, "HFAPP_MSG_ACK_SAMPLES: message too short\n");
	else {
		for (i = 0; i < SPECTRUM_NUMSAMPLES; i++)
			tmp[i] = ntohs(data[i]);
		spec = SPECTRUM(gtk_object_get_data(GTK_OBJECT(wspec), "spec"));
		spectrum_setdata(spec, tmp);
	}
	msg.hdr.type = htonl(HFAPP_MSG_REQ_SAMPLES);
	msg.hdr.len = htonl(sizeof(msg.data.u));
	msg.hdr.err = htonl(ERR_NOERR);
	msg.data.u = htonl(SPECTRUM_NUMSAMPLES);
	msg_send(&msg);
}

/* --------------------------------------------------------------------- */

//static 
unsigned int get_freq_shift(void)
{
	GtkToggleButton *tog;

	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wspec), "shift170"));
	if (gtk_toggle_button_get_active(tog))
		return 170;
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wspec), "shift200"));
	if (gtk_toggle_button_get_active(tog))
		return 200;
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wspec), "shift425"));
	if (gtk_toggle_button_get_active(tog))
		return 425;
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wspec), "shift800"));
	if (gtk_toggle_button_get_active(tog))
		return 800;
	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data(GTK_OBJECT(wspec), "shiftother"));
	if (gtk_toggle_button_get_active(tog))
		return 0;
	return 0;
}

void set_freq_shift(int shift)
{
	GtkToggleButton *tog = NULL;
	shift = abs(shift);
	switch (shift) {
            case 170:
        	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data
			(GTK_OBJECT(wspec), "shift170"));
                break;
	    case 200:
        	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data
			(GTK_OBJECT(wspec), "shift200"));
                break;
	    case 425:
        	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data
			(GTK_OBJECT(wspec), "shift425"));
                break;
	    case 800:
        	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data
			(GTK_OBJECT(wspec), "shift800"));
                break;
            default:
        	tog = GTK_TOGGLE_BUTTON(gtk_object_get_data
			(GTK_OBJECT(wspec), "shiftother"));
        	//display_status("user-defined shift %d", shift);
	        break;
            }
	if(tog) gtk_toggle_button_set_active(tog, TRUE);
}
