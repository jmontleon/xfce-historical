/*
 * xtree_gui.c
 *
 * Copyright (C) 1999 Rasca, Berlin
 * EMail: thron@gmx.de
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
#include "constant.h"
#include "my_intl.h"
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
#include "icons/new_file.xpm"
#include "icons/new_dir.xpm"
#include "icons/new_win.xpm"
#include "icons/appinfo.xpm"
#include "icons/closewin.xpm"
#include "icons/delete.xpm"
#include "icons/dotfile.xpm"
#include "icons/home.xpm"
#include "icons/empty_trash.xpm"
#include "icons/trash.xpm"
#include "icons/go_to.xpm"
#include "icons/go_up.xpm"
#include "icons/page.xpm"
#include "icons/page_lnk.xpm"
#include "icons/dir_close.xpm"
#include "icons/dir_pd.xpm"
#include "icons/dir_close_lnk.xpm"
#include "icons/dir_open.xpm"
#include "icons/dir_open_lnk.xpm"
#include "icons/dir_up.xpm"
#include "icons/exe.xpm"
#include "icons/exe_lnk.xpm"
#include "icons/char_dev.xpm"
#include "icons/fifo.xpm"
#include "icons/socket.xpm"
#include "icons/block_dev.xpm"
#include "icons/stale_lnk.xpm"
#include "icons/xftree_icon.xpm"

#ifdef HAVE_GDK_IMLIB
  #include <gdk_imlib.h>
#endif

enum {
	MN_NONE  = 0,
	MN_DIR   = 1,
	MN_FILE  = 2,
	MN_MIXED = 3,
	MENUS,
};

enum {
	COL_NAME,
	COL_SIZE,
	COL_DATE,
	COLUMNS		/* number of columns */
};

#define SPACING 	5
#define DEF_APP		"netscape"
#define TIMERVAL 	6000
#define MAXBUF		8192

#define WINCFG		1
#define TOPWIN		2

#define yes		1
#define no		0
#define ERROR 		-1

static GtkTargetEntry target_table[] = {
	/* target,			flags,					info	*/
    {"text/uri-list",			0,  				TARGET_URI_LIST 	},
    {"text/plain", 			0,  				TARGET_PLAIN 		},
    {"STRING",     			0,  				TARGET_STRING 		},
};
#define NUM_TARGETS (sizeof(target_table)/sizeof(GtkTargetEntry))

#define ACCEL	2
typedef struct {
	char *label;
	void *func;
	int data;
	gint key;
	guint8 mod;
} menu_entry;



static GdkPixmap
	*gPIX_page,
	*gPIX_page_lnk,
	*gPIX_dir_pd,
	*gPIX_dir_close,
	*gPIX_dir_close_lnk,
	*gPIX_dir_open_lnk,
	*gPIX_dir_open,
	*gPIX_dir_up,
	*gPIX_char_dev,
	*gPIX_fifo,
	*gPIX_socket,
	*gPIX_block_dev,
	*gPIX_exe,
	*gPIX_stale_lnk,
	*gPIX_exe_lnk ;

static GdkBitmap
	*gPIM_page,
	*gPIM_page_lnk,
	*gPIM_dir_pd,
	*gPIM_dir_close,
	*gPIM_dir_close_lnk,
	*gPIM_dir_open_lnk,
	*gPIM_dir_open,
	*gPIM_dir_up ,
	*gPIM_char_dev,
	*gPIM_fifo,
	*gPIM_socket,
	*gPIM_block_dev,
	*gPIM_exe,
	*gPIM_stale_lnk,
	*gPIM_exe_lnk;

static GtkWidget *new_top (char *p, char *x, char *trash, GList *reg,
							int width, int height, int flags);
int move_dir (char *source, char *label, char *target, int trash);

/*
 * gtk init function, must be called first
 */
void
gui_init (int *argc, char ***argv, char *rc)
{
  gtk_set_locale ();
  bindtextdomain(PACKAGE, XFCE_LOCALE_DIR);
  textdomain (PACKAGE);
  gtk_init (argc, argv);
}

/*
 */
#define FATAL 1
#define alloc_error_fatal() alloc_error(__FILE__,__LINE__,FATAL)

static void
alloc_error (char *file, int num, int mode)
{
	fprintf (stderr, _("malloc error in %s at line %d\n"), file, num);
	if (mode == FATAL)
		exit (1);
}

/*
 * my own sort function
 * honor if an entry is a directory or a file
 */
static gint
my_compare (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2)
{
	GtkCTreeRow *row1 = (GtkCTreeRow *) ptr1;
	GtkCTreeRow *row2 = (GtkCTreeRow *) ptr2;
	entry *en1, *en2;
	int type1, type2;

	en1 = row1->row.data;
	en2 = row2->row.data;
	type1 = en1->type & (FT_DIR|FT_FILE);
	type2 = en2->type & (FT_DIR|FT_FILE);
	if (type1 != type2) {
		/* i want to have the directories at the top
		 */
		return (type1 < type2 ? -1 : 1);
	}
	if (clist->sort_column != COL_NAME) {
		/* use default compare function which we have saved before
		 */
		GtkCListCompareFunc compare;
		cfg *win;
		win = gtk_object_get_user_data (GTK_OBJECT(clist));
		compare = (GtkCListCompareFunc)win->compare;
		return compare(clist, ptr1, ptr2);
	}
	return strcmp (en1->label, en2->label);
}


/*
 * called if a node will be destroyed
 * so free all private data
 */
void
node_destroy (gpointer p)
{
	entry *en = (entry *)p;
	entry_free (en);
}

/*
 * count the number of  selected nodes, if there are no nodes selected
 * in "first" the root node is returned
 */
int
count_selection (GtkCTree *ctree, GtkCTreeNode **first)
{
	int num =0;
	GList *list;

	*first = GTK_CTREE_NODE(GTK_CLIST(ctree)->row_list);

	list = GTK_CLIST(ctree)->selection;
	num = g_list_length (list);
	if (num <= 0) {
		return (0);
	}
	*first = GTK_CTREE_NODE(GTK_CLIST(ctree)->selection->data);
	return (num);
}


int
selection_type (GtkCTree *ctree, GtkCTreeNode **first)
{
	int num = 0;
	GList *list;
	GtkCTreeNode *node;
	entry *en;

	list = GTK_CLIST(ctree)->selection;

	*first = GTK_CTREE_NODE(GTK_CLIST(ctree)->row_list);
	if (g_list_length (list) <= 0) return (0);

	while (list) 
	  {
		node = list->data;
		en = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), node);
	  	if ((en->type & FT_DIR) || (en->type & FT_DIR_UP) || (en->type & FT_DIR_PD))
		  num |= MN_DIR;
		else
		  num |= MN_FILE;
		list=list->next;
	  }

	*first = GTK_CTREE_NODE(GTK_CLIST(ctree)->selection->data);
	return (num);
}

/*
 */
static gint
on_click_column (GtkCList *clist, gint column, gpointer data)
{
	int num;
	GtkCTreeNode *node;
	GList *selection = NULL;

	if (column != clist->sort_column)
		gtk_clist_set_sort_column (clist, column);
	else {
		if (clist->sort_type == GTK_SORT_ASCENDING)
			clist->sort_type = GTK_SORT_DESCENDING;
		else
			clist->sort_type = GTK_SORT_ASCENDING;
	}
	num = count_selection (GTK_CTREE(clist), &node);
	if (num) {
		for (selection = g_list_copy(GTK_CLIST(clist)->selection);
					selection; selection = selection->next) {
			node = selection->data;
			if (!GTK_CTREE_ROW(node)->children ||
					(!GTK_CTREE_ROW(node)->expanded)) {
				/* select parent node */
				node = GTK_CTREE_ROW(node)->parent;
			}
			gtk_ctree_sort_node (GTK_CTREE(clist), node);
		}
	} else {
		gtk_clist_sort(clist);
	}
	g_list_free(selection);
	return TRUE;
}




/*
 * what should we do here?
 */
void
menu_detach ()
{
	printf (_("menu_detach()\n"));
}

/*
 */
void
cb_open_trash (GtkWidget *item, void *data)
{
	cfg *win = (cfg *)data;
	new_top (win->trash, win->xap, win->trash, win->reg,
				win->width, win->height, 0);
}

/*
 *
 */
void
cb_new_window (GtkWidget *widget, GtkCTree *ctree)
{
	int num;
	GList *selection = NULL;
	GtkCTreeNode *node;
	entry *en = NULL;
	cfg *win;

	win = gtk_object_get_user_data (GTK_OBJECT(ctree));

	num = count_selection (ctree, &node);
	if (num) {
		for (selection = g_list_copy(GTK_CLIST(ctree)->selection);
					selection; selection = selection->next) {
			node = selection->data;
			en = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), node);
			if (!(en->type & FT_DIR)) {
				node = GTK_CTREE_ROW(node)->parent;
				if (!node) {
					continue;
				}
			}
			en = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), node);
			new_top (uri_clear_path(en->path), win->xap, win->trash, win->reg,
						win->width, win->height, en->flags);
		}
	} else {
		en = gtk_ctree_node_get_row_data (ctree, node);
		new_top (uri_clear_path(en->path), win->xap, win->trash, win->reg,
					win->width, win->height, en->flags);
	}
	g_list_free(selection);
}

/*
 */
static void
node_unselect_by_type (GtkCTree *ctree, GtkCTreeNode *node, void *data)
{
	entry *en;

	en = gtk_ctree_node_get_row_data (ctree, node);
	if (en->type & (int)data) {
		gtk_ctree_unselect (ctree, node);
	}
}

/*
 */
static void
cb_select (GtkWidget *item, GtkCTree *ctree)
{
	int num;
	GtkCTreeNode *node;

	num = count_selection (ctree, &node);
	if (!GTK_CTREE_ROW(node)->expanded)
		node = GTK_CTREE_ROW(node)->parent;
	gtk_ctree_select_recursive (ctree, node);
	gtk_ctree_unselect (ctree, node);
	gtk_ctree_pre_recursive (ctree, node, node_unselect_by_type,
			(void *)FT_DIR_UP);
}

/*
 */
void
cb_unselect (GtkWidget *widget, GtkCTree *ctree)
{
	gtk_ctree_unselect_recursive (ctree, NULL);
}



/*
 * popup context menu
 */
static gint
on_button_press (GtkWidget *widget, GdkEventButton *event, void *data)
{
	GtkCTree *ctree = GTK_CTREE(widget);
	GtkWidget **menu = (GtkWidget **)data;
	GtkCTreeNode *node;
	int num, row, column = MN_NONE;

	if (event->button == 3) {
		num = selection_type (ctree, &node);
		if (!num) {
			row = -1;
			gtk_clist_get_selection_info (GTK_CLIST(widget),
				event->x, event->y,
				&row, &column);
			if (row > -1) {
				gtk_clist_select_row (GTK_CLIST(ctree), row, 0);
				if (GTK_CLIST(ctree)->selection) num = selection_type (ctree, &node);
			}
		}
		gtk_menu_popup (GTK_MENU(menu[num]), NULL, NULL, NULL, NULL,
				3, event->time);
		return TRUE;
	}
	return FALSE;
}

static void
ctree_freeze (GtkCTree *ctree)
{
	cursor_wait (GTK_WIDGET(ctree));
	gtk_clist_freeze (GTK_CLIST(ctree));
}

static void
ctree_thaw (GtkCTree *ctree)
{
	gtk_clist_thaw (GTK_CLIST(ctree));
	cursor_reset (GTK_WIDGET(ctree));
}

/*
 */
