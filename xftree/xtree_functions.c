/*
 * xtree_functions.c: general gui functions.
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
#include <glob.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkenums.h>
#include "constant.h"
#include "my_intl.h"
#include "my_string.h"
#include "xpmext.h"
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
#include "xtree_mess.h"
#include "xtree_pasteboard.h"
#include "xtree_go.h"
#include "xtree_cb.h"
#include "xtree_tar.h"
#include "xtree_toolbar.h"
#include "xtree_functions.h"
#include "xtree_icons.h"

#ifdef HAVE_GDK_PIXBUF
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif


#ifdef HAVE_GDK_IMLIB
#include <gdk_imlib.h>
#endif

#ifndef HAVE_SNPRINTF
#  include "snprintf.h"
#endif

#ifdef DMALLOC
#  include "dmalloc.h"
#endif

#ifndef GLOB_TILDE
#define GLOB_TILDE 0
#endif

static void update_status(GtkCTreeNode * node,GtkCTree * ctree);
static int update_tree (GtkCTree * ctree, GtkCTreeNode * node);
/*
 * check if a node is a directory and is visible and expanded
 * will be called for every node
 */
static void
get_visible_or_parent (GtkCTree * ctree, GtkCTreeNode * node, gpointer data)
{
  GtkCTreeNode *child;
  GList **list = (GList **) data;

  if (GTK_CTREE_ROW (node)->is_leaf)
    return;

  if (gtk_ctree_node_is_visible (ctree, node) && GTK_CTREE_ROW (node)->expanded)
  {
    /* we can't remove a node or something else here,
     * that would break the calling gtk_ctree_pre_recursive()
     * so we remember the node in a linked list
     */
    *list = g_list_append (*list, node);
    return;
  }

  /* check if at least one child is visible
   */
  child = GTK_CTREE_ROW (node)->children;
  while (child)
  {
    if (gtk_ctree_node_is_visible (GTK_CTREE (ctree), child))
    {
      *list = g_list_append (*list, node);
      return;
    }
    child = GTK_CTREE_ROW (child)->sibling;
  }
}

/*
 * timer function to update the view
 */
gint update_timer (GtkCTree * ctree)
{
  GList *list = NULL, *tmp;
  GtkCTreeNode *node,*status_node;
  gboolean manage_timeout;
  cfg *win;

  //return (TRUE);
  win = gtk_object_get_user_data (GTK_OBJECT (ctree));
  status_node=win->status_node;

  /* has root directory vanished ? */
  {
    entry *en;
    glob_t dirlist;
    GtkCTreeNode *root;
    root = GTK_CTREE_NODE (GTK_CLIST (ctree)->row_list);
    en = gtk_ctree_node_get_row_data (GTK_CTREE (ctree),root); 
    if (glob (en->path, GLOB_ERR, NULL, &dirlist) != 0){
	    fprintf(stderr,"xftree: xftree root has vanished. Now going home.\n");
	    globfree(&dirlist);
	    /* clean history (what else has vanished?)*/
	    while (win->gogo) win->gogo=popgo(win->gogo);
	    /* go home */
	    cb_go_home (NULL, ctree);
	    return (TRUE);
    } else globfree(&dirlist);
  }

  manage_timeout = (win->timer != 0);
  if (manage_timeout)
  {
    gtk_timeout_remove (win->timer);
    win->timer = 0;
  }


  /* get a list of directories we have to check
   */
  gtk_ctree_post_recursive (ctree, 
		  GTK_CTREE_NODE (GTK_CLIST (ctree)->row_list), 
		  get_visible_or_parent, &list);

  gtk_clist_freeze (GTK_CLIST (ctree));
  tmp = list;
  while (tmp)
  {
    node = tmp->data;
    if (update_tree (ctree, node) != TRUE) break;
    tmp = tmp->next;
  }
  g_list_free (list);
  if (manage_timeout)
  {
    win->timer = gtk_timeout_add (TIMERVAL, (GtkFunction) update_timer, ctree);
  }
  update_status(status_node,ctree);
  gtk_clist_thaw (GTK_CLIST (ctree));
  
  return (TRUE);
}


