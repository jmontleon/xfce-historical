/*
 * xtree_cb.c: cb functions which used to live in xtree_gui.c
 *
 * Copyright (C) 1999 Rasca, Berlin
 * EMail: thron@gmx.de
 *
 * Olivier Fourdan (fourdan@xfce.org)
 * Heavily modified as part of the Xfce project (http://www.xfce.org)
 *
 * Edscott Wilson Garcia 2001, for xfce project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <utime.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkenums.h>
#include "constant.h"
#include "my_intl.h"
#include "my_string.h"
#include "xpmext.h"
#include "xtree_functions.h"
#include "xtree_gui.h"
#include "gtk_dlg.h"
#include "gtk_exec.h"
#include "gtk_prop.h"
#include "gtk_dnd.h"
#include "xtree_cfg.h"
#include "xtree_dnd.h"
#include "entry.h"
#include "uri.h"
#include "io.h"
#include "top.h"
#include "reg.h"
#include "xfcolor.h"
#include "xfce-common.h"
#include "xtree_tar.h"

#include "xtree_mess.h"
#include "xtree_pasteboard.h"
#include "xtree_go.h"
#include "xtree_cpy.h"
#include "xtree_cfg.h"


#ifdef HAVE_GDK_IMLIB
#include <gdk_imlib.h>
#endif

#ifndef HAVE_SNPRINTF
#  include "snprintf.h"
#endif

#ifdef DMALLOC
#  include "dmalloc.h"
#endif


static gboolean abort_delete=FALSE;

char *valid_path(GtkCTree *ctree,gboolean expand){
  GtkCTreeNode *node;
  entry *en;
  count_selection (GTK_CTREE (ctree), &node);
  en = gtk_ctree_node_get_row_data (GTK_CTREE (ctree), node);
  while (!(en->type&FT_DIR)||(en->type&FT_TARCHILD)){
	  node=GTK_CTREE_ROW (node)->parent;
	  en = gtk_ctree_node_get_row_data (GTK_CTREE (ctree), node);
  }
  if ((expand)&&(!GTK_CTREE_ROW (node)->expanded))
	 gtk_ctree_expand (GTK_CTREE (ctree), node);
  return en->path;
}

/*
 * find a node and check if it is expanded
 */
static void
node_is_open (GtkCTree * ctree, GtkCTreeNode * node, void *data)
{
  GtkCTreeRow *row;
  entry *check = (entry *) data;
  entry *en = gtk_ctree_node_get_row_data (ctree, node);
  if (strcmp (en->path, check->path) == 0)
  {
    row = GTK_CTREE_ROW (node);
    if (row->expanded)
    {
      check->label = (char *) node;
      check->flags = TRUE;
    }
  }
}

static int
compare_node_path (gconstpointer ptr1, gconstpointer ptr2)
{
  entry *en1 = (entry *) ptr1, *en2 = (entry *) ptr2;

  return strcmp (en1->path, en2->path);
}

static int errno_error_continue(GtkWidget *parent,char *path){
	return xf_dlg_error_continue (parent,strerror(errno),path);
}

static void
node_unselect_by_type (GtkCTree * ctree, GtkCTreeNode * node, void *data)
{
  entry *en;

  en = gtk_ctree_node_get_row_data (ctree, node);
  if (en->type & (int) ((long) data))
  {
    gtk_ctree_unselect (ctree, node);
  }
}

void
cb_open_trash (GtkWidget * item, GtkCTree *ctree)
{
  cfg *win;
  win = gtk_object_get_user_data (GTK_OBJECT (ctree));
  new_top (win->trash, win->xap, win->trash, win->reg, win->width, win->height, 0);
}

void
cb_new_window (GtkWidget * widget, GtkCTree * ctree)
{
  entry *en;
  cfg *win;
  char *path;

  win = gtk_object_get_user_data (GTK_OBJECT (ctree));
  en = gtk_ctree_node_get_row_data (ctree, GTK_CTREE_NODE (GTK_CLIST (ctree)->row_list));
  path=valid_path((GtkCTree *)ctree,FALSE);
  new_top (path, win->xap, win->trash, win->reg, win->width, win->height, en->flags); 
}

void
cb_select (GtkWidget * item, GtkCTree * ctree)
{
  int num;
  GtkCTreeNode *node;

  num = count_selection (ctree, &node);
  if (!GTK_CTREE_ROW (node)->expanded)
    node = GTK_CTREE_ROW (node)->parent;
  gtk_ctree_select_recursive (ctree, node);
  gtk_ctree_unselect (ctree, node);
  gtk_ctree_pre_recursive (ctree, node, node_unselect_by_type, (gpointer) ((long) FT_DIR_UP));
}

