/*

   ORIGINAL FILE NAME : xfwm_cmd.h

   ********************************************************************
   *                                                                  *
   *           X F C E  - Written by O. Fourdan (c) 1997              *
   *                                                                  *
   *           This software is absolutely free of charge             *
   *                                                                  *
   ********************************************************************

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHOR (O. FOURDAN) BE 
   LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN 
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
 */

#ifndef __XFWM_CMD_H__
#define __XFWM_CMD_H__

/*
   Define some commands to send to XFwm (or FVWM) 
 */

#define MOVE_CMD	  "Move"
#define DESK_CMD	  "Desk 0"
#define QUIT_CMD	  "Quit"
#define ICON_CMD	  "Iconify"
#define NOP_CMD	          "Nop"
#define FOCUSCLICK_CMD	  "FocusMode ClickToFocus"
#define FOCUSMOUSE_CMD	  "FocusMode FollowMouse"
#define OPAQUEMOVEON_CMD  "OpaqueMove On"
#define OPAQUEMOVEOFF_CMD "OpaqueMove Off"
#define AUTORAISEON_CMD   "AutoRaise On"
#define AUTORAISEOFF_CMD  "AutoRaise Off"

#endif
