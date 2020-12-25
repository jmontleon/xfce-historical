/** Header file generated with fdesign on Wed Mar  3 17:18:23 1999.**/

#ifndef FD_xfmouse_h_
#define FD_xfmouse_h_

#include "../xfce/constant.h"
#include "../xfce/fileutil.h"
#include "../xfce/my_string.h"
#include "../xfce/sendinfo.h"
#include "../xfce/color.h"
#include "../xfce/my_goodies.h"

#define BUTTONLEN	16
#define TITLELEN	64
#define TEXTLEN 	128

#define ACCEL_MIN	1
#define ACCEL_MAX	30
#define DENOMINATOR	3
#define THRESH_MIN	1
#define THRESH_MAX	20

typedef struct
  {
    char *xfmousetitle;
    char *oklabel;
    char *applylabel;
    char *cancellabel;
    char *buttonlabel;
    char *motionlabel;
    char *leftlabel;
    char *rightlabel;
    char *accelerationlabel;
    char *thresholdlabel;
    char *tinyfont;
    char *regularfont;
    char *boldfont;
    char *bigfont;
    char *titlefont;
  }
res_XFMouse;

typedef struct
{
 	int   button;
	int   accel;
	int   thresh;
} XFMouse;

extern res_XFMouse   rxfmouse;

extern XFMouse mouseval;

extern void show_fd_xfmouse(char *, XFMouse *);
extern void mouse_values(XFMouse *);

extern void loadcfg (XFMouse *);
extern int  savecfg (XFMouse *);

/** Callbacks, globals and object handlers **/
extern void rightbtn_cb(FL_OBJECT *, long);
extern void leftbtn_cb(FL_OBJECT *, long);
extern void accel_cb(FL_OBJECT *, long);
extern void thresh_cb(FL_OBJECT *, long);
extern void ok_cb(FL_OBJECT *, long);
extern void apply_cb(FL_OBJECT *, long);
extern void cancel_cb(FL_OBJECT *, long);


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *xfmouse;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *rightbtn;
	FL_OBJECT *leftbtn;
	FL_OBJECT *accel;
	FL_OBJECT *thresh;
	FL_OBJECT *ok;
	FL_OBJECT *apply;
	FL_OBJECT *cancel;
} FD_xfmouse;

extern FD_xfmouse * create_form_xfmouse(void);
extern FD_xfmouse *fd_xfmouse;

#endif /* FD_xfmouse_h_ */