void
cb_unselect (GtkWidget * widget, GtkCTree * ctree)
{
  gtk_ctree_unselect_recursive (ctree, NULL);
}

/* function to call xfdiff */
void
cb_diff (GtkWidget * widget,  GtkCTree * ctree)
{
  /* use:
   * prompting for left and right files: xfdiff [left file] [right file]
   * without prompting for files:        xfdiff -n  
   * prompting for patch dir and file:   xfdiff -p [directory] [patch file]
   * */
  int num;
  GtkCTreeNode *node;
  char *command;
  entry *en_1,*en_2;
  GList *selection;
  cfg *win;
  win = gtk_object_get_user_data (GTK_OBJECT (ctree));
  
  num = count_selection (ctree, &node);
  if (num) {
    selection = GTK_CLIST (ctree)->selection;
    while (selection){	  
     node = selection->data;
     en_1 = gtk_ctree_node_get_row_data (ctree, node);
     if (en_1->type & FT_TARCHILD) gtk_ctree_unselect (ctree, node);
     selection=selection->next;
    }	    
  }
  num = count_selection (ctree, &node);
  
  if (!num) {
    io_system ("xfdiff&");
    return;
  }
  /* take first 2 */
/*  if (num > 2) {
    if (xf_dlg_continue (win->top,_("Proceed with the first 2 selections?"),NULL)==DLG_RC_CANCEL)
	    return;
  }*/
  
  selection = GTK_CLIST (ctree)->selection;
  node = selection->data;
  en_1 = gtk_ctree_node_get_row_data (ctree, node);
  if (en_1->type & FT_TARCHILD){io_system ("xfdiff&"); return;}
  selection=selection->next;
  if (selection){
	node = selection->data;
	en_2 = gtk_ctree_node_get_row_data (ctree, node);
        if (en_2->type & FT_TARCHILD){io_system ("xfdiff&"); return;} 
	command=(char *)malloc(strlen("xfdiff")+strlen(en_1->path)+strlen(en_2->path)+6);
  	if (!command) return;
  	sprintf(command,"xfdiff %s %s&",en_1->path,en_2->path);
  } else {
  	command=(char *)malloc(strlen("xfdiff")+strlen(en_1->path)+4);
  	if (!command) return;
  	sprintf(command,"xfdiff %s&",en_1->path);
  }
  io_system (command);
  free(command);
}

/* function2 to call xfdiff */
void
cb_patch (GtkWidget * widget,  GtkCTree * ctree)
{
  /* use:
   * prompting for left and right files: xfdiff [left file] [right file]
   * without prompting for files:        xfdiff -n  
   * prompting for patch dir and file:   xfdiff -p [directory] [patch file]
   * */
    io_system ("xfdiff -p&");
}


/*
 * really delete files incl. subs
 */

gboolean
delete_files (GtkWidget *ctree,char *path)
{
  struct stat st;
  DIR *dir;
  char *test;
  struct dirent *de;
  char *complete;
  gboolean tar_entry;
  cfg *win;

  win = gtk_object_get_user_data (GTK_OBJECT (ctree));

  /*printf("dbg:delete_files():%s\n",path);fflush(NULL);*/
  if (abort_delete) return TRUE;
  tar_entry=(strncmp(path,"tar:",strlen("tar:")))?FALSE:TRUE;
  if (!tar_entry) {
	 if (lstat (path, &st) == -1) goto delete_error_errno;
	 if ((test = strrchr (path, '/')))  {
	   test++;
	   if (!io_is_valid (test)) goto delete_error;
 	 }
  }
  if (!tar_entry && S_ISDIR (st.st_mode) && (!S_ISLNK (st.st_mode)))
  {
    if (access (path, R_OK | W_OK) == -1)goto delete_error;
    if ((dir = opendir (path))==NULL) goto delete_error;
    while ((de = readdir (dir)) != NULL)
    {
      if (io_is_current (de->d_name)) continue;
      if (io_is_dirup (de->d_name))   continue;
      if ((complete = (char *)malloc(strlen(path)+strlen(de->d_name)+2))==NULL) continue;
      sprintf (complete, "%s/%s", path, de->d_name);
      delete_files (ctree,complete);
      free(complete);
    }
    closedir (dir);
    if (rmdir (path)<1)goto delete_error_errno; 
  }
  else
  {
    if (tar_entry){
	    /*printf("dbg:tar delete %s\n",path);*/
	    if (tar_delete((GtkCTree *)ctree,path) < 0) {
		    /*printf("dbg: error returned from tar_delete\n");*/
		    goto delete_error_errno;
	    }
    } else if (unlink (path)<1){
/*	    printf("dbg:%d:%s\n",errno,strerror(errno));*/
	    goto delete_error_errno; 
    }
  }
  return TRUE;
delete_error:
  if (xf_dlg_new (win->top,_("error deleting file"),path,NULL,DLG_CONTINUE|DLG_CANCEL)==DLG_RC_CANCEL)
	  abort_delete=TRUE;
  return FALSE;
delete_error_errno:
  if ((errno)&&(xf_dlg_new (win->top,strerror(errno),path,NULL,DLG_CONTINUE|DLG_CANCEL)==DLG_RC_CANCEL)) abort_delete=TRUE;	  
  return FALSE;
  
  
}

