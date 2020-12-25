/*
 * gtk_cpy.h
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

#ifndef __GTK_CPY_H__
#define __GTK_CPY_H__

#define TR_COPY		1
#define TR_MOVE		2
#define TR_LINK		4
#define TR_OVERRIDE	8

int transfer (GtkCTree * s, GList * list, char *target, int mode, int *alive);
void transfer_view (GtkWidget *, int *);
#endif