GtkCTreeNode *
add_node (GtkCTree *ctree, GtkCTreeNode *parent, GtkCTreeNode *sibling,
			char *label, char *path, int *type, int flags)
{
	entry *en;
	GtkCTreeNode *item;
	gchar *text[COLUMNS];
	gchar size[16] = {""};
	gchar date[32] = {""};

	if (!label || !path) {
		return NULL;
	}
	if (*type & FT_DUMMY) {
		en = entry_new ();
		en->label = g_strdup (label);
		en->path  = g_strdup (path);
		en->type = FT_DIR | FT_DUMMY;
	} else {
		en = entry_new_by_path_and_label (path, label);
		if (!en) {
			return 0;
		}
		en->flags = flags;

		sprintf (date, "%02d-%02d-%02d  %02d:%02d",
						en->date.year, en->date.month, en->date.day,
						en->date.hour, en->date.min);
		if (en->size < 0) {
			sprintf (size, "?(ERR %d)", -en->size);
		} else {
			sprintf (size, "%10d", (int)en->size);
		}
	}
	text[COL_NAME] = en->label;
	text[COL_DATE] = date;
	text[COL_SIZE] = size;

	if (en->type & FT_EXE) {
		if (en->type & FT_LINK) {
			item = gtk_ctree_insert_node (ctree, parent, NULL, text, SPACING,
						gPIX_exe_lnk, gPIM_exe_lnk, NULL, NULL, TRUE, FALSE);
		} else {
			item = gtk_ctree_insert_node (ctree, parent, NULL, text, SPACING,
						gPIX_exe, gPIM_exe, NULL, NULL, TRUE, FALSE);
		}
	} else if (en->type & FT_FILE) {
		if (en->type & FT_LINK) {
			item = gtk_ctree_insert_node (ctree, parent, NULL, text, SPACING,
						gPIX_page_lnk, gPIM_page_lnk, NULL, NULL, TRUE, FALSE);
		} else {
			item = gtk_ctree_insert_node (ctree, parent, NULL, text, SPACING,
						gPIX_page, gPIM_page, NULL, NULL, TRUE, FALSE);
		}
	} else if (en->type & FT_DIR_UP) {
		item = gtk_ctree_insert_node (ctree, parent, NULL, text, SPACING,
					gPIX_dir_up, gPIM_dir_up, NULL, NULL, TRUE, FALSE);
	} else if (en->type & FT_DIR_PD) {
		item = gtk_ctree_insert_node (ctree, parent, NULL, text, SPACING,
					gPIX_dir_pd, gPIM_dir_pd,
					gPIX_dir_pd, gPIM_dir_pd, FALSE, FALSE);
	} else if (en->type & FT_DIR) {
		if (en->type & FT_LINK) {
			item = gtk_ctree_insert_node (ctree, parent, sibling, text, SPACING,
						gPIX_dir_close_lnk, gPIM_dir_close_lnk,
						gPIX_dir_open_lnk, gPIM_dir_open_lnk, FALSE, FALSE);
		} else {
			item = gtk_ctree_insert_node (ctree, parent, sibling, text, SPACING,
						gPIX_dir_close, gPIM_dir_close,
						gPIX_dir_open, gPIM_dir_open, FALSE, FALSE);
		}
	} else if (en->type & FT_CHAR_DEV) {
		item = gtk_ctree_insert_node (ctree, parent, NULL, text, SPACING,
					gPIX_char_dev, gPIM_char_dev, NULL, NULL, TRUE, FALSE);
	} else if (en->type & FT_BLOCK_DEV) {
		item = gtk_ctree_insert_node (ctree, parent, NULL, text, SPACING,
					gPIX_block_dev, gPIM_block_dev, NULL, NULL, TRUE, FALSE);
	} else if (en->type & FT_FIFO) {
		item = gtk_ctree_insert_node (ctree, parent, NULL, text, SPACING,
					gPIX_fifo, gPIM_fifo, NULL, NULL, TRUE, FALSE);
	} else if (en->type & FT_SOCKET) {
		item = gtk_ctree_insert_node (ctree, parent, NULL, text, SPACING,
					gPIX_socket, gPIM_socket, NULL, NULL, TRUE, FALSE);
	} else {
		item = gtk_ctree_insert_node (ctree, parent, NULL, text, SPACING,
						gPIX_stale_lnk, gPIM_stale_lnk, NULL,NULL,TRUE, FALSE);
	}
	if (item)
	  gtk_ctree_node_set_row_data_full (ctree, item, en, node_destroy);
	*type = en->type;
	return (item);
}

void
update_node (GtkCTree *ctree, GtkCTreeNode *node, int type, char *label)
{
	if (!ctree || !node || !label) {
		return;
	}

	if (type & FT_EXE) {
		if (type & FT_LINK) {
			gtk_ctree_set_node_info (ctree, node, label, SPACING,
						gPIX_exe_lnk, gPIM_exe_lnk, NULL, NULL, TRUE, FALSE);
		} else {
			gtk_ctree_set_node_info (ctree, node, label, SPACING,
						gPIX_exe, gPIM_exe, NULL, NULL, TRUE, FALSE);
		}
	} else if (type & FT_FILE) {
		if (type & FT_LINK) {
			gtk_ctree_set_node_info (ctree, node, label, SPACING,
						gPIX_page_lnk, gPIM_page_lnk, NULL, NULL, TRUE, FALSE);
		} else {
			gtk_ctree_set_node_info (ctree, node, label, SPACING,
						gPIX_page, gPIM_page, NULL, NULL, TRUE, FALSE);
		}
	} else if (type & FT_DIR_UP) {
		gtk_ctree_set_node_info (ctree, node, label, SPACING,
					gPIX_dir_up, gPIM_dir_up, NULL, NULL, TRUE, FALSE);
	} else if (type & FT_DIR_PD) {
		gtk_ctree_set_node_info (ctree, node, label, SPACING,
					gPIX_dir_pd, gPIM_dir_pd,
					gPIX_dir_pd, gPIM_dir_pd, FALSE, FALSE);
	} else if (type & FT_DIR) {
		if (type & FT_LINK) {
			gtk_ctree_set_node_info (ctree, node, label, SPACING,
						gPIX_dir_close_lnk, gPIM_dir_close_lnk,
						gPIX_dir_open_lnk, gPIM_dir_open_lnk, FALSE, FALSE);
		} else {
			gtk_ctree_set_node_info (ctree, node, label, SPACING,
						gPIX_dir_close, gPIM_dir_close,
						gPIX_dir_open, gPIM_dir_open, FALSE, FALSE);
		}
	} else if (type & FT_CHAR_DEV) {
		gtk_ctree_set_node_info (ctree, node, label, SPACING,
					gPIX_char_dev, gPIM_char_dev, NULL, NULL, TRUE, FALSE);
	} else if (type & FT_BLOCK_DEV) {
		gtk_ctree_set_node_info (ctree, node, label, SPACING,
					gPIX_block_dev, gPIM_block_dev, NULL, NULL, TRUE, FALSE);
	} else if (type & FT_FIFO) {
		gtk_ctree_set_node_info (ctree, node, label, SPACING,
					gPIX_fifo, gPIM_fifo, NULL, NULL, TRUE, FALSE);
	} else if (type & FT_SOCKET) {
		gtk_ctree_set_node_info (ctree, node, label, SPACING,
					gPIX_socket, gPIM_socket, NULL, NULL, TRUE, FALSE);
	} else {
		gtk_ctree_set_node_info (ctree, node, label, SPACING,
						gPIX_stale_lnk, gPIM_stale_lnk, NULL,NULL,TRUE, FALSE);
	}
}

/*
 */
void
add_subtree (GtkCTree *ctree, GtkCTreeNode *root, char *path, int depth,
			int flags)
{
	DIR *dir;
	struct dirent *de;
	GtkCTreeNode *item = NULL, *first= NULL;
	char *base;
	gchar complete[PATH_MAX+NAME_MAX+1];
	gchar label[NAME_MAX+1];
	int add_slash = no, len, d_len;
	int type = 0;

	if (depth == 0)
		return;

	if (!path)
		return;
	len = strlen (path);
	if (!len)
		return;
	if (path[len-1] != '/') {
		add_slash = yes;
		len++;
	}
	base = g_malloc (len+1);
	if (!base)
		alloc_error_fatal();
	strcpy (base, path);
	if (add_slash)
		strcat (base, "/");

	if (depth == 1) {
		/* create dummy entry */
		sprintf (complete, "%s.", base);
		type = FT_DUMMY;
		add_node (GTK_CTREE(ctree), root, NULL, ".", complete, &type, flags);
		g_free (base);
		return;
	}
	dir = opendir (path);
	if (!dir) {
		g_free (base);
		return;
	}
	while ((de = readdir (dir)) != NULL) {
		type = 0;
		item = NULL;
		d_len = strlen (de->d_name);
		if (io_is_dirup(de->d_name))
			type |= FT_DIR_UP | FT_DIR;
		else if ((*de->d_name == '.') && ((flags & IGNORE_HIDDEN) && (d_len >= 1)))
			continue;
		sprintf (complete, "%s%s", base, de->d_name);
		strcpy  (label, de->d_name);
		if ((!io_is_current(de->d_name)))
		   item = add_node (GTK_CTREE(ctree), root, first, label, complete, &type, flags);
		if ((type & FT_DIR) && (!(type & FT_DIR_UP)) 
		    && (!(type & FT_DIR_PD)) && (io_is_valid(de->d_name)) && item)
		  add_subtree (ctree, item, complete, depth-1, flags);
		else if (!first) 
		  first = item;
	}
	g_free (base);
	closedir (dir);
	gtk_ctree_sort_node (ctree, root);
}

/*
 */
void
on_dotfiles (GtkWidget *item, GtkCTree *ctree)
{
	GtkCTreeNode *node, *child;
	entry *en;

	gtk_clist_freeze (GTK_CLIST(ctree));

	/* use first selection if available
	 */
	count_selection (ctree, &node);
	en = gtk_ctree_node_get_row_data (ctree, node);
	if (!(en->type & FT_DIR)) {
		/* select parent node */
		node = GTK_CTREE_ROW(node)->parent;
		en = gtk_ctree_node_get_row_data (ctree, node);
	}
	/* Ignore toggle on parent directory */
	if (en->type & FT_DIR_UP)
	  {
   	    gtk_clist_thaw (GTK_CLIST(ctree));
	    return;
	  }
	child = GTK_CTREE_ROW(node)->children;
	while (child) {
		gtk_ctree_remove_node (ctree, child);
		child = GTK_CTREE_ROW(node)->children;
	}
	if (en->flags & IGNORE_HIDDEN)
		en->flags &= ~IGNORE_HIDDEN;
	else
		en->flags |= IGNORE_HIDDEN;
	add_subtree (ctree, node, en->path, 2, en->flags);
	gtk_ctree_expand (ctree, node);
	gtk_clist_thaw (GTK_CLIST(ctree));
}


/*
 */
void
on_expand (GtkCTree *ctree, GtkCTreeNode *node, char *path)
{
	GtkCTreeNode *child;
	entry *en;

	ctree_freeze (ctree);
	child = GTK_CTREE_ROW(node)->children;
	while (child) {
		gtk_ctree_remove_node (ctree, child);
		child = GTK_CTREE_ROW(node)->children;
	}
	en = gtk_ctree_node_get_row_data (ctree, node); 
	add_subtree (ctree, node, en->path, 2, en->flags);
	ctree_thaw (ctree);
}

/*
 * unmark all marked on collapsion
 */
void
on_collapse (GtkCTree *ctree, GtkCTreeNode *node, char *path)
{
	GtkCTreeNode *child;
	/* unselect all children */
	child = GTK_CTREE_NODE(GTK_CTREE_ROW(node)->children);
	while (child) {
		gtk_ctree_unselect (ctree, child);
		child = GTK_CTREE_ROW(child)->sibling;
	}
}

/*
 * realy delete files incl. subs
 */
void
delete_files (char *path)
{
	struct stat st;
	DIR *dir;
	char *test;
	struct dirent *de;
	char complete[PATH_MAX+NAME_MAX+1];

	if (lstat (path, &st) == -1) {
		perror (path);
		return ;
	}
	if ((test = strrchr (path, '/')))
	  {
	    test++;
	    if (!io_is_valid (test)) return;
	  }
	if (S_ISDIR (st.st_mode) && (!S_ISLNK (st.st_mode))) {
		if (access (path, R_OK|W_OK) == -1) {
			return;
		}
		dir = opendir (path);
		if (!dir) {
			return;
		}
		while ((de = readdir(dir)) != NULL) {
			if (io_is_current (de->d_name))
				continue;
			if (io_is_dirup (de->d_name))
				continue;
			sprintf (complete, "%s/%s", path, de->d_name);
			delete_files (complete);
		}
		closedir (dir);
		rmdir (path);
	} else {
		unlink (path);
	}
}

/*
 * file: filename incl. path
 * label: filename
 * target: copy filename to target directory
 */
int
move_file (char *ofile, char *label, char *target, int trash)
{
	int len, num =0;
	struct stat stfile, st, stdir;
	char nfile[PATH_MAX+NAME_MAX+1];
	char lnk[PATH_MAX+1];
	FILE *ofp, *nfp;
	char buff[1024];
	struct utimbuf ut;

	if (!io_is_valid (label))
		return(FALSE);

	if (!io_can_write_to_parent (ofile)) {
		return (FALSE);
	}

	if (access (target, W_OK|X_OK) == -1)
		return (FALSE);

	/* move or copy/delete */
	if (lstat (ofile, &stfile) == -1)
		return (FALSE);
	if (stat (target, &stdir) == -1)
		return (FALSE);
	sprintf (nfile, "%s/%s", target, label);
	while (++num) {
		if (lstat (nfile, &st) == 0) {
			/* file still exists */
			if (!trash)
				return (FALSE);
			/* just use a new file name
			 */
			sprintf (nfile, "%s/%s;%d", target, label, num);
		} else
			break;
	}
	if (strcmp (ofile, nfile) == 0) {
		/* source and target are the same
		*/
		return (FALSE);
	}

	if (stfile.st_dev == stdir.st_dev) {
		/* rename */
		if (rename (ofile, nfile) == -1) {
			return (FALSE);
		}
		return (TRUE);
	}

	/* check if file is a symbolic link */
	if (S_ISLNK(stfile.st_mode)) {
		len = readlink (ofile, lnk, PATH_MAX);
		if (len <= 0) {
			perror ("readlink()");
			return (FALSE);
		}
		lnk[len] = '\0';
		if (symlink (lnk, nfile) == -1)
			return (FALSE);
		if (unlink (ofile) == -1) {
			perror ("unlink()");
			return (FALSE);
		}
		return (TRUE);
	}

	/* we can just rename but not copy special device files ..
	 */
	if (S_ISCHR(stfile.st_mode) ||
		S_ISBLK(stfile.st_mode) ||
		S_ISFIFO(stfile.st_mode)||
		S_ISSOCK(stfile.st_mode)) {
		printf (_("Can't copy device, fifo and socket files as regular files!\n"));
	}
	/* copy and delete
	 */
	ofp = fopen (ofile, "rb");
	if (!ofp)
		return (0);
	nfp = fopen (nfile,"wb");
	if (!nfp) {
		fclose (ofp);
		return (0);
	}
	while ((num = fread(buff, 1, 1024, ofp)) > 0) {
		fwrite (buff, 1, num, nfp);
	}
	fclose (nfp);
	fclose (ofp);
	/* reset time stamps
	 */
	ut.actime = stfile.st_atime;
	ut.modtime= stfile.st_mtime;
	utime (nfile, &ut);
	if (unlink (ofile) != 0)
		return (FALSE);
	return (TRUE);
}