/*
 * empty trash folder
 */
void
cb_empty_trash (GtkWidget * widget, GtkCTree * ctree)
{
  GtkCTreeNode *node;
  cfg *win;
  DIR *dir;
  struct dirent *de;
  char *complete;
  entry check;

  win = gtk_object_get_user_data (GTK_OBJECT (ctree));
  check.path = win->trash;
  check.flags = FALSE;
  if (!win)
    return;
  /* check if the trash dir is open, so we have to update */
  gtk_ctree_pre_recursive (ctree, GTK_CTREE_NODE (GTK_CLIST (ctree)->row_list), node_is_open, &check);
  dir = opendir (win->trash);
  if (!dir)
    return;
  cursor_wait (GTK_WIDGET (ctree));
  while ((de = readdir (dir)) != NULL)
  {
    if (io_is_current (de->d_name)) continue;
    if (io_is_dirup (de->d_name)) continue;
    if ((complete = (char *)malloc(strlen(win->trash)+strlen(de->d_name)+2))==NULL) continue;
    sprintf (complete, "%s/%s", win->trash, de->d_name);
    delete_files ((GtkWidget *)ctree,complete);

    if (check.flags)
    {
      /* remove node */
      check.path = complete;
      node = gtk_ctree_find_by_row_data_custom (ctree, GTK_CTREE_NODE (GTK_CLIST (ctree)->row_list), &check, compare_node_path);
      if (node)
      {
	gtk_ctree_remove_node (ctree, node);
      }
    }
    free(complete);
  }
  closedir (dir);
  cursor_reset (GTK_WIDGET (ctree));
}

/*
 * menu callback for deleting files
 */
