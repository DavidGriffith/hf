#include "log.h"
struct logbook lb;

/* number of logentry just being edited, this is important for
   editing old entries. 0 if new entry is being created.-------- */
int actlog = 0;

void log_get(int i)
{
/* get entry out of log frame and put it into struct lb.line[i].
   If log_on == 1 we use log frame of main window,
   else we use the qso-editor popup window. ---------------------*/	
	GtkEntry *entry;
	GtkObject *logwin;
	if (log_on == 1)
	{
	  logwin = GTK_OBJECT(wmain);
	}
	else
	{
	  logwin = GTK_OBJECT(wqsoeditor);
	}
	
	//only for new log entry !!!
	if (actlog == 0)
	{
	  timequery();
	  strncpy(lb.line[i].time, gmt, sizeof(gmt));
	}

	entry = GTK_ENTRY(gtk_object_get_data(logwin, "entry_call"));
	strncpy(lb.line[i].call, gtk_entry_get_text(entry), sizeof(lb.line[i].call));

	entry = GTK_ENTRY(gtk_object_get_data(logwin, "entry_name"));
	strncpy(lb.line[i].name, gtk_entry_get_text(entry), sizeof(lb.line[i].name));

	entry = GTK_ENTRY(gtk_object_get_data(logwin, "entry_qth"));
	strncpy(lb.line[i].qth, gtk_entry_get_text(entry), sizeof(lb.line[i].qth));

	entry = GTK_ENTRY(gtk_object_get_data(logwin, "entry_rstin"));
	strncpy(lb.line[i].rstin, gtk_entry_get_text(entry), sizeof(lb.line[i].rstin));

	entry = GTK_ENTRY(gtk_object_get_data(logwin, "entry_rstout"));
	strncpy(lb.line[i].rstout, gtk_entry_get_text(entry), sizeof(lb.line[i].rstout));

	entry = GTK_ENTRY(gtk_object_get_data(logwin, "entry_notes"));
	strncpy(lb.line[i].notes, gtk_entry_get_text(entry), sizeof(lb.line[i].notes));

	entry = GTK_ENTRY(gtk_object_get_data(logwin, "combo_entry_mode"));
	strncpy(lb.line[i].mode, gtk_entry_get_text(entry), sizeof(lb.line[i].mode));

	entry = GTK_ENTRY(gtk_object_get_data(logwin, "combo_entry_band"));
	strncpy(lb.line[i].band, gtk_entry_get_text(entry), sizeof(lb.line[i].band));
}

void log_set(int i)
{
/* write entry out of struct into log frame .
   If log_on == 1 we use log frame of main window,
   else we use the qso-editor popup window. ---------------------*/	
	GtkEntry *entry;
	GtkObject *logwin;
	unsigned char buf[129];
	
	if (log_on == 1)
	{
	  logwin = GTK_OBJECT(wmain);
	}
	else
	{
	  logwin = GTK_OBJECT(wqsoeditor);
	}
	
	entry = GTK_ENTRY(gtk_object_get_data(logwin, "entry_call"));
	strncpy(buf, lb.line[i].call, sizeof(lb.line[i].call));
	buf[sizeof(lb.line[i].call)] = 0;
	gtk_entry_set_text(entry, buf);

	entry = GTK_ENTRY(gtk_object_get_data(logwin, "entry_name"));
	strncpy(buf, lb.line[i].name, sizeof(lb.line[i].name));
	buf[sizeof(lb.line[i].name)] = 0;
	gtk_entry_set_text(entry, buf);
	
	entry = GTK_ENTRY(gtk_object_get_data(logwin, "entry_qth"));
	strncpy(buf, lb.line[i].qth, sizeof(lb.line[i].qth));
	buf[sizeof(lb.line[i].qth)] = 0;
	gtk_entry_set_text(entry, buf);

	entry = GTK_ENTRY(gtk_object_get_data(logwin, "entry_rstin"));
	strncpy(buf, lb.line[i].rstin, sizeof(lb.line[i].rstin));
	buf[sizeof(lb.line[i].rstin)] = 0;
	gtk_entry_set_text(entry, buf);

	entry = GTK_ENTRY(gtk_object_get_data(logwin, "entry_rstout"));
	strncpy(buf, lb.line[i].rstout, sizeof(lb.line[i].rstout));
	buf[sizeof(lb.line[i].rstout)] = 0;
	gtk_entry_set_text(entry, buf);

	entry = GTK_ENTRY(gtk_object_get_data(logwin, "entry_notes"));
	strncpy(buf, lb.line[i].notes, sizeof(lb.line[i].notes));
	buf[sizeof(lb.line[i].notes)] = 0;
	gtk_entry_set_text(entry, buf);

	entry = GTK_ENTRY(gtk_object_get_data(logwin, "combo_entry_mode"));
	strncpy(buf, lb.line[i].mode, sizeof(lb.line[i].mode));
	buf[sizeof(lb.line[i].mode)] = 0;
	gtk_entry_set_text(entry, buf);

	entry = GTK_ENTRY(gtk_object_get_data(logwin, "combo_entry_band"));
	strncpy(buf, lb.line[i].band, sizeof(lb.line[i].band));
	buf[sizeof(lb.line[i].band)] = 0;
	gtk_entry_set_text(entry, buf);
}