/*
 * count the number of  selected nodes, if there are no nodes selected
 * in "first" the root node is returned
 */

int
count_selection (GtkCTree * ctree, GtkCTreeNode ** first)
{
  int num = 0;
  GList *list;

  *first = GTK_CTREE_NODE (GTK_CLIST (ctree)->row_list);

  list = GTK_CLIST (ctree)->selection;
  num = g_list_length (list);
  if (num <= 0)
  {
    return (0);
  }
  *first = GTK_CTREE_NODE (GTK_CLIST (ctree)->selection->data);
  return (num);
}


/*
 * called if a node will be destroyed
 * so free all private data
 */
void
node_destroy (gpointer p)
{
  entry *en = (entry *) p;
  entry_free (en);
}


int
selection_type (GtkCTree * ctree, GtkCTreeNode ** first)
{
  int num = 0;
  GList *list;
  GtkCTreeNode *node;
  entry *en;

  list = GTK_CLIST (ctree)->selection;

  *first = GTK_CTREE_NODE (GTK_CLIST (ctree)->row_list);
  if (g_list_length (list) <= 0)
    return (0);

  while (list)
  {
    node = list->data;
    en = gtk_ctree_node_get_row_data (GTK_CTREE (ctree), node);
    if ((en->type & FT_DIR) || (en->type & FT_DIR_UP) || (en->type & FT_DIR_PD))
      num |= MN_DIR;
    else
      num |= MN_FILE;
    list = list->next;
  }

  *first = GTK_CTREE_NODE (GTK_CLIST (ctree)->selection->data);
  return (num);
}




/*
 * what should we do here?
 */
void menu_detach (void)
{
  /*printf (_("dbg:menu_detach()\n"));*/
}

void
tree_unselect  (GtkCTree *ctree,GList *node,gpointer user_data)
{
  gtk_ctree_unselect_recursive (ctree, NULL);
}

void
ctree_freeze (GtkCTree * ctree)
{
  cursor_wait (GTK_WIDGET (ctree));
  gtk_clist_freeze (GTK_CLIST (ctree));
}

void
ctree_thaw (GtkCTree * ctree)
{
  gtk_clist_thaw (GTK_CLIST (ctree));
  cursor_reset (GTK_WIDGET (ctree));
}