void
cb_delete (GtkWidget * widget, GtkCTree * ctree)
{
    static char *fname=NULL;
    static char *mname=NULL;
    int fnamelen;
    FILE *tmpfile;
    FILE *movefile;
    int zapitems=0,moveitems=0;  
  int num, i;
  GtkCTreeNode *node;
  entry *en;
  int result;
  int ask = TRUE;
  int ask_again = TRUE;
  cfg *win;
  struct stat st_target;
  struct stat st_trash;
  GList *selection;

  
  win = gtk_object_get_user_data (GTK_OBJECT (ctree));
  abort_delete=FALSE;
  num = count_selection (ctree, &node);
  if (!num)
  {
    /* nothing to delete */
    xf_dlg_warning (win->top,_("No files marked !"));
    return;
  }
  selection = GTK_CLIST (ctree)->selection;
  
  /*freezeit */
  ctree_freeze (ctree);
  
     
  if (fname) free(fname);
  if (mname) free(mname);
  fnamelen=strlen("/tmp/xftree.9999.tmp")+1;
  srandom(time(NULL));
  fname = (char *)malloc(sizeof(char)*(fnamelen));
  mname = (char *)malloc(sizeof(char)*(fnamelen));
  if (!fname) return ; if (!mname) return ;
  sprintf(fname,"/tmp/xftree.%d.tmp",(int)((9999.0/RAND_MAX)*random()));
  sprintf(mname,"/tmp/xftree.%d.tmp",(int)((9999.0/RAND_MAX)*random()));
  
  /*fprintf(stderr,"dbg:fname=%s,mname=%s",fname,mname);*/
  
  if ((tmpfile=fopen(fname,"w"))==NULL) return ;
  if ((movefile=fopen(mname,"w"))==NULL){
	  fclose(tmpfile); 
	  unlink(fname);
	  return ;
  }

  for (i = 0; (i < num)&&(selection!=NULL); i++,selection=selection->next){
    gboolean zap,tar_entry;
    zap=FALSE;
    node = selection->data;
    en = gtk_ctree_node_get_row_data (ctree, node);
    tar_entry=(strncmp(en->path,"tar:",strlen("tar:")))?FALSE:TRUE;
    if (!io_is_valid (en->label) || (en->type & FT_DIR_UP) || (tar_entry && (en->type & FT_DIR))) {
      /* we do not process ".." */
      gtk_ctree_unselect (ctree, node);
      continue;
    }
    if (tar_entry) zap=TRUE;
    else {
      if (lstat (en->path, &st_target) == -1) {
	if (errno_error_continue(win->top,en->path) == DLG_RC_CANCEL) goto delete_done;
      }
      if (stat (win->trash, &st_trash) == -1) {
	if (errno_error_continue(win->top,win->trash) == DLG_RC_CANCEL) goto delete_done;
      }
      /* only regular files and links to trash */
      if (!(en->type & FT_FILE) && !(en->type & FT_LINK)) zap=TRUE;
      /* files already in trash get zapped */
      if (my_strncmp (en->path, win->trash, strlen (win->trash) )==0 )zap=TRUE;
      /* files > 1MB or on different device get zapped */
      if ((st_target.st_dev != st_trash.st_dev) || (st_target.st_size > 1048576))zap=TRUE;
    } 
    if (ask) {
      if (num - i == 1)
	result = xf_dlg_question (win->top,_("Delete item ?"), en->path);
      else
	result = xf_dlg_question_l (win->top,_("Delete item ?"), en->path, DLG_ALL | DLG_SKIP);
    }
    else result = DLG_RC_ALL;
    if (result == DLG_RC_CANCEL) goto delete_done;
    else if (result == DLG_RC_OK || result == DLG_RC_ALL)  {
      if (result == DLG_RC_ALL) ask = FALSE;
      if (zap) {
	if (ask_again) { 
		if (num - i == 1)
			result = xf_dlg_question (win->top,
					_("Can't move file to trash, hard delete ?"),
				        en->path);
		else	
			result = xf_dlg_question_l (win->top,
					_("Can't move file to trash, hard delete ?"), 
					en->path, DLG_ALL | DLG_SKIP);

			
		if (result == DLG_RC_ALL)  ask_again = FALSE;
	} else result=DLG_RC_OK;
      }
      if ((result == DLG_RC_ALL)||(result ==DLG_RC_OK)){ 
	    if (zap) {
                    fprintf(tmpfile,"%lu:%s:%s/%s\n",(long unsigned)node,
				    en->path,win->trash,en->label);
		    zapitems++; 
	    } else {
                    fprintf(movefile,"%d:%s:%s/%s\n",TR_MOVE,
				    en->path,win->trash,en->label);
		    moveitems++;
	    }
            /*fprintf(stderr,"%d:%s:%s/%s\n",TR_MOVE,
				    en->path,win->trash,en->label);*/
            fflush(NULL); 
      }
    }
  }
  
  fclose (tmpfile);
  fclose (movefile);

  if (moveitems) DirectTransfer((GtkWidget *)ctree,TR_MOVE,mname);

  if (zapitems) {
    char line[256];
    if ((tmpfile=fopen(fname,"r"))==NULL) {
     unlink(mname);
     unlink(fname);
     return;
    }
    while (!feof(tmpfile)&&fgets(line,255,tmpfile)){
	    char *w,*word;
	    word=strtok(line,":"); if (!word) continue;
	    word=strtok(NULL,"\n"); if (!word) continue;
  	/*printf("dbg:w:%s\n",word);fflush(NULL);*/
	    w=strrchr(word,':'); if (!w) continue; else *w=0;
  	/*printf("dbg:w:%s\n",word);fflush(NULL);*/
	    delete_files ((GtkWidget *)ctree,word);
    }
    fclose(tmpfile);
  }
  
  /* immediate refresh */
delete_done: 
  unlink(mname);
  unlink(fname);
  
  ctree_thaw (ctree);
  update_timer (ctree);
}

void
cb_refresh (GtkWidget * widget, GtkWidget * ctree){
  update_timer (GTK_CTREE (ctree));
}
	
void
cb_find (GtkWidget * item, GtkWidget * ctree)
{
  char *cmd,*path;
  path=valid_path((GtkCTree *)ctree,TRUE);
  cmd=(char *)malloc(strlen(path)+1+10);
  if (!cmd) return;
  sprintf (cmd, "xfglob %s&",path);
  io_system (cmd);  free(cmd);  
}