void log_search()
{
	GtkEntry *entry;
	char search_call[16];
	int i = 0;
	entry = GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(Wsearchlogentr), "searchcall"));
	strncpy(search_call, gtk_entry_get_text(entry), sizeof(search_call));
	for (i = 1; i <= lb.logsize; i++)
	{
	  if (strcmp(lb.line[i].call, search_call) == 0)
	  {
	    actlog = i;
	    display_status("log entry found");
	    return;
	  }
	}
	display_status("sorry, log entry not found.");
	return;
}


void log_list()
{
	int i;
	gtk_clist_clear(GTK_CLIST (gtk_object_get_data(GTK_OBJECT(wlistalllog), "clist1")));
	for (i = 1; i <= lb.logsize; i++)
	{
	  char* row[9];
	  row[0] = lb.line[i].time;
 	  row[1] = lb.line[i].call;
	  row[2] = lb.line[i].name;
	  row[3] = lb.line[i].qth;
	  row[4] = lb.line[i].rstin;
	  row[5] = lb.line[i].rstout;
	  row[6] = lb.line[i].mode;
	  row[7] = lb.line[i].band;
	  row[8] = lb.line[i].notes;
	  gtk_clist_append (GTK_CLIST (gtk_object_get_data(GTK_OBJECT(wlistalllog), "clist1")), row);
	}
}

void log_save()
{
	if (actlog == 0)
	// means: if not editing old logentry, 
	// means: log is growing
	{
	  if (lb.logsize >= MAXLOG)
	  {
	    fprintf(stderr,"log size has reached maximum size %u!\n", MAXLOG);
	    log_archivate();
	    log_delete_all();
	  }
	  lb.logsize ++;
	  log_get(lb.logsize);
	}
	if (actlog > 0)
	// means: editing old logentry
	{
	  if (actlog > lb.logsize)
	  {
	    fprintf(stderr, "error in search for logentry.");
	    return;
	  }
	else
	  {
	    log_get(actlog);
	    display_status("corrected log entry will be saved.");
	    actlog = 0;
	    /* setting actlog = 0 means switching to normal modus:
	    log can go on growing  */
	  }
	}
	log_store();
}

void log_delete_entry(int i)
{
	memset(lb.line[i].time, 0, sizeof(lb.line[i].time));
	memset(lb.line[i].call, 0, sizeof(lb.line[i].call));
	memset(lb.line[i].name, 0, sizeof(lb.line[i].name));
	memset(lb.line[i].qth, 0, sizeof(lb.line[i].qth));
	memset(lb.line[i].rstin, 0, sizeof(lb.line[i].rstin));
	memset(lb.line[i].rstout, 0, sizeof(lb.line[i].rstout));
	memset(lb.line[i].notes, 0, sizeof(lb.line[i].notes));
	memset(lb.line[i].mode, 0, sizeof(lb.line[i].mode));
	memset(lb.line[i].band, 0, sizeof(lb.line[i].band));
	return;
}

void log_shift_entries(int j)
{
/* use this after deleting an entry: shift rest down to fill gap */	
	int i;
	for (i = j; i < lb.logsize; i++)
	{
	  strcpy (lb.line[i].time, lb.line[i+1].time);
	  strcpy(lb.line[i].call, lb.line[i+1].call);
	  strcpy(lb.line[i].name,  lb.line[i+1].name);
	  strcpy(lb.line[i].qth, lb.line[i+1].qth);
	  strcpy(lb.line[i].rstin, lb.line[i+1].rstin);
	  strcpy(lb.line[i].rstout, lb.line[i+1].rstout);
	  strcpy(lb.line[i].notes,  lb.line[i+1].notes);
	  strcpy(lb.line[i].mode, lb.line[i+1].mode);
	  strcpy(lb.line[i].band, lb.line[i+1].band);
	}
	lb.logsize--;
	return;
}

