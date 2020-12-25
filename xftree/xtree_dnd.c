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
	GtkCTreeNode *node;
	uri *u;
	int nitems, action; 
	int mode = 0;
	int source_state;
	char msg[DLG_MAX], *string;
	int row, col;
	GtkCList *clist = GTK_CLIST(ctree);

	if ((data->length < 0) || (data->format != 8)) {
		gtk_drag_finish (context, FALSE, FALSE, time);
		return;
	}

	action = context->action <= GDK_ACTION_DEFAULT ?
				GDK_ACTION_COPY : context->action;

	row = col = -1;
	y -= clist->column_title_area.height;
	gtk_clist_get_selection_info (clist, x, y, &row, &col);
	/* initialize row pointer */
	win->dnd_row = 0;

	if (row >= 0) {
		node = gtk_ctree_node_nth (GTK_CTREE(ctree), row);
		en = gtk_ctree_node_get_row_data(GTK_CTREE(ctree), node);
		if ((((en->type & FT_DIR) && (access(en->path, W_OK|X_OK) ==0))  ||
			((en->type & FT_FILE) && (access(en->path, W_OK) ==0))))
		  win->dnd_row = row;
		else
		  win->dnd_row = 0;
		}
	else if (win->dnd_row >=0)
			win->dnd_row = 0;
			
	switch (info) {
		case TARGET_XTREE_WIDGET:
		case TARGET_XTREE_WINDOW:
		case TARGET_PLAIN:
		case TARGET_STRING:
		case TARGET_URI_LIST:
			if (action == GDK_ACTION_MOVE) {
				string = _("move");
				mode = TR_MOVE;
			} else if (action == GDK_ACTION_COPY) {
				string = _("copy");
				mode = TR_COPY;
			} else if (action == GDK_ACTION_LINK) {
				string = _("link");
				mode = TR_LINK;
			} else {
				dlg_error (_("Unknown action !"), NULL);
				gtk_drag_finish (context, FALSE, FALSE, time);
				return;
			}
			nitems = uri_parse_list ((guchar *) data->data, &list);
			uri_remove_file_prefix_from_list (list);
			node = gtk_ctree_node_nth (GTK_CTREE(ctree), win->dnd_row);
			en = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), node);
			if (!(en->type & FT_DIR))
			{
			  node = GTK_CTREE_ROW (node)->parent;
			  en = gtk_ctree_node_get_row_data (GTK_CTREE(ctree), node);
			}
			if ((!(en->type & FT_DIR)) || (en->type & FT_DIR_UP) || (!io_is_valid(en->label))) {
				dlg_error (_("Target must be a directory !"), NULL);
				gtk_drag_finish (context, FALSE, (mode == TR_MOVE) , time);
				uri_free_list (list);
				return ;
			}
			
			sprintf (msg, _("Do you want to %s the item(s) to"), string);
			if (dlg_question(msg, en->path) != DLG_RC_OK) {
				gtk_drag_finish (context, FALSE, (mode == TR_MOVE), time);
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
			break;
		default:
			break;
	}
	gtk_drag_finish (context, TRUE, (mode == TR_MOVE), time);
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