void
cb_about (GtkWidget * item, GtkWidget * ctree)
{
  cfg *win;
  win = gtk_object_get_user_data (GTK_OBJECT (ctree));
  xf_dlg_info (win->top,_("This is XFTree (C) under GNU GPL\n" "with code contributed by:\n" "Rasca, Berlin\n" "Olivier Fourdan\n" "Edscott Wilson Garcia\n"));
}

/*
 * create a new folder in the current
 */
void
cb_new_subdir (GtkWidget * item, GtkWidget * ctree)
{
  char *path, *new_path,*label, *entry_return, *fullpath;
  cfg *win;

  win = gtk_object_get_user_data (GTK_OBJECT (ctree));
  
  path=valid_path((GtkCTree *)ctree,TRUE);
  if (!path) return;

  
  label=(char *)malloc(2+strlen(_("New_Folder")));
  if (!label) return;
  
  new_path=(char *)malloc(2+strlen(path));
  if (!new_path){
	  free (label);
	  return;
  }
  if (path[strlen (path) - 1] == '/') sprintf (new_path, "%s", path);
  else sprintf (new_path, "%s/", path);
  
  strcpy (label, _("New_Folder"));
  entry_return = (char *)xf_dlg_string (win->top,new_path, label);
  if (!entry_return) {
	 free(new_path); free(label); 
	 return; /* cancelled button pressed */
  }
  
  fullpath = (char *)malloc(strlen(new_path)+strlen(entry_return)+2);
  if (!fullpath){
	 xf_dlg_error(win->top,"xftree:malloc error",NULL);
	 free(new_path);
         free(label); 
	 return;
  }
  sprintf(fullpath,"%s%s",new_path,entry_return);
  if (mkdir (fullpath, 0xFFFF) != -1)
      update_timer (GTK_CTREE (ctree));
  else
      xf_dlg_error (win->top,fullpath, strerror (errno));
  free(new_path); free(fullpath);  free(label);
  return;
}

void
cb_new_file (GtkWidget * item, GtkWidget * ctree)
{
  char *path,*spath, *label, *entry_return, *fullpath;
  struct stat st;
  FILE *fp;
  cfg *win;

  win = gtk_object_get_user_data (GTK_OBJECT (ctree));
  spath=valid_path((GtkCTree *)ctree,TRUE);
  if (!spath) return;
  
  
  path=(char *)malloc(2+strlen(spath));
  if (!path) return;
  label=(char *)malloc(2+strlen(_("New_File")));
  if (!label){
	 free(path);
	 return;
  }

  if (spath[strlen (spath) - 1] == '/')  sprintf (path, "%s", spath);
  else    sprintf (path, "%s/", spath);
  strcpy (label, _("New_File"));
  entry_return = (char *)xf_dlg_string (win->top,path, label);
  
  if (!entry_return || !strlen(entry_return) || !io_is_valid (entry_return)){
	free(path); free(label); 
	return;
  }
  fullpath = (char *)malloc(strlen(path)+strlen(entry_return)+2);
  if (!fullpath){
	 xf_dlg_error(win->top,"xftree:malloc error",NULL);
	 free(path);
         free(label); 
	 return;
  }
  

  sprintf (fullpath, "%s%s", path, entry_return);
  if (stat (fullpath, &st) != -1) {
      /*if (dlg_question (_("File exists ! Override ?"), compl) != DLG_RC_OK)*/
      if (xf_dlg_new(win->top,override_txt(fullpath,NULL),
			      _("File exists !"),NULL,DLG_OK|DLG_CANCEL)!= DLG_RC_OK) {
	 free(path);free(fullpath); free(label); 
 	 return;
      }
  }
  fp = fopen (fullpath, "w");
  if (!fp)    {
     xf_dlg_error (win->top,fullpath,strerror(errno));
     free(path);free(fullpath); free(label); 
     return;
  }
  fclose (fp);
  update_timer (GTK_CTREE(ctree));
}