/*
 * path: directory incl. path
 * label: directory
 * target: copy source to target directory
 * trash: if == 1, auto-rename in trash-dir
 */
int
move_dir (char *source, char *label, char *target, int trash)
{
	DIR *dir;
	int len, num = 0;
	struct dirent *de;
	struct stat st_source, st_target, st_file;
	char new_path[PATH_MAX+1];
	char file[PATH_MAX+NAME_MAX+1];
	char name[NAME_MAX+1];
	
	if (access (target, X_OK|W_OK) != 0) {
		perror (target);
		return (FALSE);
	}
	if (access (source, X_OK|R_OK) != 0) {
		perror (source);
		return (FALSE);
	}
	if (lstat (target, &st_target) != 0) {
		perror (target);
		return (FALSE);
	}
	if (lstat (source, &st_source) != 0) {
		perror (target);
		return (FALSE);
	}

	if (!(io_is_valid (label)))
	    return(FALSE);

	sprintf (new_path, "%s/%s", target, label);
	while (++num) {
		if (lstat (new_path, &st_file) == 0) {
			if (!trash)
				return (FALSE);
			/* dir still exists, we have to rename */
			sprintf (new_path, "%s/%s;%d", target, label, num);
		} else
			break;
	}
	if (st_source.st_dev == st_target.st_dev) {
		if (rename (source, new_path) == -1)
			return (FALSE);
		return (TRUE);
	}

	if (!S_ISDIR(st_source.st_mode)) {
		printf (_("Moving file..\n"));
		return move_file (source, label, target, trash);
	}

	/* we have to copy .. */
	dir = opendir (source);
	if (!dir) {
		perror (source);
		return (FALSE);
	}
	if (mkdir (new_path, 0xFFFF) == -1) {
		perror (source);
		closedir(dir);
		return (FALSE);
	}

	while ((de = readdir (dir)) != NULL) {
		len = strlen (de->d_name);
		if (((len == 1) && (*de->d_name == '.')) ||
			((len == 2) && (de->d_name[0] == '.') && (de->d_name[1] == '.'))) {
			continue;
		}
		strcpy (name, de->d_name);
		sprintf (file, "%s/%s", source, name);
		if (lstat (file, &st_file) != 0) {
			perror (file);
			return (FALSE);
		}
		if (S_ISDIR(st_file.st_mode)) {
			if (move_dir (file, name, new_path, trash) != TRUE) {
				printf (_("move_dir() recursive failed\n"));
				return (FALSE);
			}
		} else {
			if (move_file (file, name, new_path, trash) != TRUE) {
				printf (_("move_dir() move_file() failed\n"));
				return (FALSE);
			}
		}
	}
	closedir (dir);
	rmdir (source);
	return (TRUE);
}

/*
 * find a node and check if it is expanded
 */
void
node_is_open (GtkCTree *ctree, GtkCTreeNode *node, void *data)
{
	GtkCTreeRow *row;
	entry *check = (entry *) data;
	entry *en = gtk_ctree_node_get_row_data (ctree, node);
	if (strcmp (en->path, check->path) == 0) {
		row = GTK_CTREE_ROW(node);
		if (row->expanded) {
			check->label = (char *)node;
			check->flags = TRUE;
		}
	}
}

/*
 */
int
compare_node_path (gconstpointer ptr1, gconstpointer ptr2)
{
	entry *en1 = (entry *)ptr1, *en2 = (entry *) ptr2;

	return strcmp (en1->path, en2->path);
}

/*
 * empty trash folder
 */
void
cb_empty_trash (GtkWidget *widget, GtkCTree *ctree)
{
	GtkCTreeNode *node;
	cfg *win;
	DIR *dir;
	struct dirent *de;
	char complete[PATH_MAX+1];
	entry check;

	win = gtk_object_get_user_data (GTK_OBJECT(ctree));
	check.path = win->trash;
	check.flags = FALSE;
	if (!win)
		return;
	/* check if the trash dir is open, so we have to update */
	gtk_ctree_pre_recursive (ctree,
		GTK_CTREE_NODE(GTK_CLIST(ctree)->row_list), node_is_open, &check);
	dir = opendir (win->trash);
	if (!dir)
		return;
	cursor_wait (GTK_WIDGET(ctree));
	while ((de = readdir(dir)) != NULL) {
		if (io_is_current (de->d_name))
			continue;
		if (io_is_dirup (de->d_name))
			continue;
		sprintf (complete, "%s/%s", win->trash, de->d_name);
		delete_files (complete);
		if (check.flags) {
			/* remove node */
			check.path = complete;
			node = gtk_ctree_find_by_row_data_custom (ctree,
						GTK_CTREE_NODE(GTK_CLIST(ctree)->row_list), &check,
						compare_node_path);
			if (node) {
				gtk_ctree_remove_node (ctree, node);
			}
		}
	}
	closedir(dir);
	cursor_reset (GTK_WIDGET(ctree));
}

/*
 * menu callback for deleting files
 */
static void
cb_delete (GtkWidget *widget, GtkCTree *ctree)
{
	int num, i;
	GtkCTreeNode *node;
	entry *en;
	int result;
	int ask = TRUE;
	cfg *win;

	win = gtk_object_get_user_data (GTK_OBJECT(ctree));

	num = count_selection (ctree, &node);
	if (!num) {
		/* nothing to delete */
		dlg_warning (_("No files marked !"));
		return;
	}
	for (i = 0; i < num; i++) {
		if (!GTK_CLIST(ctree)->selection) {
			continue;
		}
		node = GTK_CLIST(ctree)->selection->data;
		en = gtk_ctree_node_get_row_data (ctree, node);
		if (!io_is_valid(en->label) || (en->type & FT_DIR_UP)) {
			/* we do not process ".." */
			gtk_ctree_unselect (ctree, node);
			continue;
		}
		if (ask) {
			if (num -i == 1)
				result = dlg_question(_("Delete item ?"),en->path);
			else
				result = dlg_question_l(_("Delete item ?"),en->path,DLG_ALL|DLG_SKIP);
		} else 
			result = DLG_RC_ALL;
		if (result == DLG_RC_CANCEL) {
			return ;
		} else if (result == DLG_RC_OK || result == DLG_RC_ALL) {
			if (result == DLG_RC_ALL) {
				ask = FALSE;
			}
			while (gtk_events_pending ())
				gtk_main_iteration ();
			ctree_freeze (ctree);
			if ((en->type & FT_FILE)||(en->type & FT_LINK)) {
				if (!move_file (en->path, en->label, win->trash, 1)) {
					perror (_("move_file()"));
					gtk_ctree_unselect (ctree, node);
					if (dlg_error_continue(en->path, _("Move to trash failed"))
						== DLG_RC_CANCEL) {
						ctree_thaw (ctree);
						return;
					}
				} else {
					gtk_ctree_remove_node (ctree, node);
				}
			} else if (en->type & FT_DIR) {
				if (!move_dir (en->path, en->label, win->trash, 1)) {
					dlg_error(en->path, _("Failed"));
					gtk_ctree_unselect (ctree, node);
				} else {
					gtk_ctree_remove_node (ctree, node);
				}
			} else {
				if (dlg_question(_("Can't move file to trash, hard delete ?"),
					en->path) == DLG_RC_OK) {
					if (unlink (en->path) == ERROR) {
						dlg_error(en->path, strerror(errno));
					}
				}
				gtk_ctree_unselect (ctree, node);
			}
			ctree_thaw (ctree);
		} else if (result == DLG_RC_SKIP) {
			gtk_ctree_unselect (ctree, node);
		}
	}
}

/*
 * open find dialog
 */
void
cb_find (GtkWidget *item, GtkWidget *ctree)
{
	GtkCTreeNode *node;
	char pattern[PATH_MAX+1];
	char path[PATH_MAX+1];
	entry *en;

	count_selection (GTK_CTREE(ctree), &node);
	en = gtk_ctree_node_get_row_data(GTK_CTREE(ctree), node);

	strcpy (pattern, "*.c");
	sprintf(path, _("Find pattern in '%s'"), en->path);
	if (dlg_string (path, pattern) == DLG_RC_OK) {
		sprintf (path, "%s -e sh -c \"find %s -name '%s' -print | more ; echo Press ENTER; read\"",
			TERMINAL, en->path, pattern);
		io_system (path);
	}
}

/*
 */
void
cb_about (GtkWidget *item, GtkWidget *ctree)
{
	dlg_info(_("This is XFTree, based on 'XTree' "
				"\n(c) by Rasca\npublished under GNU GPL"
				"\nhttp://home.pages.de/~rasca/xap/"));
}

/*
 */
static void
set_title (GtkWidget *w, const char *path)
{
	char title[PATH_MAX+1+20];
	sprintf (title, "XFTree: %s", path);
	gtk_window_set_title (GTK_WINDOW(gtk_widget_get_toplevel(w)), title);
}

/*
 */
static void
go_to (GtkCTree *ctree, GtkCTreeNode *root, char *path, int flags)
{
	entry *en;
	int i;
	char *label[COLUMNS];
	char *icon_name;

	en = entry_new_by_path_and_label (path, path);
	if (!en) {
		printf (_("Can't find row data\n"));
		return;
	}
	if (!io_is_valid(en->label) || (en->type & FT_DIR_UP))
		return;
	en->flags= flags;

	for (i = 0; i < COLUMNS; i++) {
		if (i == COL_NAME)
			label[i] = uri_clear_path(en->path);
		else
			label[i] = "";
	}
	ctree_freeze (ctree);
	gtk_ctree_remove_node (ctree, root);

	root = gtk_ctree_insert_node (ctree, NULL, NULL, label, 8,
				gPIX_dir_close, gPIM_dir_close, gPIX_dir_open, gPIM_dir_open,
				FALSE, TRUE);
	gtk_ctree_node_set_row_data_full (ctree, root, en, node_destroy);
	add_subtree (ctree, root, uri_clear_path(en->path), 2, en->flags);
	ctree_thaw (ctree);
	set_title (GTK_WIDGET(ctree), uri_clear_path(en->path));
	icon_name = strrchr(en->path, '/');
	if ((icon_name) && (!(*(++icon_name)))) icon_name = NULL;
	gdk_window_set_icon_name (gtk_widget_get_toplevel(GTK_WIDGET(ctree))->window, (icon_name ? icon_name : "/"));
}

/*
 */
static void
free_list (GList *list)
{
	GList *t = list;

	while (t) {
		g_free (t->data);
		t = t->next;
	}
	g_list_free (list);
}

/*
 */
void
cb_go_to (GtkWidget *item, GtkCTree *ctree)
{
	GtkCTreeNode *node, *root;
	entry *en;
	char path[PATH_MAX+1], *p;
	GList *list;
	int count;

	root = GTK_CTREE_NODE(GTK_CLIST(ctree)->row_list);
	count = count_selection (ctree, &node);
	en = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), node);

	if ((!count) || (count > 1) || (en && (!(en->type & FT_DIR)))) {
		list = NULL;
		if (en) {
			strcpy (path, en->path);
			p = path;
			while ((p = strrchr (path, '/')) != NULL) {
				if (p == path) {
					*(p+1) = '\0';
					list = g_list_append (list, g_strdup(path));
					break;
				}
				*p = '\0';
				list = g_list_append (list, g_strdup (path));
			}
			if (!io_is_valid(en->label) || (en->type & FT_DIR_UP)) {
				strcpy (path, en->path);
			} else {
				p = strrchr (en->path, '/');
				if (p) {
					strncpy (path, en->path, p - en->path);
					path[p - en->path] = '\0';
				} else {
					strcpy (path, en->path);
				}
			}
		} else {
			list = g_list_append (list, "/");
			list = g_list_append (list, "/usr");
			list = g_list_append (list, "/home");
			strcpy (path, "/");
		}
		if (dlg_combo(_("Go to"), path, list) != DLG_RC_OK) {
			free_list (list);
			return;
		}
		free_list (list);
	} else {
		if (en)
			strcpy (path, en->path);
	}
	if (en)
		go_to (ctree, root, path, en->flags);
}