GtkCTreeNode *
add_node (GtkCTree * ctree, GtkCTreeNode * parent, GtkCTreeNode * sibling, char *label, char *path, int *type, int flags)
{
  cfg *win;
  entry *en;
  GtkCTreeNode *item;
  gchar *text[COLUMNS];
  gchar size[32];
  gchar date[32];
  icon_pix pix;  
  gboolean isleaf;
  struct passwd *pw;
  struct group *gr;

  win = gtk_object_get_user_data (GTK_OBJECT (ctree));
  if (!label || !path)
  {
    return NULL;
  }
  if (*type & FT_DUMMY)
  {
    en = entry_new ();
    en->label = g_strdup (label);
    en->path = g_strdup (path);
    en->type = FT_DIR | FT_DUMMY;
  }
  else
  {
    en = entry_new_by_path_and_label (path, label);
    if (!en)
    {
	   /* fprintf(stderr,"dbg:can't do %s\n",path);*/
      return 0;
    }
    en->flags = flags;
    if (strstr(en->label,".")){
	  char *w;
	  w=strrchr(en->label,'.');
	  if (strcmp(w,".tar")==0) en->type |= (FT_TAR|FT_TAR_DUMMY);
	  if (strcmp(w,".tgz")==0) en->type |= (FT_TAR|FT_TAR_DUMMY|FT_GZ);
	  if (strcmp(w,".gz")==0) {
		  en->type |= (FT_GZ);
		  if (strstr(en->label,".tar.gz")) en->type |= (FT_TAR|FT_TAR_DUMMY);
	  } 
	  if (strcmp(w,".Z")==0) {
		  en->type |= (FT_COMPRESS);
		  if (strstr(en->label,".tar.Z")) en->type |= (FT_TAR|FT_TAR_DUMMY);
	  }
	  if (strcmp(w,".bz2")==0) {
		  en->type |= (FT_BZ2);
		  if (strstr(en->label,".tar.bz2")) en->type |= (FT_TAR|FT_TAR_DUMMY);
	  }
    }
    {
      char *tag="  ";
      unsigned long long tama;
      tama =  en->st.st_size;
      if (tama >= (long long)1024*1024*1024*10) {tama /= (long long)1024*1024*1024; tag=" Gb";}
      else if (tama >= 1024*1024*10) {tama /= 1024*1024; tag=" Mb";}
      else if (tama >= 1024*10) {tama /= 1024; tag=" Kb";}
      sprintf (size, " %llu%s", tama,tag);
    }
  }
  sprintf (date, "%02d-%02d-%02d  %02d:%02d", en->date.year, en->date.month, en->date.day, en->date.hour, en->date.min);
  if (win->preferences&ABREVIATE_PATHS) text[COL_NAME] = abreviateP(en->label); else 
  text[COL_NAME] = en->label;
  text[COL_DATE] = date;
  text[COL_SIZE] = size;
  text[COL_MODE] = mode_txt(en->st.st_mode);
  pw=getpwuid(en->st.st_uid); 
  text[COL_UID] = (pw)? pw->pw_name : _("unknown");
  gr=getgrgid (en->st.st_gid); 
  text[COL_GID] = (gr)? gr->gr_name : _("unknown");

  isleaf=set_icon_pix(&pix,en->type,en->label);
  if (en->type & FT_TAR) isleaf=FALSE;
  
  /*fprintf(stderr,"dbg:%s en_is_tar=%d\n",en->label,en->type & FT_TAR);*/
/**************************/

  item = gtk_ctree_insert_node (ctree, parent, NULL, text, SPACING, 
		  pix.pixmap,pix.pixmask, pix.open,pix.openmask, isleaf, FALSE);
  if (item){
    gtk_ctree_node_set_row_data_full (ctree, item, en, node_destroy);
    if (en->type & FT_TAR) {
	    add_tar_dummy(ctree, item,en);
    }
/*    if (en->type & FT_TAR) add_tar_tree(ctree, item,en);*/
  }
  *type = en->type;
  return (item);
}

void
update_node (GtkCTree * ctree, GtkCTreeNode * node, int type, char *label)
{
  icon_pix pix;  
  gboolean isleaf;
  entry *en;
  
  if (!ctree || !node || !label)
  {
    return;
  }
  en = gtk_ctree_node_get_row_data (ctree, node); 
  isleaf=set_icon_pix(&pix,en->type,en->label);
  gtk_ctree_set_node_info (ctree, node, label, SPACING, 
		  pix.pixmap,pix.pixmask, pix.open, pix.openmask, isleaf, FALSE);
  
}

static void update_status(GtkCTreeNode * node,GtkCTree * ctree){
   GtkCTreeNode *root,*child;
   entry *en,*p_en;
   status_info status_inf;
   cfg * win;
   char *texto;
   
   win = gtk_object_get_user_data (GTK_OBJECT (ctree));
   status_inf.howmany=0;
   status_inf.howmuch=0;
   /* is the status node still there? */
   root = GTK_CTREE_NODE (GTK_CLIST (ctree)->row_list);
   if (!root) printf("dbg: root is null!!!\n");
   if (!gtk_ctree_find (ctree,root,node)){
	   /*fprintf(stderr,"dbg:status node gone away.\n");*/
	   node=root;
	   win->status_node=root;
   }
 
   p_en = gtk_ctree_node_get_row_data (ctree, node); 
   
  for (child=GTK_CTREE_ROW (node)->children;child!=NULL;child = GTK_CTREE_ROW (child)->sibling){
      en = gtk_ctree_node_get_row_data (ctree, child); 
      status_inf.howmany++;
      status_inf.howmuch += en->st.st_size;
	  /* fprintf(stderr,"dbg:%s=%lld total=%lld\n",en->path,(long long)en->st.st_size,status_inf.howmuch);*/
  }
   /*
    * status_inf.howmuch *= 0.0009765625;
    * This not worth doing.
    * Firstly the compiler already knows about multiply/divide tricks. Secondly
    * floating point is fantastically more expensive on many platforms. Also
    * for  x / 1024 gcc will generate x >> 10 anyway providing en is unsigned. If
    * its signed you want to force it explicitly.
    * 
    * The compiler is your friend. */
   
   status_inf.howmuch = ((unsigned long long)(status_inf.howmuch));
   texto=(char *)malloc(128+strlen(p_en->path));
   if (!texto) return;
   {
       char *tag="";
       unsigned long long tama;
       tama =  (unsigned long long)status_inf.howmuch;
       if (tama >= (long long)1024*1024*1024*10) {tama /= (long long)1024*1024*1024; tag="G";}
       else if (tama >= 1024*1024*10) {tama /= 1024*1024; tag="M";}
       else if (tama >= 1024*10) {tama /= 1024; tag="K";}
       sprintf(texto,"%s: %lld %s, %lld %sbytes.",p_en->path, 
		   (long long)status_inf.howmany,
		   _("files"),tama,tag);
  }
   gtk_label_set_text ((GtkLabel *)win->status,texto);

   /*fprintf(stderr,"dbg:%s\n",texto);*/
   free(texto);
   win->status_node=node;
	
}


