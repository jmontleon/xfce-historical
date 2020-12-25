/*
 * xtree_dnd.c
 *
 * Copyright (C) 1998 Rasca, Berlin
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
#include <string.h>
#include <gtk/gtk.h>
#include "my_intl.h"
#include "xtree_cfg.h"
#include "xtree_dnd.h"
#include "xtree_misc.h"
#include "entry.h"
#include "uri.h"
#include "io.h"
#include "gtk_dlg.h"
#include "gtk_get.h"
#include "gtk_cpy.h"
#include "gtk_dnd.h"

/*
 * called form drag source at startup of DND
 */
void
on_drag_begin (GtkWidget *ctree, GdkDragContext *context, void *data)
{
	;
}

/*
 */
gboolean
on_drag_drop (GtkWidget *w, GdkDragContext *context, gint x, gint y,
				guint time, gpointer p)
{
	return (FALSE);
}


/*
 * DND receiver: called if the receiver gets a drag_leave-event
 */
void
on_drag_leave (GtkWidget *ctree, GdkDragContext *context, guint time, void *p)
{
	cfg *win = (cfg *)p;
	win->dnd_has_drag = 0;
}

/*
 * DND receiver: called during motion
 */
gboolean
on_drag_motion (GtkWidget *widget, GdkDragContext *context,
				gint x, gint y, guint time, void *data)
{
	GtkCList *clist = GTK_CLIST(widget);
	int row, col;
	GtkCTreeNode *node;
	entry *en;
	cfg *win = (cfg *)data;


	row = col = -1;
	y -= clist->column_title_area.height;
	gtk_clist_get_selection_info (clist, x, y, &row, &col);
	/* initialize row pointer */
	if (!win->dnd_has_drag)
		win->dnd_row = -1;

	if (row >= 0) {
		if (row == win->dnd_row) {
			return (TRUE);
		}
		node = gtk_ctree_node_nth (GTK_CTREE(widget), row);
		en = gtk_ctree_node_get_row_data(GTK_CTREE(widget), node);
		if ((((en->type & FT_DIR) && (access(en->path, W_OK|X_OK) ==0))  ||
			((en->type & FT_FILE) && (access(en->path, W_OK) ==0))))
		  win->dnd_row = row;
		else
		  win->dnd_row = -1;
		}
	else if (win->dnd_row >=0)
			win->dnd_row = -1;
	win->dnd_has_drag = 1;
	/* gdk_drag_status (context, context->suggested_action, time); */
	return (TRUE);
}


/*
 * called if drop data will be received
 * signal: drag_data_received
 */
void
on_drag_data (GtkWidget *ctree, GdkDragContext *context, gint x, gint y,
				GtkSelectionData *data, guint info, guint time, void *client)
{
	cfg *win = (cfg *)client;
	GList *list, *t, *tmp;
	entry *en, *s_en;
	GtkCTree *s_ctree;
	GtkCTreeNode *node;
	uri *u;
	int nitems, action, mode, source_state;
	guint handler;
	char msg[DLG_MAX], *string, *target;
	char *title;
	win->dnd_has_drag = 0;

	if ((data->length <= 0) || (data->format != 8)) {
		gtk_drag_finish (context, FALSE, TRUE, time);
		return;
	}
	/* find target row
	 */
	if (win->dnd_row < 0) {
		win->dnd_row = 0;
	}
	action = context->action <= GDK_ACTION_DEFAULT ?
				GDK_ACTION_COPY : context->action;
	switch (info) {
		case TARGET_XTREE_WIDGET:
		case TARGET_XTREE_WINDOW:
			/* we transfer only the number of marked entries
			 * and the address of the ctree widget
			 */
			sscanf (data->data, "%d %d", &nitems, (int*)&s_ctree);
			/* find target
			 */
			node = gtk_ctree_node_nth (GTK_CTREE(ctree), win->dnd_row);
			en   = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), node);
			if (((nitems > 1) && (!(EN_IS_DIR(en)))) || (en->type & FT_DIR_UP) || (!io_is_valid(en->label)))
			{
				dlg_error (_("Target must be a directory !"), NULL);
				gtk_drag_finish (context, FALSE, TRUE, time);
				return ;
			}
			/* assume that at this point the source window is still alive
			 */
			source_state = 1;
			handler = gtk_signal_connect (GTK_OBJECT(s_ctree), "destroy",
						GTK_SIGNAL_FUNC(transfer_view), &source_state);

			if (entry_list_from_selection (s_ctree, &list, &source_state)
					!= nitems) {
				dlg_error (_("Error in 'DnD' ?!"), _("Wrong number of items"));
				gtk_drag_finish (context, FALSE, TRUE, time);
				gtk_signal_disconnect (GTK_OBJECT(s_ctree), handler);
				return;
			}
			else if (entry_list_from_selection (s_ctree, &list, &source_state) == 0)
			{
				gtk_drag_finish (context, FALSE, TRUE, time);
				gtk_signal_disconnect (GTK_OBJECT(s_ctree), handler);
				return;
			}
			switch (action) {
				case GDK_ACTION_COPY:
					title = _("Copy item(s) to ?");
					mode = TR_COPY;
					break;
				case GDK_ACTION_MOVE:
					title = _("Move item(s) to ?");
					mode = TR_MOVE;
					break;
				default:
					fprintf (stderr,
						_("on_drag_data() action not supported (=%d)\n"),
						action);
					gtk_drag_finish (context, FALSE, TRUE, time);
   				        gtk_signal_disconnect (GTK_OBJECT(s_ctree), handler);
					return;
					break;
			}
			t = list;
			while (t) {
				if (strcmp (((entry *)t->data)->path, en->path) == 0) {
					/* free the list */
					entry_list_free (list);
					/* source and target are the same
					 * return silently
					 */
					gtk_drag_finish (context, FALSE, TRUE, time);
					gtk_signal_disconnect (GTK_OBJECT(s_ctree), handler);
					return;
				}
				t = t->next;
			}
			if (dlg_question (title , en->path) != DLG_RC_OK) {
				gtk_drag_finish (context, FALSE, TRUE, time);
				gtk_signal_disconnect (GTK_OBJECT(s_ctree), handler);
				return;
			}
			target = g_strdup (en->path);
			gtk_drag_finish (context, TRUE, TRUE, time);
			if (g_list_length (list)) transfer (s_ctree, list, target, mode, &source_state);
			if (source_state)
				gtk_signal_disconnect (GTK_OBJECT(s_ctree), handler);
			g_free (target);
			return;
			break;
		case TARGET_PLAIN:
		case TARGET_STRING:
		case TARGET_URI_LIST:
			nitems = uri_parse_list (data->data, &list);
			uri_remove_file_prefix_from_list (list);
			node = gtk_ctree_node_nth (GTK_CTREE(ctree), win->dnd_row);
			en = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), node);
			if (((nitems > 1) && (!(en->type & FT_DIR))) || (en->type & FT_DIR_UP) || (!io_is_valid(en->label))) {
				dlg_error (_("Target must be a directory !"), NULL);
				gtk_drag_finish (context, FALSE, TRUE, time);
				uri_free_list (list);
				return ;
			}
			if (action == GDK_ACTION_MOVE) {
				string = _("move");
				mode = TR_MOVE;
			} else if (action == GDK_ACTION_COPY) {
				string = _("copy");
				mode = TR_COPY;
			} else {
				dlg_error (_("Unknown action !"), NULL);
				gtk_drag_finish (context, FALSE, TRUE, time);
				uri_free_list (list);
				return;
			}
			sprintf (msg, _("Do you want to %s the item(s) to"), string);
			if (dlg_question(msg, en->path) != DLG_RC_OK) {
				gtk_drag_finish (context, FALSE, TRUE, time);
				uri_free_list (list);
				return;
			}
			t = list;
			while (t) {
				u = t->data;
				if ((u->type & URI_FTP) || (u->type & URI_HTTP)) {
					if (!download (u, en->path))
						break;
				} else if (u->type == URI_LOCAL) {
					/* printf (" %s -> %s\n", u->url, en->path); */
					tmp = NULL;
					s_en = entry_new_by_path (u->url);
					if (!s_en) {
						perror (u->url);
						t = t->next;
						continue;
					}
					tmp = g_list_append (tmp, s_en);
					transfer (NULL, tmp, en->path, mode, &source_state);
				} else {
					fprintf (stderr, _("Type not supported.. (%d)\n"), u->type);
				}
				t = t->next;
			}
			uri_free_list (list);
			gtk_drag_finish (context, TRUE, TRUE, time);
			break;
		default:
			break;
	}
	if (source_state) gtk_drag_finish (context, FALSE, TRUE, time);
}