void
cb_rename (GtkWidget * item, GtkCTree * ctree)
{
  entry *en;
  GtkCTreeNode *node;
  char *ofile,*nfile,*p,*entry_return,*label;
  cfg *win;
  struct stat st;

  cursor_wait (GTK_WIDGET (ctree));
  gtk_clist_freeze (GTK_CLIST (ctree));
  win = gtk_object_get_user_data (GTK_OBJECT (ctree));
  if (!count_selection (ctree, &node))
  {
    xf_dlg_warning (win->top,_("No item marked !"));
    goto rename_return;
  }
  en = gtk_ctree_node_get_row_data (GTK_CTREE (ctree), node);

  if (en->type & FT_TARCHILD) {
	xf_dlg_error(win->top,_("This function is not available for the contents of tar files"),NULL);
	goto rename_return;
  }

  if (!io_is_valid (en->label) || (en->type & FT_DIR_UP)) goto rename_return;
  if (strchr (en->label, '/'))  goto rename_return;

  label = (char *)malloc(strlen(en->label)+1);
  if (!label) goto rename_return;
  strcpy(label,en->label);
  entry_return = (char *)xf_dlg_string (win->top,_("Rename to : "),label);
  
  if (!entry_return || !strlen(entry_return) || !io_is_valid (entry_return)){
	goto rename_return;
  }

  if ((p = strchr (entry_return, '/')) != NULL) {
      p[1] = '\0';
      xf_dlg_error (win->top,_("Character not allowed in filename"), p);
      goto rename_return;
  }

  ofile = (char *)malloc(strlen(en->path)+1);
  if (!ofile){
	  goto rename_return;
  }	  
  strcpy(ofile,en->path);
  
  nfile = (char *)malloc(strlen(en->path)+strlen(entry_return)+1);
  if (!nfile) {
	  free(ofile);
	  goto rename_return;
  }
  strcpy (nfile,ofile);
  p=strrchr(nfile,'/');
  p[1]=0;
  strcat(nfile,entry_return);

  /*fprintf(stderr,"dbg: rename %s->%s\n",ofile,nfile);*/

  if (lstat (nfile, &st) != ERROR)  {
      if (xf_dlg_new(win->top,override_txt(nfile,NULL),_("File exists !"),NULL,DLG_OK|DLG_CANCEL)!= DLG_RC_OK)
      {
	free(ofile); free(nfile);
	goto rename_return;
      }
  }
  if (rename (ofile, nfile) == -1)  {
      xf_dlg_error (win->top,nfile, strerror (errno));
      free(ofile); free(nfile);
      goto rename_return;
  }
  update_timer (GTK_CTREE(ctree));
  free(ofile); free(nfile);
rename_return:
  gtk_clist_thaw (GTK_CLIST (ctree));
  cursor_reset (GTK_WIDGET (ctree)); 
  return;
  
}

/*
 * call the dialog "open with"
 */
void
cb_open_with (GtkWidget * item, GtkCTree * ctree)
{
  entry *en;
  cfg *win;
  GtkCTreeNode *node;
  char *prg;

  win = gtk_object_get_user_data (GTK_OBJECT (ctree));
  if (!count_selection (ctree, &node))
  {
    xf_dlg_warning (win->top,_("No files marked !"));
    return;
  }
  en = gtk_ctree_node_get_row_data (GTK_CTREE (ctree), node);
  prg = reg_app_by_file (win->reg, en->path);
  xf_dlg_open_with ((GtkWidget *)ctree,win->xap, prg ? prg : DEF_APP, en->path);
}

/*
 * call the dialog "properties"
 */
