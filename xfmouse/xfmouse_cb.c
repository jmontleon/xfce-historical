#include "forms.h"
#include "xfmouse.h"
#include <X11/Xlib.h>

void
show_fd_xfmouse(char *title, XFMouse *s)
{
  fl_set_button (fd_xfmouse->rightbtn, (s->button));
  fl_set_button (fd_xfmouse->leftbtn, !(s->button));
  fl_set_slider_value (fd_xfmouse->accel,  s->accel);
  fl_set_slider_value (fd_xfmouse->thresh, s->thresh);
  fl_show_form(fd_xfmouse->xfmouse,FL_PLACE_CENTER + FL_FIX_SIZE, FL_FULLBORDER, title);
  fl_set_cursor (fd_xfmouse->xfmouse->window, XFCE_CURS);
}


void 
mouse_values(XFMouse *s)
{
   unsigned char map[5];
   
   XChangePointerControl(fl_display, 1, 1, s->accel, DENOMINATOR, s->thresh);
   if (s->button) {
     map[0]=1;
     map[1]=2;
     map[2]=3; 
   }
   else {
     map[0]=3;
     map[1]=2;
     map[2]=1; 
   }
   XSetPointerMapping(fl_display, map, 3);
}

void 
apply_mouse_values(XFMouse *s)
{
   s->button = (int) fl_get_button (fd_xfmouse->rightbtn);
   s->accel  = (int) fl_get_slider_value (fd_xfmouse->accel);
   s->thresh = (int) fl_get_slider_value (fd_xfmouse->thresh);
   mouse_values ( s );
}


/* callbacks and freeobj handles for form xfmouse */
void rightbtn_cb(FL_OBJECT *ob, long data)
{
   fl_set_button (fd_xfmouse->leftbtn, 0);
}

void leftbtn_cb(FL_OBJECT *ob, long data)
{
   fl_set_button (fd_xfmouse->rightbtn, 0);
}

void accel_cb(FL_OBJECT *ob, long data)
{
}

void thresh_cb(FL_OBJECT *ob, long data)
{
}


void ok_cb(FL_OBJECT *ob, long data)
{
   apply_mouse_values(&mouseval);
   savecfg (&mouseval);
   fl_finish ();
   exit (0);
}

void apply_cb(FL_OBJECT *ob, long data)
{
   apply_mouse_values(&mouseval);
}

void cancel_cb(FL_OBJECT *ob, long data)
{
   fl_finish ();
   exit (0);
}



