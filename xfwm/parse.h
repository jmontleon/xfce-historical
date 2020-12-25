


/****************************************************************************
 * This module was originally based on the twm module of the same name. 
 * Since its use and contents have changed so dramatically, I have removed
 * the original twm copyright, and inserted my own.
 *
 * by Rob Nation 
 * Copyright 1993 Robert Nation. No restrictions are placed on this code,
 * as long as the copyright notice is preserved
 ****************************************************************************/

/**********************************************************************
 *
 * Codes for xfwm builtins 
 *
 **********************************************************************/

#ifndef _PARSE_
#define _PARSE_

#define F_NOP			0
#define F_BEEP			1
#define F_QUIT			2
#define F_RESTART               3
#define F_REFRESH		4
#define F_TITLE			5
#define F_CIRCULATE_UP          6
#define F_CIRCULATE_DOWN        7
#define F_WINDOWLIST            8 
#define F_MOVECURSOR            9 
#define F_FUNCTION              10
#define F_MODULE                11
#define F_DESK                  12
#define F_CHANGE_WINDOWS_DESK   13
#define F_EXEC			14	/* string */
#define F_POPUP			15	/* string */
#define F_WAIT                  16
#define F_CLOSE                 17
#define F_SET_MASK              18
#define F_ADDMENU               19
#define F_ADDFUNC               20
#define F_STYLE                 21
#define F_PIXMAP_PATH           22
#define F_ICON_PATH             23
#define F_MODULE_PATH           24
#define F_HICOLOR               25
#define F_LOCOLOR               26
#define F_SETDESK               27 
#define F_MOUSE                 28
#define F_KEY                   29
#define F_OPAQUE                30
#define F_XOR                   31
#define F_CLICK                 32
#define F_MENUCOLOR             33
#define F_ICONFONT              34
#define F_WINDOWFONT            35
#define F_BUTTON_STYLE          36
#define F_READ                  37
#define F_ADDMENU2              38
#define F_NEXT                  39
#define F_PREV                  40
#define F_NONE                  41
#define F_STAYSUP	        42	/* string */
#define F_RECAPTURE             43
#define F_CONFIG_LIST	        44
#define F_DESTROY_MENU	        45
#define F_ZAP	                46
#define F_QUIT_SCREEN		47
#define F_COLORMAP_FOCUS        48
#define F_TITLESTYLE            49
#define F_EXEC_SETUP            50
#define F_CURSOR_STYLE          51
#define F_CURRENT               52
#define F_CURSOR_COLOR          53
#define F_MENUFONT              54
#define F_SETFOCUSMODE          55
#define F_ANIMATE               56
#define F_OPAQUEMOVE            57
#define F_AUTORAISE             58
#define F_AUTORAISEDELAY        59
#define F_ICONPOS               60
#define F_ARRANGEICONS          61
#define F_SNAPSIZE		62
#define F_OPAQUERESIZE          63

/* Functions which require a target window */
#define F_RESIZE		100
#define F_RAISE			101
#define F_LOWER			102
#define F_DESTROY		103
#define F_DELETE		104
#define F_MOVE			105
#define F_ICONIFY		106
#define F_STICK                 107
#define F_RAISELOWER            108
#define F_MAXIMIZE              109
#define F_SHADE                 109
#define F_FOCUS                 110
#define F_WARP                  111
#define F_SEND_STRING           112
#define F_ADD_MOD               113
#define F_DESTROY_MOD           114
#define F_FLIP_FOCUS            115
#define F_ECHO                  116
#define F_GLOBAL_OPTS           118
#define F_WINDOWID              119
#define F_ADD_BUTTON_STYLE      120
#define F_ADD_TITLE_STYLE       121
#define F_WINDOW_SHADE          125

/* Functions for use by modules only! */
#define F_SEND_WINDOW_LIST     1000

/* Functions for internal  only! */
/* #define F_RAISE_IT              2000 */

#endif /* _PARSE_ */