/*
 */
void
cb_go_home (GtkWidget *item, GtkCTree *ctree)
{
	GtkCTreeNode *root;
	root = GTK_CTREE_NODE(GTK_CLIST(ctree)->row_list);
	go_to (ctree, root, getenv ("HOME"), IGNORE_HIDDEN);
}


/*
 * change root and go one directory up
 */
void
cb_go_up (GtkWidget *item, GtkCTree *ctree)
{
	entry *en;
	char path[PATH_MAX+1], *p;
	GtkCTreeNode *root;
	
	root = GTK_CTREE_NODE(GTK_CLIST(ctree)->row_list);
	en = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), root);
	if (!en) {
		return;
	}

	strcpy (path, en->path);
	p = strrchr (path, '/');
	if (p == path) {
		if (!*(p+1))
			return;
		*(p+1) = '\0';
	} else
		*p = '\0';
	go_to (ctree, root, path, en->flags);
}

/*
 * start the marked program on double click
 */
static gint
on_double_click (GtkWidget *ctree, GdkEventButton *event, void *menu)
{
	GtkCTreeNode *node;
	entry *en, *up;
	char cmd[(PATH_MAX+3)*2];
	char *wd;
	cfg *win;
	reg_t *prg;
	gint row, col;
	if ((event->type == GDK_2BUTTON_PRESS) && (event->button == 1)) {
		/* double_click
		 */

		/* check if the double click was over a directory
		 */
		row = -1;
		gtk_clist_get_selection_info (GTK_CLIST(ctree), event->x,event->y, &row, &col);
		if (row > -1) {
			node = gtk_ctree_node_nth (GTK_CTREE(ctree), row);
			en = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), node);
			if (EN_IS_DIR(en) && (event->state & GDK_MOD1_MASK)) {
				/* Alt button is pressed it's the same as _go_to()..
				 */
				go_to (GTK_CTREE(ctree),
						GTK_CTREE_NODE(GTK_CLIST(ctree)->row_list),
						en->path, en->flags);
				return (TRUE);
			}
		}
		if (!count_selection(GTK_CTREE(ctree), &node)) {
			return (TRUE);
		}
		if (!node) {
			return (TRUE);
		}
		en = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), node);

		if (en->type & FT_DIR_UP) {
			node = GTK_CTREE_NODE(GTK_CLIST(ctree)->row_list);
			go_to (GTK_CTREE(ctree), node, uri_clear_path(en->path), en->flags);
			return (TRUE);
		}
		if (!(en->type & FT_FILE))
			return (FALSE);
		up = gtk_ctree_node_get_row_data (GTK_CTREE(ctree),
				GTK_CTREE_ROW(node)->parent);
		cursor_wait (GTK_WIDGET(ctree));

		wd = getcwd (NULL, PATH_MAX);
		chdir (up->path);
		if (en->type & FT_EXE) /*io_can_exec (en->path))*/ {
			if (event->state & GDK_MOD1_MASK)
				sprintf (cmd, "%s -e \"%s\" &", TERMINAL, en->path);
			else
				sprintf (cmd, "\"%s\" &", en->path);
			io_system (cmd);
		} else {
			/* call open with dialog */
			win = gtk_object_get_user_data (GTK_OBJECT(ctree));
			prg = reg_prog_by_file (win->reg, en->path);
			if (prg) {
				if (prg->arg)
					sprintf (cmd, "\"%s\" %s \"%s\" &",
								prg->app, prg->arg, en->path);
				else
					sprintf (cmd, "\"%s\" \"%s\" &", prg->app, en->path);
				io_system (cmd);
			} else {
				dlg_open_with (win->xap, "", en->path);
			}
		}
		chdir (wd);
		free (wd);
		cursor_reset (GTK_WIDGET(ctree));
		return (TRUE);
	}
	return (FALSE);
}

/*
 * handle keyboard short cuts
 */
static gint
on_key_press (GtkWidget *ctree, GdkEventKey *event, void *menu)
{
	int num, i;
	entry *en;
	GtkCTreeNode *node;
	GdkEventButton evbtn;

	switch (event->keyval) {
		case GDK_Return:
			evbtn.type = GDK_2BUTTON_PRESS;
			evbtn.button = 1;
			evbtn.state = event->state;
			on_double_click (ctree, &evbtn, menu);
			return (TRUE);
			break;
		default:
			if ((event->keyval >= GDK_A) && (event->keyval <= GDK_z) &&
				(event->state <= GDK_SHIFT_MASK)) {
				num = g_list_length (GTK_CLIST(ctree)->row_list);
				for (i =0 ; i < num; i++) {
					node = gtk_ctree_node_nth (GTK_CTREE(ctree), i);
					en = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), node);
					if (en->label && (*en->label == (char)event->keyval) &&
						gtk_ctree_node_is_visible(GTK_CTREE(ctree), node)) {
						GTK_CLIST(ctree)->focus_row = i;
						gtk_ctree_unselect_recursive (GTK_CTREE(ctree), NULL);
						gtk_clist_moveto (GTK_CLIST(ctree), i, COL_NAME, 0,0);
						gtk_clist_select_row (GTK_CLIST(ctree), i, COL_NAME);
						break;
					}
				}
				return (TRUE);
			}
			break;
	}
	return (FALSE);
}

/*
 */
static int
node_has_child (GtkCTree *ctree, GtkCTreeNode *node, char *label)
{
	GtkCTreeNode *child;
	entry *en;

	child = GTK_CTREE_ROW(node)->children;
	while (child) {
		en = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), child);
		if (strcmp (en->label, label) == 0) {
			return (1);
		}
		child = GTK_CTREE_ROW(child)->sibling;
	}
	return (0);
}

/*
 * update a node and its childs if visible
 * return 1 if some nodes have removed or added, else 0
 *
 */
static int
update_tree (GtkCTree *ctree, GtkCTreeNode *node)
{
 	GtkCTreeNode *child = NULL, *new_child = NULL, *next;
	entry *en, *child_en;
	struct dirent *de;
	DIR *dir;
	char compl[PATH_MAX+1];
	char label[NAME_MAX+1];
	int type, p_len, changed, tree_updated, root_changed;
	gchar size[16];
	gchar date[32];

	tree_updated = FALSE;
	en = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), node);
	if ((root_changed = entry_update (en)) == ERROR) 
	  {
	    next = GTK_CTREE_ROW(node)->sibling;
 	    gtk_ctree_remove_node (ctree, node);
	    if (!next) 
	      return TRUE;
	    node = next;
	}
	child = GTK_CTREE_ROW(node)->children;
	while (child) {
		child_en = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), child);
		if ((changed = entry_update (child_en)) == ERROR) {
			gtk_ctree_remove_node (ctree, child);
			/* realign the list */
			child = GTK_CTREE_ROW(node)->children;
			tree_updated = TRUE;
			continue;
		} else if (changed == TRUE) {
			/* update the labels */
			sprintf (date, "%02d-%02d-%02d  %02d:%02d",
				child_en->date.year, child_en->date.month, child_en->date.day,
				child_en->date.hour, child_en->date.min);
			sprintf (size, "%10d", (int)child_en->size);
	                gtk_ctree_node_set_text (ctree, child, COL_DATE, date);
	                gtk_ctree_node_set_text (ctree, child, COL_SIZE, size);
		}
		if (entry_type_update(child_en) == TRUE) {
			update_node (ctree, child, child_en->type, child_en->label);
			tree_updated = TRUE;
		}
	  	if (!(GTK_CTREE_ROW(child)->children) && (io_is_valid(child_en->label)) 
		    && !(child_en->type & FT_DIR_UP) && !(child_en->type & FT_DIR_PD)
		    && (child_en->type & FT_DIR))
               		add_subtree (GTK_CTREE(ctree), child, child_en->path, 1, child_en->flags);
		child = GTK_CTREE_ROW(child)->sibling;
	}

	if ((root_changed || tree_updated) && (en->type & FT_DIR)) 
	  {
	    if (GTK_CTREE_ROW(node)->expanded)
	      {
	    	/* may be there are new files */
		dir = opendir (en->path);
		if (!dir) return TRUE;
		p_len = strlen (en->path);
		while ((de = readdir (dir)) != NULL) 
		  {
		     if (io_is_hidden(de->d_name) && (en->flags & IGNORE_HIDDEN))
		       continue;
		     if (io_is_current(de->d_name))
		       continue;
		     strcpy (label, de->d_name);
		     if (!node_has_child (ctree, node, label) && !(io_is_current(label))) 
		       {
		     	  if (io_is_root(label))
		     	    sprintf (compl, "%s%s", en->path, label);
		     	  else
		     	    sprintf (compl, "%s/%s", en->path, label);
		     	  type = 0;
			  new_child = NULL;
			  if (!io_is_current(label) && label)
		     	    new_child = add_node (ctree, node, NULL, label, compl, &type, en->flags);
		     	  if ((type & FT_DIR) && (io_is_valid(label)) 
			      && !(type & FT_DIR_UP) && !(type & FT_DIR_PD) && new_child)
		     	    add_subtree (ctree, new_child, compl, 1, en->flags);
			  if (entry_type_update(en) == TRUE)
			  	update_node (ctree, node, en->type, en->label);
			  entry_update (en);
		     	  tree_updated = TRUE;
		       }
		  }
		closedir (dir);
	     }
	  else if ((GTK_CTREE_ROW(node)->children) && (io_is_valid(en->label)) 
	  	   && !(en->type & FT_DIR_UP) && !(en->type & FT_DIR_PD))
	    {
	       add_subtree (GTK_CTREE(ctree), node, en->path, 1, en->flags);
	       if (entry_type_update(en) == TRUE)
	       		update_node (ctree, node, en->type, en->label);
	       entry_update (en);
	    }
	  if (tree_updated) {
	     gtk_ctree_sort_node (GTK_CTREE(ctree), node);
	  }
	}
	return (tree_updated);
}


/*
 * create a new folder in the current
 */
void
cb_new_subdir (GtkWidget *item, GtkWidget *ctree)
{
	entry *en;
	GtkCTreeNode *node;
	char path[PATH_MAX+1];
	char label[PATH_MAX+1];
	char compl[PATH_MAX+1];

	count_selection (GTK_CTREE(ctree), &node);
	en = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), node);
	if (!(en->type & FT_DIR)) {
		node = GTK_CTREE_ROW(node)->parent;
		en = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), node);
	}

	if (!GTK_CTREE_ROW(node)->expanded)
		gtk_ctree_expand (GTK_CTREE(ctree), node);

	if (en->path[strlen(en->path)-1] == '/')
		sprintf (path, "%s", en->path);
	else
		sprintf (path, "%s/", en->path);
	strcpy (label, _("New_Folder"));
	if (dlg_string(path, label) == DLG_RC_OK) {
		sprintf (compl, "%s%s", path, label);
		if (mkdir (compl, 0xFFFF) != -1)
			update_tree (GTK_CTREE(ctree), node);
		else dlg_error (compl, strerror(errno));
	}
}

/*
 * new file
 */
void
cb_new_file (GtkWidget *item, GtkWidget *ctree)
{
	entry *en;
	GtkCTreeNode *node;
	char path[PATH_MAX+1];
	char label[PATH_MAX+1];
	char compl[PATH_MAX+1];
	int tmp = 0, exists = 0;
	struct stat st;
	FILE *fp;

	count_selection (GTK_CTREE(ctree), &node);
	en = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), node);

	if (!(en->type & FT_DIR)) {
		node = GTK_CTREE_ROW(node)->parent;
		en = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), node);
	}

	if (!GTK_CTREE_ROW(node)->expanded)
		gtk_ctree_expand (GTK_CTREE(ctree), node);

	if (en->path[strlen(en->path)-1] == '/')
		sprintf (path, "%s", en->path);
	else
		sprintf (path, "%s/", en->path);
	strcpy (label, "New_File.c");
	if ((dlg_string(path, label) == DLG_RC_OK) && strlen(label) && io_is_valid(label)) {
		sprintf (compl, "%s%s", path, label);
		if (stat (compl, &st) != -1) {
			if (dlg_question (_("File exists ! Override ?"),compl) != DLG_RC_OK){
				return;
			}
			exists = 1;
		}
		fp = fopen (compl, "w");
		if (!fp) {
			dlg_error(_("Can't create : "), compl);
			return;
		}
		fclose (fp);
		if (!exists)
		  add_node (GTK_CTREE(ctree), node, NULL, label, compl,&tmp,en->flags);
	}
}


/*
 * duplicate a file
 */
