#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

/***************************************************************************
 * Please translate the strings into the language which you use for 
 * your pop-up menus.
 *
 * Some decisions about where a function is prohibited (based on 
 * mwm-function-hints) is based on a string comparison between the 
 * menu item and the strings below.
 ***************************************************************************/
#define MOVE_STRING "move"
#define RESIZE_STRING1 "size"
#define RESIZE_STRING2 "resize"
#define MINIMIZE_STRING "minimize"
#define MINIMIZE_STRING2 "iconify"
#define MAXIMIZE_STRING "maximize"
#define CLOSE_STRING1 "close"
#define CLOSE_STRING2 "delete"
#define CLOSE_STRING3 "destroy"
#define CLOSE_STRING4 "quit"

/* #ifdef __alpha */
#if defined(__alpha) && !defined(linux)
#define NEEDS_ALPHA_HEADER
#undef BROKEN_SUN_HEADERS
#endif /* (__alpha) */

/* end of configure.h */
