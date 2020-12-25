/*  gxfce
 *  Copyright (C) 1999 Olivier Fourdan (fourdan@csi.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/



#ifndef __CONFIGFILE_H__
#define __CONFIGFILE_H__

typedef struct 
  {
	int panel_x;
	int panel_y;
	int wm;
	int visible_screen;
	int visible_popup;
	int select_icon_size;
	int popup_icon_size;
	int colorize_root;
	int gradient_root;
	int detach_menu;
	int clicktofocus;
	int opaquemove;
	int autoraise;
	int gradient_active_title;
	int gradient_inactive_title;
	int iconpos;
	char *fonts[3];
  } 
config;

config current_config;

extern char *skiphead (char *);
extern char *skiptail (char *);
extern config *initconfig(config *newconf);
extern void backupconfig (char *extension);
extern void writeconfig (void);
extern void resetconfig (void);
extern void readconfig (void);
extern void update_config_screen_visible(int);

#endif