void
cb_duplicate (GtkWidget *item, GtkCTree *ctree)
{
	entry *en;
	GtkCTreeNode *node;
	char nfile[PATH_MAX+1];
	int num, len;
	struct stat s;
	FILE *ofp, *nfp;
	char buf[MAXBUF];

	if (!count_selection (ctree, &node)) {
		return;
	}
	en = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), node);

	if (!io_is_valid(en->label) || (en->type & FT_DIR))
		return;
	cursor_wait (GTK_WIDGET(ctree));
	num = 0;
	sprintf (nfile, "%s-%d", en->path, num++);
	while (stat(nfile, &s) != -1) {
		sprintf (nfile, "%s-%d", en->path, num++);
	}
	ofp = fopen (en->path, "rb");
	if (!ofp) {
		dlg_error(en->path, strerror(errno));
		cursor_reset(GTK_WIDGET(ctree));
		return;
	}
	nfp = fopen (nfile, "wb");
	if (!nfp) {
		dlg_error(nfile, strerror(errno));
		fclose (ofp);
		cursor_reset(GTK_WIDGET(ctree));
		return;
	}
	while ((len = fread (buf, 1, MAXBUF, ofp)) > 0) {
		fwrite (buf, 1, len, nfp);
	}
	fclose (nfp);
	fclose (ofp);
	cursor_reset(GTK_WIDGET(ctree));
}

/*
 * rename a file
 */
void
cb_rename (GtkWidget *item, GtkCTree *ctree)
{
	entry *en;
	GtkCTreeNode *node;
	GdkPixmap *pix, *pim;
	guint8 spacing;
	char ofile[PATH_MAX+NAME_MAX+1];
	char nfile[PATH_MAX+NAME_MAX+1];
	char *p;
	struct stat st;

	if (!count_selection (ctree, &node)) {
		dlg_warning (_("No item marked !"));
		return;
	}
	en = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), node);
	if (!io_is_valid(en->label) || (en->type & FT_DIR_UP))
		return;
	if (strchr (en->label, '/'))
		return;
	sprintf (nfile, "%s", en->label);

	if ((dlg_string(_("Rename to : "), nfile) == DLG_RC_OK) && strlen(nfile) && io_is_valid(nfile)) {
		if ((p=strchr (nfile, '/')) != NULL) {
			p[1] = '\0';
			dlg_error (_("Character not allowed in filename"), p);
			return;
		}
		sprintf (ofile, "%s", en->path);
		p = strrchr (ofile, '/');
		p++;
		sprintf (p, "%s", nfile);
		strcpy (nfile, ofile);
		strcpy (ofile, en->path);
		if (lstat (nfile, &st) != ERROR) {
			if (dlg_question (_("Override ?"), ofile) != DLG_RC_OK) {
				return;
			}
		}
		if (rename (ofile, nfile) == -1) {
			dlg_error(nfile, strerror(errno));
		} else {
			g_free (en->path);
			g_free (en->label);
			en->path = g_strdup (nfile);
			p = strrchr (nfile, '/');
			p++;
			en->label = g_strdup (p);
			gtk_ctree_get_node_info (ctree, node, NULL, &spacing, &pix, &pim,
					NULL, NULL, NULL, NULL);
			gtk_ctree_node_set_pixtext (ctree, node, 0, p, spacing, pix, pim);
			update_tree (ctree, node);
		}
	}
}

/*
 * call the dialog "open with"
 */
void
cb_open_with (GtkWidget *item, GtkCTree *ctree)
{
	entry *en;
	cfg *win;
	GtkCTreeNode *node;
	char *prg;

	if (!count_selection (ctree, &node)) {
		dlg_warning (_("No files marked !"));
		return;
	}
	en  = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), node);
	win = gtk_object_get_user_data (GTK_OBJECT(ctree));
	prg = reg_app_by_file (win->reg, en->path);
	dlg_open_with (win->xap, prg ? prg : DEF_APP, en->path);
}

/*
 * call the dialog "properties"
 */