void
cb_props (GtkWidget * item, GtkCTree * ctree)
{
  entry *en;
  GtkCTreeNode *node;
  fprop oprop, nprop;
  GList *selection;
  struct stat fst;
  int rc = DLG_RC_CANCEL, ask = 1, flags = 0;
  int first_is_stale_link = 0;
  cfg *win;

  win = gtk_object_get_user_data (GTK_OBJECT (ctree));
  ctree_freeze (ctree);
  selection = g_list_copy (GTK_CLIST (ctree)->selection);
  if (!selection) {
	  xf_dlg_error(win->top,_("Nothing selected !"),NULL);
	ctree_thaw (ctree);
	  return;
  }

  while (selection)
  {
    node = selection->data;
    en = gtk_ctree_node_get_row_data (GTK_CTREE (ctree), node);
    if (!io_is_valid (en->label))
    {
      selection = selection->next;
      continue;
    }
    if (en->type & FT_TARCHILD) {
	xf_dlg_error(win->top,_("This function is not available for the contents of tar files"),NULL);
	g_list_free (selection);
	ctree_thaw (ctree);
	return;
    }

    if (selection->next)
      flags |= IS_MULTI;
    if (lstat (en->path, &fst) == -1)
    {
      if (xf_dlg_continue (win->top,en->path, strerror (errno)) != DLG_RC_OK)
      {
	g_list_free (selection);
	ctree_thaw (ctree);
	return;
      }
      selection = selection->next;
      continue;
    }
    else
    {
      if (S_ISLNK (fst.st_mode))
      {
	if (stat (en->path, &fst) == -1)
	{
	  flags |= IS_STALE_LINK;
	  if (ask)
	  {
	    /* if the first is a stale link we can not
	     * change mode for all other if the user
	     * presses "all", cause it would result in
	     * rwxrwxrwx :-(
	     */
	    first_is_stale_link = 1;
	  }
	}
      }
      oprop.mode = fst.st_mode;
      oprop.uid = fst.st_uid;
      oprop.gid = fst.st_gid;
      oprop.ctime = fst.st_ctime;
      oprop.mtime = fst.st_mtime;
      oprop.atime = fst.st_atime;
      oprop.size = fst.st_size;
      if (ask)
      {
	nprop.mode = oprop.mode;
	nprop.uid = oprop.uid;
	nprop.gid = oprop.gid;
	nprop.ctime = oprop.ctime;
	nprop.mtime = oprop.mtime;
	nprop.atime = oprop.atime;
	nprop.size = oprop.size;
	rc = xf_dlg_prop ((GtkWidget *)ctree,en->path, &nprop, flags);
      }
      switch (rc)
      {
      case DLG_RC_OK:
      case DLG_RC_ALL:
	if (io_is_valid (en->label))
	{
	  if ((oprop.mode != nprop.mode) && (!(flags & IS_STALE_LINK)) && (!first_is_stale_link))
	  {
	    /* chmod() on a symlink itself isn't possible */
	    if (chmod (en->path, nprop.mode) == -1)
	    {
	      if (xf_dlg_continue (win->top,en->path, strerror (errno)) != DLG_RC_OK)
	      {
		g_list_free (selection);
		ctree_thaw (ctree);
		return;
	      }
	      selection = selection->next;
	      continue;
	    }
	  }
	  if ((oprop.uid != nprop.uid) || (oprop.gid != nprop.gid))
	  {
	    if (chown (en->path, nprop.uid, nprop.gid) == -1)
	    {
	      if (xf_dlg_continue (win->top,en->path, strerror (errno)) != DLG_RC_OK)
	      {
		g_list_free (selection);
		ctree_thaw (ctree);
		return;
	      }
	      selection = selection->next;
	      continue;
	    }
	  }
	  if (rc == DLG_RC_ALL)
	    ask = 0;
	  if (ask)
	    first_is_stale_link = 0;
	}
	break;
      case DLG_RC_SKIP:
	selection = selection->next;
	continue;
	break;
      default:
	ctree_thaw (ctree);
	g_list_free (selection);
	return;
	break;
      }
    }
    selection = selection->next;
  }
  g_list_free (selection);
  ctree_thaw (ctree);
}


/*
 */
void
on_destroy (GtkWidget * top,  GtkCTree * ctree)
{
  cfg * win;
  win = gtk_object_get_user_data (GTK_OBJECT (ctree));
  geometryX = top->allocation.width;
  geometryY = top->allocation.height;
  save_defaults(win->top);
  top_delete (top);
  if (win->timer)
  {
    gtk_timeout_remove (win->timer);
  }
  g_free (win->trash);
  g_free (win->xap);
  g_free (win);
  if (!top_has_more ())
  {
    gtk_main_quit ();
  }
}

void
cb_destroy (GtkWidget * widget, GtkCTree * ctree)
{
  cfg *win;
  GtkWidget *root;
  win = gtk_object_get_user_data (GTK_OBJECT (ctree));
  root = win->top;
  geometryX = root->allocation.width;
  geometryY = root->allocation.height;
  save_defaults(NULL);
  /* free history list (avoid memory leaks)*/
  while (win->gogo){
	  golist *previous;
	  previous=win->gogo->previous;
	  if (win->gogo->path) free (win->gogo->path);
	  free(win->gogo);
	  win->gogo=previous;
  }
  gtk_widget_destroy (root);
}
void 
cb_quit (GtkWidget * top,  GtkCTree * ctree)
{
  cfg *win;
  GtkWidget *root;
  win = gtk_object_get_user_data (GTK_OBJECT (ctree));
  root = win->top;
  geometryX = root->allocation.width;
  geometryY = root->allocation.height;
  save_defaults(NULL);
  gtk_main_quit();
}
	

void
cb_term (GtkWidget * item, GtkWidget * ctree)
{
  char *path,*cmd;

  path=valid_path((GtkCTree *)ctree,FALSE);
  if (!path) return;
  cmd=(char *)malloc(strlen(path)+13);
  if (!cmd) return;
  sprintf (cmd, "xfterm \"%s\" &", path);
  io_system (cmd);
  free(cmd);
}

void
cb_exec (GtkWidget * top,GtkWidget * ctree)
{
  cfg *win;
  win = gtk_object_get_user_data (GTK_OBJECT (ctree));
  xf_dlg_execute (ctree,win->xap, NULL);
}