void add_subtree (GtkCTree * ctree, GtkCTreeNode * root, char *path, int depth, int flags)
{
  unsigned int count=0;
  cfg * win;
  xf_dirent *diren;
  GtkCTreeNode *item = NULL, *first = NULL;
  char *base;
  char *complete;
  char *label;
  int add_slash = no, len, d_len;
  int type = 0;
  win = gtk_object_get_user_data (GTK_OBJECT (ctree));

  if (depth == 0) return ;

  if (!path) return ;
  len = strlen (path);
  if (!len)  return ;
  if (path[len - 1] != '/')
  {
    add_slash = yes;
    len++;
  }
  base = g_malloc (len + 1);
  if (!base){
    xf_dlg_error(win->top,_("internal malloc error:"),strerror(errno));
    exit(1);
  }
  strcpy (base, path);
  if (add_slash)
    strcat (base, "/");

      /* fprintf(stderr,"dbg:base=%s\n",base);*/
  if (depth == 1)
  {
    /* create dummy entry */
    if ((complete=(char *)malloc(strlen(base)+2))==NULL) return;
    sprintf (complete, "%s.", base);
    type = FT_DUMMY;
    add_node (GTK_CTREE (ctree), root, NULL, ".", complete, &type, flags);
    g_free (base);
    free(complete);
    return ;
  }
  {
   char *name;
   /* ../ is usually not filtered in, so add it */
   if ((win->preferences&FILTER_OPTION)&&
        ((win->filterOpts & FILTER_DIRS) || (win->filterOpts & FILTER_FILES))){ /* filtering */
     type=FT_DIR_UP;
     if ((complete=(char *)malloc(strlen(base)+3))==NULL) return;
     sprintf (complete, "%s..", base);
     add_node (GTK_CTREE (ctree), root, NULL, "..",complete , &type, flags);
     free(complete);
   }
   diren = xf_opendir (path,(GtkWidget *)ctree);
   if (!diren) {
       /*fprintf(stderr,"dbg:add_subtree() xf_opendir failed\n");*/
    g_free (base);
    return ;
   }
   while ((name = xf_readdir (diren,(GtkWidget *)ctree)) != NULL) {
    type = 0;
    item = NULL;
    d_len = strlen (name);
	/*fprintf(stderr,"dbg:%s\n",name);*/
    if (io_is_dirup (name)) type |= FT_DIR_UP | FT_DIR;
    else if ((d_len >= 1) && io_is_hidden (name) && ((flags & IGNORE_HIDDEN)))
      continue;
    if ((complete=(char *)malloc(strlen(base)+strlen(name)+1))==NULL) continue;
    sprintf (complete, "%s%s", base, name);
    if ((label=(char *)malloc(strlen(name)+1))==NULL) continue;
    strcpy (label, name);
    if ((!io_is_current (name))){
      entry *en;
      item = add_node (GTK_CTREE (ctree), root, first, label, complete, &type, flags);
      en = gtk_ctree_node_get_row_data (ctree, item); 
    }
    if ((++count % 512)==0){
        gtk_clist_thaw (GTK_CLIST (ctree));
        while (gtk_events_pending()) gtk_main_iteration();	     
        gtk_clist_freeze (GTK_CLIST (ctree));
    }
    if ((type & FT_DIR) && (!(type & FT_DIR_UP)) && (!(type & FT_DIR_PD)) && (io_is_valid (name)) && item){
	    /* this is just to get the necesary expanders on startup */
      add_subtree (ctree, item, complete, depth - 1, flags);
    }
    else {if (!first) {first = item;}}
    free(complete);
    free(label);
   }
   diren=xf_closedir (diren);
   update_status(root,ctree);
  } 
  g_free (base);
  /* do not autosort directories with more than 1024 entries */
  if (count <= 1024)gtk_ctree_sort_node (ctree, root);
  return ;
}