void
cb_props (GtkWidget *item, GtkCTree *ctree)
{
	entry *en;
	GtkCTreeNode *node;
	fprop oprop, nprop;
	GList *selection;
	struct stat fst;
	int rc = DLG_RC_CANCEL, ask = 1, flags = 0;
	int first_is_stale_link = 0;

	ctree_freeze(ctree);
	selection = g_list_copy(GTK_CLIST(ctree)->selection);

	while (selection) {
		node = selection->data;
		en = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), node);
		if (!io_is_valid(en->label)) {
		       selection = selection->next;
		       continue;
		}

		if (selection->next) flags |=IS_MULTI;
		if (lstat (en->path, &fst) == -1)
		  {
			if (dlg_continue (en->path, strerror(errno)) != DLG_RC_OK)
			{
				g_list_free(selection);
				ctree_thaw(ctree);
				return ;
			}
		        selection = selection->next;
		        continue;
		  }
		else
		  {
		       if (S_ISLNK(fst.st_mode)) {
			       if (stat (en->path, &fst) == -1) {
				       flags |= IS_STALE_LINK;
				       if (ask) {
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
		       oprop.uid  = fst.st_uid;
		       oprop.gid  = fst.st_gid;
		       oprop.ctime= fst.st_ctime;
		       oprop.mtime= fst.st_mtime;
		       oprop.atime= fst.st_atime;
		       oprop.size = fst.st_size;
		       if (ask) {
			       nprop.mode = oprop.mode;
			       nprop.uid  = oprop.uid ;
			       nprop.gid  = oprop.gid ;
			       nprop.ctime= oprop.ctime;
			       nprop.mtime= oprop.mtime;
			       nprop.atime= oprop.atime;
			       nprop.size = oprop.size;
			       rc = dlg_prop (en->path, &nprop, flags);
		       }
		       switch (rc) {
			       case DLG_RC_OK:
			       case DLG_RC_ALL:
				 if(io_is_valid(en->label))
				    {
				       if ((oprop.mode != nprop.mode) &&
					       (!(flags & IS_STALE_LINK)) &&
					       (! first_is_stale_link)) {
					       /* chmod() on a symlink itself isn't possible */
					       if (chmod (en->path, nprop.mode) == -1) {
						       if (dlg_continue (en->path, strerror(errno)) != DLG_RC_OK)
						       {
							       g_list_free(selection);
							       ctree_thaw(ctree);
							       return;
						       }
		       				       selection = selection->next;
		       				       continue;
					       }
				       }
				       if ((oprop.uid != nprop.uid) ||
					       (oprop.gid != nprop.gid)) {
					       if (chown (en->path, nprop.uid, nprop.gid) == -1) {
						       if (dlg_continue (en->path, strerror(errno)) != DLG_RC_OK)
						       {
							       g_list_free(selection);
							       ctree_thaw(ctree);
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
				       ctree_thaw(ctree);
				       g_list_free(selection);
				       return;
				       break;
		       }
		}
		selection = selection->next;
	}
	g_list_free(selection);
	ctree_thaw(ctree);
}


/*
 * ask user if he want to register a named suffix
 */
void
cb_register (GtkWidget *item, GtkWidget *ctree)
{
	GtkCTreeNode *node;
	char label[PATH_MAX+1];
	char path[PATH_MAX+1];
	char *sfx, *arg;
	entry *en;
	cfg *win;
	GList *apps;
	reg_t *prog;

	if (!GTK_CLIST(ctree)->selection)
		return;
	ctree_freeze(GTK_CTREE(ctree));
	node = GTK_CLIST(ctree)->selection->data;
	en = gtk_ctree_node_get_row_data(GTK_CTREE(ctree), node);
	win = gtk_object_get_user_data (GTK_OBJECT(ctree));

	sfx = strrchr (en->label, '.');
	if (!sfx) {
		if (dlg_continue(
			_("Can't find suffix in filename, using complete filename"), en->label) != DLG_RC_OK)
			{
				ctree_thaw(GTK_CTREE(ctree));
				return;
			}
		sfx = en->label;
		sprintf (label, _("Register program for file \"%s\""), sfx);
	} else {
		sprintf (label, _("Register program for suffix \"%s\""), sfx);
	}
	prog = reg_prog_by_suffix (win->reg, sfx);
	if (prog) {
		if (prog->arg) {
			sprintf (path, "%s %s", prog->app, prog->arg);
		} else {
			strcpy (path, prog->app);
		}
	} else
		strcpy (path, DEF_APP);
	apps = reg_app_list (win->reg);
	if (dlg_combo (label, path, apps) == DLG_RC_OK) {
		if (*path) {
			if ((arg = strchr (path, ' ')) != NULL) {
				*arg++ = '\0';
				if (!*arg)
					arg = NULL;
			}
			win->reg = reg_add_suffix (win->reg, sfx, path, arg);
			reg_save (win->reg);
		}
	}
	ctree_thaw(GTK_CTREE(ctree));
	g_list_free (apps);
}

/*
 */
static
XFCE_palette * global_pal;

void
on_destroy (GtkWidget *top, cfg *win)
{
	top_delete (top);
	gtk_timeout_remove (win->timer);
	g_free (win->trash);
	g_free (win->xap);
	g_free (win);
	if (!top_has_more ()) {
		free_app_list ();
		gtk_main_quit ();
	}
}

/*
 * if window manager send delete event
 */
gint
on_delete (GtkWidget *w, GdkEvent *event, gpointer data)
{
	return (FALSE);
}

/*
 * called if Alt+w was pressed
 */
void
cb_destroy (GtkWidget *top, void *data)
{
	gtk_widget_destroy ((GtkWidget *)data);
}

/*
 * check if a node is a directory and is visible and expanded
 * will be called for every node
 */
static void
get_visible_or_parent (GtkCTree *ctree, GtkCTreeNode *node, gpointer data)
{
	GtkCTreeNode *child;
	GList **list = (GList **)data;

	if (GTK_CTREE_ROW(node)->is_leaf)
		return;

	if (gtk_ctree_node_is_visible (ctree, node) &&
		GTK_CTREE_ROW(node)->expanded) {
		/* we can't remove a node or something else here,
		 * that would break the calling gtk_ctree_pre_recursive()
		 * so we remember the node in a linked list
		 */
		*list = g_list_append (*list, node);
		return;
	}

	/* check if at least one child is visible
	 */
	child = GTK_CTREE_ROW(node)->children;
	while (child) {
		if (gtk_ctree_node_is_visible(GTK_CTREE(ctree), child)) {
			*list = g_list_append (*list, node);
			return;
		}
		child = GTK_CTREE_ROW(child)->sibling;
	}
}

/*
 * timer function to update the view
 */
gint
update_timer (GtkCTree *ctree)
{
	GList *list = NULL, *tmp;
	GtkCTreeNode *node;

	/* get a list of directories we have to check
	 */
	gtk_ctree_post_recursive (ctree,
		GTK_CTREE_NODE(GTK_CLIST(ctree)->row_list),get_visible_or_parent,&list);

	tmp = list;
	while (tmp) {
		node = tmp->data;
		if (update_tree (ctree, node) == TRUE) {
			break;
		}
		tmp = tmp->next;
	}
	g_list_free (list);
	return (TRUE);
}

/*
 */
static void
cb_exec (GtkWidget *top, gpointer data)
{
	cfg *win = (cfg *) data;
	dlg_execute (win->xap, NULL);
}

XErrorHandler 
ErrorHandler (Display * dpy, XErrorEvent * event)
{
  if ((event->error_code   == BadWindow) ||
      (event->error_code   == BadDrawable) ||
      (event->request_code == X_GetGeometry) ||
      (event->request_code == X_ChangeProperty) ||
      (event->request_code == X_SetInputFocus) ||
      (event->request_code == X_ChangeWindowAttributes) ||
      (event->request_code == X_GrabButton) ||
      (event->request_code == X_ChangeWindowAttributes) ||
      (event->request_code == X_InstallColormap))
    return (0);

  fprintf (stderr, "xftree: Fatal XLib internal error\n");
  exit(1);
  return (0);
}

GtkWidget*
create_toolbar (GtkWidget *top, GtkWidget *ctree, cfg* win)
{
  GtkWidget *hbox;
  GtkWidget *button;
  GtkWidget *pixmap;
  GtkWidget *separator;
  GtkTooltips* tooltip;

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);

  button = gtk_button_new ();
  gtk_button_set_relief((GtkButton *) button, GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_widget_set_usize (button, 30, 30);

  tooltip = gtk_tooltips_new();
  gtk_tooltips_set_tip (tooltip, button, _("New window"), "ContextHelp/buttons/?");

  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                             GTK_SIGNAL_FUNC (cb_new_window),
                             (gpointer) ctree);

  pixmap = MyCreateFromPixmapData (button, new_win_xpm);
  if (pixmap == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_show (pixmap);
  gtk_container_add (GTK_CONTAINER (button), pixmap);

  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

  button = gtk_button_new ();
  gtk_button_set_relief((GtkButton *) button, GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_widget_set_usize (button, 30, 30);

  tooltip = gtk_tooltips_new();
  gtk_tooltips_set_tip (tooltip, button, _("Close window"), "ContextHelp/buttons/?");

  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                             GTK_SIGNAL_FUNC (cb_destroy),
                             (gpointer) top);

  pixmap = MyCreateFromPixmapData (button, closewin_xpm);
  if (pixmap == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_show (pixmap);
  gtk_container_add (GTK_CONTAINER (button), pixmap);

  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

  separator = gtk_vseparator_new();
  gtk_widget_show (separator);
  gtk_box_pack_start (GTK_BOX (hbox), separator, FALSE, FALSE, 0);

  button = gtk_button_new ();
  gtk_button_set_relief((GtkButton *) button, GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_widget_set_usize (button, 30, 30);

  tooltip = gtk_tooltips_new();
  gtk_tooltips_set_tip (tooltip, button, _("New Folder ..."), "ContextHelp/buttons/?");

  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                             GTK_SIGNAL_FUNC (cb_new_subdir),
                             (gpointer) ctree);

  pixmap = MyCreateFromPixmapData (button, new_dir_xpm);
  if (pixmap == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_show (pixmap);
  gtk_container_add (GTK_CONTAINER (button), pixmap);

  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

  button = gtk_button_new ();
  gtk_button_set_relief((GtkButton *) button, GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_widget_set_usize (button, 30, 30);

  tooltip = gtk_tooltips_new();
  gtk_tooltips_set_tip (tooltip, button, _("New file ..."), "ContextHelp/buttons/?");

  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                             GTK_SIGNAL_FUNC (cb_new_file),
                             (gpointer) ctree);

  pixmap = MyCreateFromPixmapData (button, new_file_xpm);
  if (pixmap == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_show (pixmap);
  gtk_container_add (GTK_CONTAINER (button), pixmap);

  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

  button = gtk_button_new ();
  gtk_button_set_relief((GtkButton *) button, GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_widget_set_usize (button, 30, 30);

  tooltip = gtk_tooltips_new();
  gtk_tooltips_set_tip (tooltip, button, _("Properties ..."), "ContextHelp/buttons/?");

  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                             GTK_SIGNAL_FUNC (cb_props),
                             (gpointer) ctree);

  pixmap = MyCreateFromPixmapData (button, appinfo_xpm);
  if (pixmap == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_show (pixmap);
  gtk_container_add (GTK_CONTAINER (button), pixmap);

  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

  button = gtk_button_new ();
  gtk_button_set_relief((GtkButton *) button, GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_widget_set_usize (button, 30, 30);

  tooltip = gtk_tooltips_new();
  gtk_tooltips_set_tip (tooltip, button, _("Delete ..."), "ContextHelp/buttons/?");

  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                             GTK_SIGNAL_FUNC (cb_delete),
                             (gpointer) ctree);

  pixmap = MyCreateFromPixmapData (button, delete_xpm);
  if (pixmap == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_show (pixmap);
  gtk_container_add (GTK_CONTAINER (button), pixmap);

  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

  separator = gtk_vseparator_new();
  gtk_widget_show (separator);
  gtk_box_pack_start (GTK_BOX (hbox), separator, FALSE, FALSE, 0);

  button = gtk_button_new ();
  gtk_button_set_relief((GtkButton *) button, GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_widget_set_usize (button, 30, 30);

  tooltip = gtk_tooltips_new();
  gtk_tooltips_set_tip (tooltip, button, _("Open Trash"), "ContextHelp/buttons/?");

  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                             GTK_SIGNAL_FUNC (cb_open_trash),
                             (gpointer) win);

  pixmap = MyCreateFromPixmapData (button, trash_xpm);
  if (pixmap == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_show (pixmap);
  gtk_container_add (GTK_CONTAINER (button), pixmap);

  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

  button = gtk_button_new ();
  gtk_button_set_relief((GtkButton *) button, GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_widget_set_usize (button, 30, 30);

  tooltip = gtk_tooltips_new();
  gtk_tooltips_set_tip (tooltip, button, _("Empty Trash"), "ContextHelp/buttons/?");

  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                             GTK_SIGNAL_FUNC (cb_empty_trash),
                             (gpointer) ctree);

  pixmap = MyCreateFromPixmapData (button, empty_trash_xpm);
  if (pixmap == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_show (pixmap);
  gtk_container_add (GTK_CONTAINER (button), pixmap);

  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

  separator = gtk_vseparator_new();
  gtk_widget_show (separator);
  gtk_box_pack_start (GTK_BOX (hbox), separator, FALSE, FALSE, 0);

  button = gtk_button_new ();
  gtk_button_set_relief((GtkButton *) button, GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_widget_set_usize (button, 30, 30);

  tooltip = gtk_tooltips_new();
  gtk_tooltips_set_tip (tooltip, button, _("Go to ..."), "ContextHelp/buttons/?");

  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                             GTK_SIGNAL_FUNC (cb_go_to),
                             (gpointer) ctree);

  pixmap = MyCreateFromPixmapData (button, go_to_xpm);
  if (pixmap == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_show (pixmap);
  gtk_container_add (GTK_CONTAINER (button), pixmap);

  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

  button = gtk_button_new ();
  gtk_button_set_relief((GtkButton *) button, GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_widget_set_usize (button, 30, 30);

  tooltip = gtk_tooltips_new();
  gtk_tooltips_set_tip (tooltip, button, _("Go up"), "ContextHelp/buttons/?");

  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                             GTK_SIGNAL_FUNC (cb_go_up),
                             (gpointer) ctree);

  pixmap = MyCreateFromPixmapData (button, go_up_xpm);
  if (pixmap == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_show (pixmap);
  gtk_container_add (GTK_CONTAINER (button), pixmap);

  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

  button = gtk_button_new ();
  gtk_button_set_relief((GtkButton *) button, GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_widget_set_usize (button, 30, 30);

  tooltip = gtk_tooltips_new();
  gtk_tooltips_set_tip (tooltip, button, _("Go home"), "ContextHelp/buttons/?");

  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                             GTK_SIGNAL_FUNC (cb_go_home),
                             (gpointer) ctree);

  pixmap = MyCreateFromPixmapData (button, home_xpm);
  if (pixmap == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_show (pixmap);
  gtk_container_add (GTK_CONTAINER (button), pixmap);

  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

  separator = gtk_vseparator_new();
  gtk_widget_show (separator);
  gtk_box_pack_start (GTK_BOX (hbox), separator, FALSE, FALSE, 0);

  button = gtk_button_new ();
  gtk_button_set_relief((GtkButton *) button, GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_widget_set_usize (button, 30, 30);

  tooltip = gtk_tooltips_new();
  gtk_tooltips_set_tip (tooltip, button, _("Toggle Dotfiles"), "ContextHelp/buttons/?");

  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                             GTK_SIGNAL_FUNC (on_dotfiles),
                             (gpointer) ctree);

  pixmap = MyCreateFromPixmapData (button, dotfile_xpm);
  if (pixmap == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_show (pixmap);
  gtk_container_add (GTK_CONTAINER (button), pixmap);

  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

  return hbox;
}

GtkWidget*
create_menu (GtkWidget *top, GtkWidget *ctree, cfg* win)
{
  GtkWidget *menubar;
  GtkWidget *menu;
  GtkWidget *menuitem;

  menubar = gtk_menu_bar_new ();
  gtk_widget_show (menubar);

  /* Create "File" menu */
  menuitem = gtk_menu_item_new_with_label (_("File"));
  gtk_menu_bar_append (GTK_MENU_BAR (menubar), menuitem);
  gtk_widget_show (menuitem);

  menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menu);

  menuitem = gtk_menu_item_new_with_label (_("New window"));
  gtk_menu_append (GTK_MENU (menu), menuitem);
  gtk_widget_show (menuitem);
  gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
                            GTK_SIGNAL_FUNC(cb_new_window),
                            (gpointer) ctree);

  menuitem = gtk_menu_item_new_with_label (_("New Folder ..."));
  gtk_menu_append (GTK_MENU (menu), menuitem);
  gtk_widget_show (menuitem);
  gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
                            GTK_SIGNAL_FUNC(cb_new_subdir),
                            (gpointer) ctree);

  menuitem = gtk_menu_item_new_with_label (_("New file ..."));
  gtk_menu_append (GTK_MENU (menu), menuitem);
  gtk_widget_show (menuitem);
  gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
                            GTK_SIGNAL_FUNC(cb_new_file),
                            (gpointer) ctree);

  menuitem = gtk_menu_item_new();
  gtk_menu_append (GTK_MENU (menu), menuitem);
  gtk_widget_show (menuitem);

  menuitem = gtk_menu_item_new_with_label (_("Run program ..."));
  gtk_menu_append (GTK_MENU (menu), menuitem);
  gtk_widget_show (menuitem);
  gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
                            GTK_SIGNAL_FUNC(cb_exec),
                            (gpointer) top);

  menuitem = gtk_menu_item_new_with_label (_("Delete ..."));
  gtk_menu_append (GTK_MENU (menu), menuitem);
  gtk_widget_show (menuitem);
  gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
                            GTK_SIGNAL_FUNC(cb_delete),
                            (gpointer) ctree);

  menuitem = gtk_menu_item_new();
  gtk_menu_append (GTK_MENU (menu), menuitem);
  gtk_widget_show (menuitem);
  
  menuitem = gtk_menu_item_new_with_label (_("Properties ..."));
  gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
        		    GTK_SIGNAL_FUNC(cb_props),
        		    (gpointer) ctree);
                            
  gtk_menu_append (GTK_MENU (menu), menuitem);
  gtk_widget_show (menuitem);

  menuitem = gtk_menu_item_new();
  gtk_menu_append (GTK_MENU (menu), menuitem);
  gtk_widget_show (menuitem);

  menuitem = gtk_menu_item_new_with_label (_("Open Trash"));
  gtk_menu_append (GTK_MENU (menu), menuitem);
  gtk_widget_show (menuitem);
  gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
                            GTK_SIGNAL_FUNC(cb_open_trash),
                            (gpointer) win);

  menuitem = gtk_menu_item_new_with_label (_("Empty Trash"));
  gtk_menu_append (GTK_MENU (menu), menuitem);
  gtk_widget_show (menuitem);
  gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
                            GTK_SIGNAL_FUNC(cb_empty_trash),
                            (gpointer) ctree);

  menuitem = gtk_menu_item_new();
  gtk_menu_append (GTK_MENU (menu), menuitem);
  gtk_widget_show (menuitem);

  menuitem = gtk_menu_item_new_with_label (_("Close window"));
  gtk_menu_append (GTK_MENU (menu), menuitem);
  gtk_widget_show (menuitem);
  gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
                            GTK_SIGNAL_FUNC(cb_destroy),
                            (gpointer) top);

  menuitem = gtk_menu_item_new();
  gtk_menu_append (GTK_MENU (menu), menuitem);
  gtk_widget_show (menuitem);

  menuitem = gtk_menu_item_new_with_label (_("Quit"));
  gtk_menu_append (GTK_MENU (menu), menuitem);
  gtk_widget_show (menuitem);
  gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
                            GTK_SIGNAL_FUNC(gtk_main_quit),
                            (gpointer) ctree);
  /* Create "Settings" menu */
  menuitem = gtk_menu_item_new_with_label (_("Settings"));
  gtk_menu_bar_append (GTK_MENU_BAR (menubar), menuitem);
  gtk_widget_show (menuitem);

  menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menu);

  menuitem = gtk_menu_item_new_with_label (_("Go to ..."));
  gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
        		    GTK_SIGNAL_FUNC(cb_go_to),
        		    (gpointer) ctree);
                            
  gtk_menu_append (GTK_MENU (menu), menuitem);
  gtk_widget_show (menuitem);

  menuitem = gtk_menu_item_new_with_label (_("Go up"));
  gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
        		    GTK_SIGNAL_FUNC(cb_go_up),
        		    (gpointer) ctree);
                            
  gtk_menu_append (GTK_MENU (menu), menuitem);
  gtk_widget_show (menuitem);

  menuitem = gtk_menu_item_new_with_label (_("Go home"));
  gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
        		    GTK_SIGNAL_FUNC(cb_go_home),
        		    (gpointer) ctree);
                            
  gtk_menu_append (GTK_MENU (menu), menuitem);
  gtk_widget_show (menuitem);

  menuitem = gtk_menu_item_new();
  gtk_menu_append (GTK_MENU (menu), menuitem);
  gtk_widget_show (menuitem);
  
  menuitem = gtk_menu_item_new_with_label (_("Toggle Dotfiles"));
  gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
        		    GTK_SIGNAL_FUNC(on_dotfiles),
        		    (gpointer) ctree);
                            
  gtk_menu_append (GTK_MENU (menu), menuitem);
  gtk_widget_show (menuitem);

  /* Create "Help" menu */
  menuitem = gtk_menu_item_new_with_label (_("Help"));
  gtk_menu_item_right_justify (GTK_MENU_ITEM (menuitem));
  gtk_menu_bar_append (GTK_MENU_BAR (menubar), menuitem);
  gtk_widget_show (menuitem);

  menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menu);
  menuitem = gtk_menu_item_new_with_label (_("About ..."));
  gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
        		    GTK_SIGNAL_FUNC(cb_about),
        		    ctree);
  gtk_menu_append (GTK_MENU (menu), menuitem);
  gtk_widget_show (menuitem);
  
  return menubar;
}

/*
 * create a new toplevel window
 */