void
cb_samba (GtkWidget * top,GtkWidget * ctree)
{
  io_system ("xfsamba&");  
}

extern GtkWidget *autotype_C;
extern autotype_t autotype[];

void
cb_autotype (GtkWidget * top,GtkWidget * ctree)
{
  FILE *pipe;
  GtkCTreeNode *node;
  int num;
  entry *en;
  char *loc,*cmd,*path;
  int i=0;
  reg_t *prg;
  cfg *win;
  
  num = count_selection ((GtkCTree *)ctree, &node);
  /*printf("dbg: num=%d\n",num);*/
  if (!num) return;
       
  ctree_freeze ((GtkCTree *)ctree);
  win = gtk_object_get_user_data (GTK_OBJECT (ctree));
  en = gtk_ctree_node_get_row_data ((GtkCTree *)ctree, node);
  prg = reg_prog_by_file (win->reg, en->path);
  if (prg) {
        char cmd[(PATH_MAX + 3) * 2];
	if (prg->arg)
	  sprintf (cmd, "\"%s\" %s \"%s\" &", prg->app, prg->arg, en->path);
	else
	  sprintf (cmd, "\"%s\" \"%s\" &", prg->app, en->path);
	io_system (cmd);
	goto end_autotype;
  }     
  
  loc=strrchr(en->path,'.');
  if (loc) for (i=0;1;i++){
       /*printf("dbg: autotype=%s,%s\n",autotype[i].extension,autotype[i].command);*/
       if (autotype[i].extension==NULL) break;
       if (strcmp(loc,autotype[i].extension)==0) break;
  }
  if (autotype[i].command==NULL) goto end_autotype;
  path=valid_path((GtkCTree *)ctree,FALSE);
  chdir(path);
  cmd = (char *) malloc(strlen(autotype[i].command)+
		  strlen(en->path)+
		  strlen(";echo \"DONE\"")+
		  5);
  if (!cmd) goto end_autotype;
  sprintf (cmd, "%s \"%s\";echo \"DONE\"",autotype[i].command,en->path);
  /*printf("dbg: cmd=%s\n",cmd);*/
  pipe = popen (cmd, "r");
  free(cmd);
  if (pipe){
    char line[32];
    while (1) { 
     fgets (line, 31, pipe);
     line[31]=0;
     if (strstr(line,"DONE")) break;
     while (gtk_events_pending()) gtk_main_iteration();
    }		     
  }
end_autotype:
  ctree_thaw ((GtkCTree *)ctree);
  update_timer ((GtkCTree *)ctree);
}


void
cb_autotar (GtkWidget * top,GtkWidget * ctree)
{
  FILE *pipe;
  GtkCTreeNode *node;
  int num;
  entry *en;
  char *cmd;
  
  num = count_selection ((GtkCTree *)ctree, &node);
  /*printf("dbg: num=%d\n",num);*/
  if (!num) return;
  ctree_freeze ((GtkCTree *)ctree);
  en = gtk_ctree_node_get_row_data ((GtkCTree *)ctree, node);

  chdir(valid_path((GtkCTree *)ctree,FALSE));
  chdir("../");
  cmd = (char *) malloc( 2*strlen(en->path)+
		  strlen("tar -czf \"%%.tgz\" \"%%\";echo \"DONE\";")+
		  5);
  if (!cmd) goto end_autotar;
  /* check for overwrite */
  {
    glob_t dirlist;
    sprintf(cmd,"%s.tgz",en->path);
    if(glob (cmd, GLOB_ERR, NULL, &dirlist)==0){
            cfg *win;
	    win = gtk_object_get_user_data (GTK_OBJECT (ctree));
	    if (xf_dlg_question(win->top,_("Override?"),cmd)!=DLG_RC_OK) 
		    goto end_autotar;
	    else unlink(cmd);
    }
    globfree(&dirlist);
  }

  
  sprintf (cmd, "tar -czf \"%s.tgz\" \"%s\";echo \"DONE\"",en->path,en->label);
  /*printf("dbg: cmd=%s\n",cmd);*/
  pipe = popen (cmd, "r");
  free(cmd);
  if (pipe){
    char line[32];
    while (1) { 
     fgets (line, 31, pipe);
     line[31]=0;
     if (strstr(line,"DONE")) break;
     while (gtk_events_pending()) gtk_main_iteration();
    }		     
  }
end_autotar:
  ctree_thaw ((GtkCTree *)ctree);
  update_timer ((GtkCTree *)ctree);
}