/*
 */
void
on_dotfiles (GtkWidget * item, GtkCTree * ctree)
{
  GtkCTreeNode *node, *child;
  entry *en;

  gtk_clist_freeze (GTK_CLIST (ctree));

  /* use first selection if available
   */
  count_selection (ctree, &node);
  en = gtk_ctree_node_get_row_data (ctree, node);
  if (!(en->type & FT_DIR))
  {
    /* select parent node */
    node = GTK_CTREE_ROW (node)->parent;
    en = gtk_ctree_node_get_row_data (ctree, node);
  }
  /* Ignore toggle on parent directory */
  if (en->type & FT_DIR_UP)
  {
    gtk_clist_thaw (GTK_CLIST (ctree));
    return;
  }
  child = GTK_CTREE_ROW (node)->children;
  while (child)
  {
    gtk_ctree_remove_node (ctree, child);
    child = GTK_CTREE_ROW (node)->children;
  }
  en->flags ^= IGNORE_HIDDEN;
  add_subtree (ctree, node, en->path, 2, en->flags);
  gtk_ctree_expand (ctree, node);
  gtk_clist_thaw (GTK_CLIST (ctree));
}


/*
 */
void
on_expand (GtkCTree * ctree, GtkCTreeNode * node, char *path)
{
  GtkCTreeNode *child;
  cfg *win;
  entry *en;

  en = gtk_ctree_node_get_row_data (ctree, node);
  if (en->type & (FT_TAR|FT_TARCHILD)) {
      if  (en->type & FT_TAR_DUMMY){
	      /*printf("doingit\n");*/
	      /* cursor wait */
              ctree_freeze (ctree);
	      /* remove dummy */
              child = GTK_CTREE_ROW (node)->children;
              while (child)  {
                gtk_ctree_remove_node (ctree, child);
                child = GTK_CTREE_ROW (node)->children;
              }
	      en->type ^= FT_TAR_DUMMY;
	      /* add tar tree */
              add_tar_tree(ctree,node,en);
	      /* cursor_reset */
              ctree_thaw (ctree);
      } 
      gtk_ctree_sort_node (ctree, node);
      return;
  }

  
  
  win = gtk_object_get_user_data (GTK_OBJECT (ctree));
  ctree_freeze (ctree);
  child = GTK_CTREE_ROW (node)->children;
  while (child)
  {
    gtk_ctree_remove_node (ctree, child);
    child = GTK_CTREE_ROW (node)->children;
  }
  en = gtk_ctree_node_get_row_data (ctree, node);
  add_subtree (ctree, node, en->path, 2, en->flags);
  ctree_thaw (ctree);
  if (win->preferences & STATUS_FOLLOWS_EXPAND){
   cfg *win;
   win = gtk_object_get_user_data (GTK_OBJECT (ctree));
   set_title_ctree((GtkWidget *)ctree,en->path);
  }
}

/*
 * unmark all marked on collapsion
 */