static GtkWidget *
new_top (char *path, char *xap, char *trash, GList *reg, int width, int height,
			int flags)
{
        GtkWidget *vbox;
        GtkWidget *handlebox1;
        GtkWidget *handlebox2;
        GtkWidget *menutop;
        GtkWidget *toolbar;
	GtkWidget *top;
        GtkWidget *scrolled; 
        GtkWidget *ctree;
        GtkWidget **menu;
        GtkWidget *menu_item;
	GtkCTreeNode *root;
	gchar *label[COLUMNS];
	gchar *titles[COLUMNS];
	char *icon_name;
	entry *en;
	cfg *win;
	GtkAccelGroup *accel;
	int i;

	menu_entry dir_mlist[] = {
		{ N_("Open in new"),		(void *) cb_new_window,		0},
		{ NULL,				NULL,			0},
		{ N_("New Folder ..."),		(void *) cb_new_subdir,		0, GDK_n, GDK_MOD1_MASK},
		{ N_("New file ..."),		(void *) cb_new_file,		0, GDK_k, GDK_MOD1_MASK},
		{ NULL,				NULL,			0},
		{ N_("Rename ..."),		(void *) cb_rename,		0, GDK_r, GDK_MOD1_MASK},
		{ N_("Delete ..."),		(void *) cb_delete,		0, GDK_x, GDK_CONTROL_MASK},
		{ NULL,				NULL,			0},
		{ N_("Go to ..."),		(void *) cb_go_to,		0},
		{ N_("Go to marked"),		(void *) cb_go_to,		0, GDK_g, GDK_MOD1_MASK},
		{ N_("Go up"),			(void *) cb_go_up,		0, GDK_u, GDK_MOD1_MASK},
		{ N_("Go home"),		(void *) cb_go_home,		0, GDK_h, GDK_MOD1_MASK},
		{ NULL,				NULL,			0},
		{ N_("Unselect"),		(void *) cb_unselect,		0, GDK_a, GDK_SHIFT_MASK|GDK_MOD1_MASK},
		{ N_("Select all"),		(void *) cb_select,		0, GDK_a, GDK_MOD1_MASK},
		{ N_("Unselect all"),		(void *) cb_unselect,		0},
		{ N_("Find ..."),		(void *) cb_find,		0, GDK_f, GDK_MOD1_MASK},
		{ N_("Toggle Dotfiles"),	(void *) on_dotfiles,		0, GDK_D, GDK_SHIFT_MASK|GDK_MOD1_MASK},
		{ NULL,				NULL,			0},
		{ N_("Properties ..."),		(void *) cb_props, 		0, GDK_i, GDK_MOD1_MASK},
		{ NULL,				NULL,			0},
		{ N_("Empty Trash"),		(void *) cb_empty_trash,		0, GDK_e, GDK_MOD1_MASK},
		{ N_("Open Trash"),		(void *) cb_open_trash, 		WINCFG, GDK_t, GDK_MOD1_MASK},
		{ N_("About ..."),		(void *) cb_about,		0, GDK_a, GDK_CONTROL_MASK},
		{ NULL,				NULL,			0},
		{ N_("Run program ..."),	(void *) cb_exec, 		WINCFG},
		{ N_("Close window"),		(void *) cb_destroy, 		TOPWIN, GDK_w, GDK_MOD1_MASK},
		{ N_("Quit"),			(void *) gtk_main_quit, 		0, GDK_q, GDK_MOD1_MASK},
	};
	#define LAST_DIR_MENU_ENTRY (sizeof(dir_mlist)/sizeof(menu_entry))

	menu_entry file_mlist[] = {
		{ N_("New window"),		(void *) cb_new_window,		0},
		{ NULL,				NULL,			0},
		{ N_("Open with ..."),		(void *) cb_open_with,		0, GDK_o, GDK_MOD1_MASK},
		{ NULL,				NULL,			0},
		{ N_("Rename ..."),		(void *) cb_rename,		0, GDK_r, GDK_MOD1_MASK},
		{ N_("Delete ..."),		(void *) cb_delete,		0, GDK_x, GDK_CONTROL_MASK},
		{ N_("Duplicate"),		(void *) cb_duplicate,		0, GDK_d, GDK_MOD1_MASK},
		{ NULL,				NULL,			0},
		{ N_("Go to ..."),		(void *) cb_go_to,		0},
		{ N_("Go up"),			(void *) cb_go_up,		0, GDK_u, GDK_MOD1_MASK},
		{ N_("Go home"),		(void *) cb_go_home,		0, GDK_h, GDK_MOD1_MASK},
		{ NULL,				NULL,			0},
		{ N_("Unselect"),		(void *) cb_unselect,		0, GDK_a, GDK_SHIFT_MASK|GDK_MOD1_MASK},
		{ N_("Select all"),		(void *) cb_select,		0, GDK_a, GDK_MOD1_MASK},
		{ N_("Unselect all"),		(void *) cb_unselect,		0},
		{ NULL,				NULL,			0},
		{ N_("Properties ..."),		(void *) cb_props,		0, GDK_i, GDK_MOD1_MASK},
		{ N_("Register ..."),		(void *) cb_register,		0, GDK_j, GDK_MOD1_MASK},
		{ NULL,				NULL,			0},
		{ N_("Empty Trash"),		(void *) cb_empty_trash,		0, GDK_e, GDK_MOD1_MASK},
		{ N_("Open Trash"),		(void *) cb_open_trash, 		WINCFG, GDK_t, GDK_MOD1_MASK},
		{ N_("About ..."),		(void *) cb_about,		0, GDK_a, GDK_CONTROL_MASK},
		{ NULL,				NULL,			0},
		{ N_("Run program ..."),	(void *) cb_exec, 		WINCFG},
		{ N_("Close window"),		(void *) cb_destroy, 		TOPWIN, GDK_w, GDK_MOD1_MASK},
		{ N_("Quit"),			(void *) gtk_main_quit, 		0, GDK_q, GDK_MOD1_MASK},
	};
	#define LAST_FILE_MENU_ENTRY (sizeof(file_mlist)/sizeof(menu_entry))

	menu_entry mixed_mlist[] = {
		{ N_("New windows"),		(void *) cb_new_window,		0},
		{ NULL,				NULL,			0},
		{ N_("Delete ..."),		(void *) cb_delete,		0, GDK_x, GDK_CONTROL_MASK},
		{ NULL,				NULL,			0},
		{ N_("Go to ..."),		(void *) cb_go_to,		0},
		{ N_("Go up"),			(void *) cb_go_up,		0, GDK_u, GDK_MOD1_MASK},
		{ N_("Go home"),		(void *) cb_go_home,		0, GDK_h, GDK_MOD1_MASK},
		{ NULL,				NULL,			0},
		{ N_("Unselect"),		(void *) cb_unselect,		0, GDK_a, GDK_SHIFT_MASK|GDK_MOD1_MASK},
		{ N_("Select all"),		(void *) cb_select,		0, GDK_a, GDK_MOD1_MASK},
		{ N_("Unselect all"),		(void *) cb_unselect,		0},
		{ N_("Find ..."),		(void *) cb_find,		0, GDK_f, GDK_MOD1_MASK},
		{ N_("Toggle Dotfiles"),	(void *) on_dotfiles,		0, GDK_D, GDK_SHIFT_MASK|GDK_MOD1_MASK},
		{ NULL,				NULL,			0},
		{ N_("Empty Trash"),		(void *) cb_empty_trash,		0},
		{ N_("Open Trash"),		(void *) cb_open_trash,		WINCFG},
		{ NULL,				NULL,			0},
		{ N_("Empty Trash"),		(void *) cb_empty_trash,		0, GDK_e, GDK_MOD1_MASK},
		{ N_("Open Trash"),		(void *) cb_open_trash, 		WINCFG, GDK_t, GDK_MOD1_MASK},
		{ N_("About ..."),		(void *) cb_about,		0, GDK_a, GDK_CONTROL_MASK},
		{ NULL,				NULL,			0},
		{ N_("Run program ..."),	(void *) cb_exec, 		WINCFG},
		{ N_("Close window"),		(void *) cb_destroy, 		TOPWIN, GDK_w, GDK_MOD1_MASK},
		{ N_("Quit"),			(void *) gtk_main_quit, 		0, GDK_q, GDK_MOD1_MASK},
	};
	#define LAST_MIXED_MENU_ENTRY (sizeof(mixed_mlist)/sizeof(menu_entry))

	menu_entry none_mlist[] = {
		{ N_("New window"),		(void *) cb_new_window,		0},
		{ NULL,				NULL,			0},
		{ N_("New Folder ..."),		(void *) cb_new_subdir,		0},
		{ N_("New file ..."),		(void *) cb_new_file,		0, GDK_k, GDK_MOD1_MASK},
		{ NULL,				NULL,			0},
		{ N_("Go to ..."),		(void *) cb_go_to,		0},
		{ N_("Go up"),			(void *) cb_go_up,		0, GDK_u, GDK_MOD1_MASK},
		{ N_("Go home"),		(void *) cb_go_home,		0},
		{ NULL,				NULL,			0},
		{ N_("Select all"),		(void *) cb_select,		0, GDK_a, GDK_MOD1_MASK},
		{ N_("Unselect all"),		(void *) cb_unselect,		0},
		{ N_("Toggle Dotfiles"),	(void *) on_dotfiles,		0, GDK_D, GDK_SHIFT_MASK|GDK_MOD1_MASK},
		{ NULL,				NULL,			0},
		{ N_("Find ..."),		(void *) cb_find,		0, GDK_f, GDK_MOD1_MASK},
		{ N_("Empty Trash"),		(void *) cb_empty_trash,		0, GDK_e, GDK_MOD1_MASK},
		{ N_("Open Trash"),		(void *) cb_open_trash, 		WINCFG, GDK_t, GDK_MOD1_MASK},
		{ N_("About ..."),		(void *) cb_about,		0, GDK_a, GDK_CONTROL_MASK},
		{ NULL,				NULL,			0},
		{ N_("Run program ..."),	(void *) cb_exec, 		WINCFG},
		{ N_("Close window"),		(void *) cb_destroy, 		TOPWIN, GDK_w, GDK_MOD1_MASK},
		{ N_("Quit"),			(void *) gtk_main_quit, 		0, GDK_q, GDK_MOD1_MASK},
	};
	#define LAST_NONE_MENU_ENTRY (sizeof(none_mlist)/sizeof(menu_entry))

        /* Set up X error Handler */
        XSetErrorHandler ((XErrorHandler) ErrorHandler);

	win = g_malloc (sizeof(cfg));
	win->dnd_row = -1;
	win->dnd_has_drag =0;
	menu = g_malloc (sizeof (GtkWidget) * MENUS);
	titles[COL_NAME] = _("Name");
	titles[COL_SIZE] = _("Size (bytes)");
	titles[COL_DATE] = _("Last changed (Y-M-D hh:mm)");
	label[COL_NAME] = path;
	label[COL_SIZE] = "";
	label[COL_DATE] = "";
	top = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_signal_connect (GTK_OBJECT(top), "destroy",
			GTK_SIGNAL_FUNC(on_destroy), (void *) win);
	gtk_signal_connect (GTK_OBJECT(top), "delete_event",
			GTK_SIGNAL_FUNC(on_delete), (void *) win);
	top_register (top);

        vbox = gtk_vbox_new (FALSE, 0);
        gtk_container_add (GTK_CONTAINER (top), vbox);
        gtk_widget_show(vbox);

        handlebox1 = gtk_handle_box_new ();
        gtk_box_pack_start (GTK_BOX (vbox), handlebox1, FALSE, FALSE, 0);
        gtk_widget_show (handlebox1);
        
        handlebox2 = gtk_handle_box_new ();
        gtk_box_pack_start (GTK_BOX (vbox), handlebox2, FALSE, FALSE, 0);
        gtk_widget_show (handlebox2);

	scrolled = gtk_scrolled_window_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	ctree = gtk_ctree_new_with_titles (COLUMNS, 0, titles);
	gtk_clist_set_auto_sort (GTK_CLIST(ctree), FALSE);

	accel = gtk_accel_group_new ();
	gtk_accel_group_attach (accel, GTK_OBJECT(top));

	gtk_widget_add_accelerator (GTK_WIDGET(
			GTK_CLIST(ctree)->column[COL_NAME].button), "clicked", accel,
			GDK_n, GDK_CONTROL_MASK|GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET(
			GTK_CLIST(ctree)->column[COL_DATE].button), "clicked", accel,
			GDK_d, GDK_CONTROL_MASK|GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET(
			GTK_CLIST(ctree)->column[COL_SIZE].button), "clicked", accel,
			GDK_s, GDK_CONTROL_MASK|GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

	win->compare = (void *) GTK_CLIST(ctree)->compare;
	win->trash = g_strdup (trash);
	win->xap = g_strdup (xap);
	win->reg = reg;
	win->width = width;
	win->height= height;
	gtk_object_set_user_data (GTK_OBJECT(ctree), win);

	gtk_clist_set_compare_func (GTK_CLIST(ctree), my_compare);
	gtk_clist_set_shadow_type (GTK_CLIST(ctree), GTK_SHADOW_IN);
	gtk_clist_set_column_auto_resize (GTK_CLIST(ctree), 0, TRUE);
	gtk_clist_set_column_resizeable (GTK_CLIST(ctree), 1, TRUE);
	gtk_clist_set_column_resizeable (GTK_CLIST(ctree), 2, TRUE);
        gtk_clist_set_row_height (GTK_CLIST(ctree), 20);
	gtk_clist_set_selection_mode (GTK_CLIST(ctree), GTK_SELECTION_EXTENDED);
	gtk_ctree_set_line_style (GTK_CTREE(ctree), GTK_CTREE_LINES_NONE);
	gtk_ctree_set_expander_style (GTK_CTREE(ctree),GTK_CTREE_EXPANDER_TRIANGLE);
        gtk_clist_set_reorderable (GTK_CLIST(ctree), FALSE);
	gtk_container_add (GTK_CONTAINER(scrolled), ctree);

	menu[MN_DIR] = gtk_menu_new ();
	gtk_menu_set_accel_group (GTK_MENU(menu[MN_DIR]), accel);

	for (i = 0; i < LAST_DIR_MENU_ENTRY; i++) {
		if (dir_mlist[i].label)
			menu_item = gtk_menu_item_new_with_label (_(dir_mlist[i].label));
		else
			menu_item = gtk_menu_item_new ();
		if (dir_mlist[i].func) {
			if (dir_mlist[i].data == WINCFG)
				gtk_signal_connect (GTK_OBJECT(menu_item), "activate",
					GTK_SIGNAL_FUNC(dir_mlist[i].func), win);
			else if (dir_mlist[i].data == TOPWIN)
				gtk_signal_connect (GTK_OBJECT(menu_item), "activate",
					GTK_SIGNAL_FUNC(dir_mlist[i].func), GTK_WIDGET(top));
			else
				gtk_signal_connect (GTK_OBJECT(menu_item), "activate",
					GTK_SIGNAL_FUNC(dir_mlist[i].func), (void*)ctree);
		}
		if (dir_mlist[i].key) {
			gtk_widget_add_accelerator (menu_item, "activate", accel,
				dir_mlist[i].key, dir_mlist[i].mod, GTK_ACCEL_VISIBLE);
		}
		gtk_menu_append (GTK_MENU(menu[MN_DIR]), menu_item);
		gtk_widget_show (menu_item);
	}
	gtk_menu_attach_to_widget(GTK_MENU(menu[MN_DIR]),ctree,(GtkMenuDetachFunc) menu_detach);

	menu[MN_FILE] = gtk_menu_new ();
	gtk_menu_set_accel_group (GTK_MENU(menu[MN_FILE]), accel);
	for (i = 0; i < LAST_FILE_MENU_ENTRY; i++) {
		if (file_mlist[i].label)
			menu_item = gtk_menu_item_new_with_label (_(file_mlist[i].label));
		else
			menu_item = gtk_menu_item_new ();
		if (file_mlist[i].func) {
			if (file_mlist[i].data == WINCFG)
				gtk_signal_connect (GTK_OBJECT(menu_item), "activate",
					GTK_SIGNAL_FUNC(file_mlist[i].func), win);
			else if (file_mlist[i].data == TOPWIN)
				gtk_signal_connect (GTK_OBJECT(menu_item), "activate",
					GTK_SIGNAL_FUNC(file_mlist[i].func), GTK_WIDGET(top));
			else
				gtk_signal_connect (GTK_OBJECT(menu_item), "activate",
					GTK_SIGNAL_FUNC(file_mlist[i].func), (void*)ctree);
		}
		if (file_mlist[i].key) {
			gtk_widget_add_accelerator (menu_item, "activate", accel,
				file_mlist[i].key, file_mlist[i].mod, GTK_ACCEL_VISIBLE);
		}
		gtk_menu_append (GTK_MENU(menu[MN_FILE]), menu_item);
		gtk_widget_show (menu_item);
	}
	gtk_menu_attach_to_widget(GTK_MENU(menu[MN_FILE]),ctree,(GtkMenuDetachFunc) menu_detach);

	menu[MN_MIXED] = gtk_menu_new ();
	gtk_menu_set_accel_group (GTK_MENU(menu[MN_MIXED]), accel);
	for (i = 0; i < LAST_MIXED_MENU_ENTRY; i++) {
		if (mixed_mlist[i].label)
			menu_item = gtk_menu_item_new_with_label (_(mixed_mlist[i].label));
		else
			menu_item = gtk_menu_item_new ();
		if (mixed_mlist[i].func) {
			if (mixed_mlist[i].data == WINCFG)
				gtk_signal_connect (GTK_OBJECT(menu_item), "activate",
					GTK_SIGNAL_FUNC(mixed_mlist[i].func), win);
			else if (mixed_mlist[i].data == TOPWIN)
				gtk_signal_connect (GTK_OBJECT(menu_item), "activate",
					GTK_SIGNAL_FUNC(mixed_mlist[i].func), GTK_WIDGET(top));
			else
				gtk_signal_connect (GTK_OBJECT(menu_item), "activate",
					GTK_SIGNAL_FUNC(mixed_mlist[i].func), (void*)ctree);
		}
		if (mixed_mlist[i].key) {
			gtk_widget_add_accelerator (menu_item, "activate", accel,
				mixed_mlist[i].key, mixed_mlist[i].mod, GTK_ACCEL_VISIBLE);
		}
		gtk_menu_append (GTK_MENU(menu[MN_MIXED]), menu_item);
		gtk_widget_show (menu_item);
	}
	gtk_menu_attach_to_widget(GTK_MENU(menu[MN_MIXED]),ctree,(GtkMenuDetachFunc) menu_detach);

	menu[MN_NONE] = gtk_menu_new ();
	gtk_menu_set_accel_group (GTK_MENU(menu[MN_NONE]), accel);

	for (i = 0; i < LAST_NONE_MENU_ENTRY; i++) {
		if (none_mlist[i].label)
			menu_item = gtk_menu_item_new_with_label (_(none_mlist[i].label));
		else
			menu_item = gtk_menu_item_new ();
		if (none_mlist[i].func) {
			if (none_mlist[i].data == WINCFG)
				gtk_signal_connect (GTK_OBJECT(menu_item), "activate",
					GTK_SIGNAL_FUNC(none_mlist[i].func), win);
			else if (none_mlist[i].data == TOPWIN)
				gtk_signal_connect (GTK_OBJECT(menu_item), "activate",
					GTK_SIGNAL_FUNC(none_mlist[i].func), GTK_WIDGET(top));
			else
				gtk_signal_connect (GTK_OBJECT(menu_item), "activate",
					GTK_SIGNAL_FUNC(none_mlist[i].func), (void*)ctree);
		}
		if (none_mlist[i].key) {
			gtk_widget_add_accelerator (menu_item, "activate", accel,
				none_mlist[i].key, none_mlist[i].mod, GTK_ACCEL_VISIBLE);
		}
		gtk_menu_append (GTK_MENU(menu[MN_NONE]), menu_item);
		gtk_widget_show (menu_item);
	}
	gtk_menu_attach_to_widget(GTK_MENU(menu[MN_NONE]),ctree,(GtkMenuDetachFunc) menu_detach);

	root = gtk_ctree_insert_node (GTK_CTREE(ctree),
				NULL, NULL, label, 8,
				gPIX_dir_close, gPIM_dir_close, gPIX_dir_open, gPIM_dir_open,
				FALSE, TRUE);
	en = entry_new_by_path_and_label (path, path);
	if (!en) {
		exit (1);
	}
	en->flags= flags;

	gtk_ctree_node_set_row_data_full (GTK_CTREE(ctree), root, en, node_destroy);
	add_subtree (GTK_CTREE(ctree), root, path, 2, flags);


	gtk_signal_connect (GTK_OBJECT(ctree), "tree_expand",
			GTK_SIGNAL_FUNC(on_expand), path);
	gtk_signal_connect (GTK_OBJECT(ctree), "tree_collapse",
			GTK_SIGNAL_FUNC(on_collapse), path);
	gtk_signal_connect (GTK_OBJECT(ctree), "click_column",
			GTK_SIGNAL_FUNC(on_click_column), en);
	gtk_signal_connect_after (GTK_OBJECT(ctree), "button_press_event",
			GTK_SIGNAL_FUNC(on_double_click), root);
	gtk_signal_connect (GTK_OBJECT(ctree), "button_press_event",
			GTK_SIGNAL_FUNC(on_button_press), menu);
	gtk_signal_connect (GTK_OBJECT(ctree), "key_press_event",
			GTK_SIGNAL_FUNC(on_key_press), menu);
	gtk_signal_connect (GTK_OBJECT(ctree), "drag_data_received",
			GTK_SIGNAL_FUNC(on_drag_data), win);
	gtk_signal_connect (GTK_OBJECT(ctree), "drag_data_get",
			GTK_SIGNAL_FUNC(on_drag_data_get), win);
	set_title (top, en->path);
	if (win->width > 0 && win->height > 0) {
		gtk_window_set_default_size (GTK_WINDOW(top), width, height);
	}

	win->timer = gtk_timeout_add (TIMERVAL, (GtkFunction) update_timer, ctree);
	gtk_widget_show_all (top);
	gtk_drag_source_set (ctree, GDK_BUTTON1_MASK|GDK_BUTTON2_MASK|GDK_BUTTON2_MASK, target_table, NUM_TARGETS, GDK_ACTION_MOVE|GDK_ACTION_COPY|GDK_ACTION_LINK);
	gtk_drag_dest_set (ctree, GTK_DEST_DEFAULT_ALL, target_table, NUM_TARGETS, GDK_ACTION_MOVE|GDK_ACTION_COPY|GDK_ACTION_LINK);


        menutop = create_menu(top, ctree, win);
        gtk_container_add (GTK_CONTAINER (handlebox1), menutop);
        gtk_widget_show (menutop);  

        toolbar = create_toolbar(top, ctree, win);
        gtk_container_add (GTK_CONTAINER (handlebox2), toolbar);
        gtk_widget_show (toolbar);  

 	icon_name = strrchr(path, '/');
	if ((icon_name) && (!(*(++icon_name)))) icon_name = NULL;
	set_icon (top, (icon_name ? icon_name : "/"), xftree_icon_xpm);
	return (top);
}

/*
 * create pixmaps and create a new toplevel tree widget
 */
void
gui_main (char *path, char *xap_path, char *trash, char *reg_file, wgeo_t *geo,
			int flags)
{
	GList *reg;
	GtkWidget *top, *new_win;

	XFCE_palette *pal;

	pal = newpal ();
	defpal (pal);
	loadpal (pal);
	applypal (pal, NULL);

	global_pal = pal ;

	top = gtk_window_new (GTK_WINDOW_DIALOG);
        gtk_widget_realize (top);
	reg = reg_build_list (reg_file);

	gPIX_page = MyCreateGdkPixmapFromData        (page_xpm, top, &gPIM_page);
	gPIX_page_lnk = MyCreateGdkPixmapFromData    (page_lnk_xpm, top, &gPIM_page_lnk);
	gPIX_dir_pd = MyCreateGdkPixmapFromData      (dir_pd_xpm, top, &gPIM_dir_pd);
	gPIX_dir_open = MyCreateGdkPixmapFromData    (dir_open_xpm, top, &gPIM_dir_open);
	gPIX_dir_open_lnk=MyCreateGdkPixmapFromData  (dir_open_lnk_xpm, top, &gPIM_dir_open_lnk);
	gPIX_dir_close= MyCreateGdkPixmapFromData    (dir_close_xpm, top, &gPIM_dir_close);
	gPIX_dir_close_lnk=MyCreateGdkPixmapFromData (dir_close_lnk_xpm, top, &gPIM_dir_close_lnk);
	gPIX_dir_up = MyCreateGdkPixmapFromData      (dir_up_xpm, top, &gPIM_dir_up);
	gPIX_exe = MyCreateGdkPixmapFromData         (exe_xpm, top, &gPIM_exe);
	gPIX_exe_lnk = MyCreateGdkPixmapFromData     (exe_lnk_xpm, top, &gPIM_exe_lnk);
	gPIX_char_dev = MyCreateGdkPixmapFromData    (char_dev_xpm, top, &gPIM_char_dev);
	gPIX_block_dev = MyCreateGdkPixmapFromData   (block_dev_xpm, top, &gPIM_block_dev);
	gPIX_fifo = MyCreateGdkPixmapFromData        (fifo_xpm, top, &gPIM_fifo);
	gPIX_socket = MyCreateGdkPixmapFromData      (socket_xpm, top, &gPIM_socket);
	gPIX_stale_lnk = MyCreateGdkPixmapFromData   (stale_lnk_xpm, top, &gPIM_stale_lnk);

	if (!io_is_directory(path)) {
		dlg_error(path, strerror(errno));
		return;
	}
	new_win = new_top (path, xap_path, trash, reg,geo->width,geo->height,flags);
	if (geo->x > -1 && geo->y > -1) {
		gtk_widget_set_uposition (new_win, geo->x, geo->y);
	}

	reg_xfce_app(new_win, pal);
	gtk_widget_destroy (top);
	gtk_main ();
}


