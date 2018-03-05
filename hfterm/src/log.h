#ifndef _LOG_H
#define _LOG_H

#include "hft.h"

#ifndef MAXLOG 
#define MAXLOG 50
#endif

extern int actlog;

struct logline {
	unsigned char time[32];
	unsigned char call[16];
	unsigned char name[32];
	unsigned char qth[32];
	unsigned char rstin[8];
	unsigned char rstout[8];
	unsigned char mode[8];
	unsigned char band[16];
	unsigned char notes[64];
};

struct logbook
{
	int logsize, olderlogs;
	struct logline line[MAXLOG +1];
};
extern struct logbook lb;

void log_get(int);
void log_set(int);
void log_search();
void log_list();
void log_save();
void log_delete_entry(int);
void log_shift_entries(int i);
void log_delete_all();
void log_read();
void log_store();
void log_archivate();

#endif /* _LOG_H */