void
on_collapse (GtkCTree * ctree, GtkCTreeNode * node, char *path)
{
  cfg *win;
  entry *en;
  GtkCTreeNode *child,*parent;
  
  /* unselect all children */
  win = gtk_object_get_user_data (GTK_OBJECT (ctree));
  child = GTK_CTREE_NODE (GTK_CTREE_ROW (node)->children);
  if (node==GTK_CTREE_NODE (GTK_CLIST (ctree)->row_list)) parent=node;
  else parent = GTK_CTREE_NODE (GTK_CTREE_ROW (node)->parent);
  while (child)  {
    gtk_ctree_unselect (ctree, child);
    child = GTK_CTREE_ROW (child)->sibling;
  }
  
  en = gtk_ctree_node_get_row_data (ctree, parent);
  if (en->type & (FT_TAR|FT_TARCHILD)) return;
  if (win->preferences & STATUS_FOLLOWS_EXPAND){
   set_title_ctree((GtkWidget *)ctree,en->path);
  }
  update_status(parent,ctree);
}


void
set_title (GtkWidget * w, const char *path)
{
  char *title;
  title = (char *)malloc((strlen("XFTree: ")+strlen(path)+1)*sizeof(char));
  if (!title) return; 
  sprintf (title, "XFTree: %s", path);
  gtk_window_set_title (GTK_WINDOW (gtk_widget_get_toplevel (w)), title);
  free(title);

}
void
set_title_ctree (GtkWidget * ctree, const char *path)
{
  char *title;
  cfg *win;
  win = gtk_object_get_user_data (GTK_OBJECT (ctree));
  title = (char *)malloc((strlen("XFTree: ")+strlen(path)+1)*sizeof(char));
  if (!title) return; 
  if (win->preferences&SHORT_TITLES) {
	  char *word;
	  word = strrchr (path,'/');
	  if (word) sprintf(title,"%s",(word[1]==0)?word:word+1);
	  else  sprintf(title,"XFTree");	  
  }
  else sprintf (title, "%s", path);
  gtk_window_set_title (GTK_WINDOW (gtk_widget_get_toplevel (win->top)), title);
  free(title);

}


/*
 */
static int
node_has_child (GtkCTree * ctree, GtkCTreeNode * node, char *label)
{
  GtkCTreeNode *child;
  entry *en;

  child = GTK_CTREE_ROW (node)->children;
  while (child)
  {
    en = gtk_ctree_node_get_row_data (GTK_CTREE (ctree), child);
       /*fprintf(stderr,"dbg:nhc() %s <--> %s\n",en->label, label);fflush(NULL);*/
    if (strcmp (en->label, label) == 0)
    {
      return (1);
    }
    child = GTK_CTREE_ROW (child)->sibling;
  }
  return (0);
}

/*
 * update a node and its childs if visible
 * return 1 if some nodes have removed or added, else 0
 *
 */
typedef struct deletenode_t {
	struct deletenode_t *next;
	GtkCTreeNode *node;
}deletenode_t;