/*
 * DND sender: prepare data for the remote
 * event: drag_data_get
 */
void
on_drag_data_get (GtkWidget *widget, GdkDragContext *context,
					GtkSelectionData *selection_data,
					guint info,
					guint time,
					gpointer data)
{
	GtkCTreeNode *node;
	GtkCTree *ctree = GTK_CTREE(widget);
	GList *selection;
	entry *en;
	int num, i, len, slen;
	gchar *files;
	cfg *win = (cfg *)data;

	num = g_list_length (GTK_CLIST(ctree)->selection);
	if (!num) {
		/* printf (_("Nothing selected ?  FIXME!\n")); */
		node = GTK_CTREE_NODE(GTK_CLIST(ctree)->row_list);
	} else
		node = GTK_CTREE_NODE(GTK_CLIST(ctree)->selection->data);

	/* prepare data for the receiver
	 */
	switch (info) {
		case TARGET_XTREE_WIDGET:
			/* check if target == source */
			selection = GTK_CLIST(ctree)->selection;
			while (selection) {
				selection = selection -> next;
			}
			win->dnd_data = g_malloc (32);
			sprintf (win->dnd_data, "%d %d", num, (int)ctree);
			gtk_selection_data_set (selection_data,
					selection_data->target, 8, win->dnd_data, 32);
			break;
		case TARGET_XTREE_WINDOW:
			win->dnd_data = g_malloc (32);
			sprintf (win->dnd_data, "%d %d", num, (int)ctree);
			gtk_selection_data_set (selection_data,
					selection_data->target, 8, win->dnd_data, 32);
			break;
		case TARGET_ROOTWIN:
			/* not implemented */
			win->dnd_data = NULL;
			break;
		default:
			selection = GTK_CLIST(ctree)->selection;
			for (len = 0, i = 0; i < num; i++) {
				node = selection->data;
				en = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), node);
				len += strlen (en->path) + 5 + 2;
				selection = selection->next;
			}
			win->dnd_data = files = g_malloc (len+1);
			files[0] = '\0';
			selection = GTK_CLIST(ctree)->selection;
			for (i = 0; i < num; i++) {
				node = selection->data;
				en = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), node);
				slen = strlen (en->path);
				sprintf (files, "file:%s\r\n", en->path);
				files += slen + 5 + 2;
				selection = selection->next;
			}
			gtk_selection_data_set (selection_data,
					selection_data->target, 8, win->dnd_data, len);
			break;
	}
}

/*
 */
void
on_drag_data_delete (GtkWidget *widget, GdkDragContext *context, gpointer *data)
{
	cfg *win = (cfg *)data;
	g_free (win->dnd_data);
}