void log_delete_all()
{
	int i;
	for (i = 0; i <= lb.logsize; i++)
	{
	  log_delete_entry(i);
	}
	lb.logsize = 0;
	return;
}

void log_read()
{
/* aus log-Datei gespeicherte Werte einlesen ------------------------ */
	FILE *logfile = fopen("hf/hflog.bin", "r");
	lb.logsize = 0;
	lb.olderlogs = 0;
	if(logfile == NULL)
	{
	  fprintf(stderr, "\nlogfile ~/hf/hflog.bin can not be opened.\n");
	  return;
	}  
	else	
	{
	  //fprintf(stderr, "logfile ~/hf/hflog.bin opened.\n");
	  if(fread(&lb, sizeof(lb), 1, logfile) != 1)
		fprintf(stderr, "\n\tError while reading logfile.\n");
	  fclose(logfile);	
	}
	return;
}

void log_store()
{
/* - Log-Daten speichern binär (zur Weiterverwendung im Programm)
 und als Textdatei (für andere Programme, Editoren, Drucken).------*/
/* Platzberechnung von logrow: um Division durch 0 auszuweichen,
 'lb.logsize +1' im Nenner !!! */
	FILE *logbinfile, *logtxtfile;
	char loghead[128], logrow[(sizeof(lb) / (lb.logsize + 1))+256];
	char logtxt[sizeof(lb) + sizeof(loghead) + (32 * lb.logsize)];
	int i;
	
	logbinfile = fopen("hf/hflog.bin", "w");
	if(logbinfile == NULL)
	{
	  fprintf(stderr, "logfile ~/hf/hflog.bin can not be opened.\n");
	  return;
	}
	else
	{
	  if(fwrite(&lb, sizeof(lb), 1, logbinfile) != 1)
	  {
	    fprintf(stderr,	"Error while writing logfile ~/hf/hflog.bin !\n");
	    return;
	  }
	  fclose(logbinfile);	
	}

	logtxtfile = fopen("hf/hflog.txt", "w");
	if(logtxtfile == NULL)
	{
	  fprintf(stderr, "logfile ~/hf/hflog.txt can not be opened.\n");
	  return;
	}
	else
	{
	  sprintf(loghead, 
	  "* * * * hfterm - logfile by %s * * * *\n%02hu\tentries\n%02hu\tolder logs\n\n", 
	  params.pactor.mycall, lb.logsize, lb.olderlogs);
	  strcpy(logtxt, loghead);
	  for ( i = 1; i <= lb.logsize; i++)
	  {
	    // ein bißchen cabrillo-adaptiert
	    sprintf(logrow, "%s %s\t%s\t%s\t%s %s out %s in %s %s\n",  				lb.line[i].time,  lb.line[i].call, lb.line[i].name, 				lb.line[i].qth, lb.line[i].band, lb.line[i].mode,
	    	lb.line[i].rstout, lb.line[i].rstin, lb.line[i].notes);	  
	    strcat(logtxt, logrow); 	  
	  }
	  fprintf(logtxtfile, logtxt);	    
	  fclose(logtxtfile);	
	display_status("log stored.");
	}
}

void log_archivate()
{
/* -- alte Logdatei umbenennen, Index lb.olderlogs um 1 erhöhen -----------*/
	char oldlogname[32];
	memset(oldlogname, 0, sizeof(oldlogname));
	sprintf (oldlogname, "hf/hfoldlog.%02hu.txt", (lb.olderlogs + 1));
	if ( (rename("hf/hflog.txt", oldlogname)) != 0)
	{
  	  fprintf(stderr, "logfile ~/hf/hflog.txt can not be renamed to %s.\n", 			oldlogname);
	  return;	
	}
	else
	{
  	  fprintf(stderr, "logfile ~/hf/hflog.txt has been renamed to %s.\n", 			oldlogname);
	  lb.olderlogs++;
	  display_status("Good OM! One more old log archivated in ~/hf!");
	}
}