int
update_tree (GtkCTree * ctree, GtkCTreeNode * node)
{
  GtkCTreeNode *child = NULL, *new_child = NULL;
  entry *en, *child_en;
  char compl[PATH_MAX + 1];
  int type, p_len, changed, tree_updated, root_changed;
  gchar size[32];
  gchar date[32];
  cfg *win;
  gboolean manage_timeout;
  deletenode_t *head_delete=NULL,*p_del=NULL,*l_del=NULL;

  if ((!ctree)||(!node)) return 0;

  win = gtk_object_get_user_data (GTK_OBJECT (ctree));
  /* simple case, global preference changed in another window */
  if ((win->preferences&FONT_STATE) != (preferences&FONT_STATE)) {
	/*fprintf(stderr,"dbg: global prefs changed!\n");*/
	if (preferences&FONT_STATE)win->preferences |= FONT_STATE;
        else win->preferences &= (0xffffffff ^ FONT_STATE);	
        set_fontT((GtkWidget *)ctree);
	return 0;
  }
  
  manage_timeout = (win->timer != 0);

  if (manage_timeout)
  {
    gtk_timeout_remove (win->timer);
    win->timer = 0;
  }

  tree_updated = FALSE;
  en = gtk_ctree_node_get_row_data (GTK_CTREE (ctree), node);
  /*fprintf(stderr,"dbg: testing update for %s\n",en->label);*/
  if ((root_changed = entry_update (en)) != 0)
  {
    if (root_changed < 0) goto done_update_timer;
    /*printf("dbg: root changed\n",en->path);*/
  }
  
  
  if (!(en->type&(FT_TAR|FT_TARCHILD)))
	  for (child=GTK_CTREE_ROW (node)->children;child != NULL;child = GTK_CTREE_ROW (child)->sibling)
  {
	  
    child_en = gtk_ctree_node_get_row_data (GTK_CTREE (ctree), child);
     /*fprintf(stderr,"dbg: testing update for %s\n",child_en->label);*/
    if ((changed = entry_update (child_en)) < 0)
    { /* node is gone */
     /*fprintf(stderr,"dbg: node is gone: %s\n",child_en->label);*/
     /* whoa! if you remove this node, the loop conditionals go beserk! 
      * put it in a linked list, and do the deleting when the loop is done 
      * no need to set tree_updated, since control is maintained with
      * head_delete pointer.*/
     l_del=p_del;
     p_del=(deletenode_t *)malloc(sizeof(deletenode_t));
     if (p_del){
	     p_del->node=child;
	     p_del->next=NULL;
	     if (l_del) l_del->next=p_del;
	     if (!head_delete) head_delete=p_del;
     }
     continue;
    }
    else if (changed == TRUE)
    {
      /* update the labels */
     struct passwd *pw;
     struct group *gr;
     /*fprintf(stderr,"dbg: doing update for %s\n",en->label);*/
      {
       char *tag="  ";
       unsigned long long tama;
       tama =  child_en->st.st_size;
       if (tama >= (long long)1024*1024*1024*10) {tama /= (long long)1024*1024*1024; tag=" Gb";}
       else if (tama >= 1024*1024*10) {tama /= 1024*1024; tag=" Mb";}
       else if (tama >= 1024*10) {tama /= 1024; tag=" Kb";}
       sprintf (size, " %llu%s", tama,tag);
      }
     
      sprintf (date, "%02d-%02d-%02d  %02d:%02d", 
		      child_en->date.year, child_en->date.month, 
		      child_en->date.day, child_en->date.hour, child_en->date.min);
      gtk_ctree_node_set_text (ctree, child, COL_DATE, date);
      gtk_ctree_node_set_text (ctree, child, COL_SIZE, size);
      gtk_ctree_node_set_text (ctree, child, COL_MODE, mode_txt(en->st.st_mode));
      pw=getpwuid(en->st.st_uid); 
      gtk_ctree_node_set_text (ctree, child, COL_UID,(pw)? pw->pw_name : _("unknown") );
      gr=getgrgid (en->st.st_gid); 
      gtk_ctree_node_set_text (ctree, child, COL_GID,(gr)? gr->gr_name : _("unknown") );      
      update_node (ctree, child, child_en->type, child_en->label); /* icon changes */
      /* skip sort if no files added tree_updated = TRUE;*/
    }
    /* this may be a dummy subtree, and need not be updated when node is collapsed
     * when node is expanded, each subnode will be updated individually by
     * the recursive tree function, methinks */
    if (!(GTK_CTREE_ROW (child)->children) && (io_is_valid (child_en->label)) && !(child_en->type & FT_DIR_UP) && !(child_en->type & FT_DIR_PD) && (child_en->type & FT_DIR)){
      add_subtree (GTK_CTREE (ctree), child, child_en->path, 1, child_en->flags);
    }
  } /* end for child */
  
  if (head_delete){ /* remove nodes that have gone away */
	  while (head_delete) {
		  p_del=head_delete;
		  head_delete=head_delete->next;
                  gtk_ctree_remove_node (ctree, p_del->node);
		  free(p_del);
	  }
  }
  

    /*fprintf(stderr,"dbg:(%s)rc=%d,tu=%d, isdir=%d\n",en->path,root_changed,tree_updated,en->type & FT_DIR);fflush(NULL);*/
  /*if ((root_changed || tree_updated) && (en->type & FT_DIR))*/
  if (root_changed && (en->type & FT_DIR) && !(en->type & (FT_TARCHILD|FT_TAR))) {
    /* root changed == maybe new entries */
    /*fprintf(stderr,"dbg:rc=%d,tu=%d\n",root_changed,tree_updated);fflush(NULL);*/
    if (GTK_CTREE_ROW (node)->expanded)
    {
      char *name;	    
      xf_dirent *diren;
      /* may be there are new files */
       /*fprintf(stderr,"dbg:update_tree()...\n");fflush(NULL);*/
       
       diren = xf_opendir (en->path,(GtkWidget *)ctree);
       if (!diren) {
	if (manage_timeout) {
	  win->timer = gtk_timeout_add (TIMERVAL, (GtkFunction) update_timer, ctree);
	}
	return TRUE;
       }
       p_len = strlen (en->path);
       while ((name = xf_readdir (diren,(GtkWidget *)ctree)) != NULL) {
	if (io_is_hidden (name) && (en->flags & IGNORE_HIDDEN))  continue;
	if (io_is_current (name))  continue;
	if (!node_has_child (ctree, node, name))
	{
	  if (io_is_root (name)) sprintf (compl, "%s%s", en->path, name);
	  else   sprintf (compl, "%s/%s", en->path, name);
	  type = 0;
	  new_child = NULL;

	  /*fprintf(stderr,"dbg:adding %s (%s)\n",label,name);fflush(NULL);*/

	  new_child = add_node (ctree, node, NULL, name, compl, &type, en->flags);
	  if ((type & FT_DIR) && (io_is_valid (name)) && !(type & FT_DIR_UP) && !(type & FT_DIR_PD) && new_child)
	    add_subtree (ctree, new_child, compl, 1, en->flags);
	  if (entry_update (en) > 0) update_node (ctree, node, en->type, en->label);
	  /* root is changed: tree_updated = TRUE;*/
	}
	/*else {fprintf(stderr,"dbg:update_tree()...node has child\n");fflush(NULL);}*/
       } /* end while */
       diren = xf_closedir (diren);  
    }
    else if ((GTK_CTREE_ROW (node)->children) && (io_is_valid (en->label)) && !(en->type & FT_DIR_UP) && !(en->type & FT_DIR_PD))
    {
      add_subtree (GTK_CTREE (ctree), node, en->path, 1, en->flags);
      /* icon changes after adding the node? not going to happen: */
      /*if (entry_update (en) > 0) update_node (ctree, node, en->type, en->label); */
    }
  }
  /*if (tree_updated || root_changed) {*/
  if (root_changed) {
      /*fprintf(stderr,"dbg:doing sort\n");fflush(NULL);*/
      gtk_ctree_sort_node (GTK_CTREE (ctree), node);
  }
 
done_update_timer:
  if (manage_timeout)
  {
    win->timer = gtk_timeout_add (TIMERVAL, (GtkFunction) update_timer, ctree);
  }
  /*fprintf(stderr,"dbg:done updating\n");fflush(NULL);*/
  return (TRUE);
}


/*
 * if window manager send delete event
 */
gint on_delete (GtkWidget * w, GdkEvent * event, gpointer data)
{
  return (FALSE);
}


XErrorHandler ErrorHandler (Display * dpy, XErrorEvent * event)
{
  char buf[64];
  if ((event->error_code == BadWindow) || (event->error_code == BadDrawable) || (event->request_code == X_GetGeometry) || (event->request_code == X_ChangeProperty) || (event->request_code == X_SetInputFocus) || (event->request_code == X_ChangeWindowAttributes) || (event->request_code == X_GrabButton) || (event->request_code == X_ChangeWindowAttributes) || (event->request_code == X_InstallColormap))
    return (0);

  XGetErrorText (dpy, event->error_code, buf, 63);
  fprintf (stderr, "xftree: Fatal XLib internal error\n");
  fprintf (stderr, "%s\n", buf);
  fprintf (stderr, "Request %d, Error %d\n", event->request_code, event->error_code);
  exit (1);
  return (0);
}




