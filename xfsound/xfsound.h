/*

   ORIGINAL FILE NAME : xfsound.h

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





#ifndef FD_xfsound_h_
#define FD_xfsound_h_

#include "../xfce/constant.h"
#include "../xfce/fileutil.h"
#include "../xfce/my_string.h"
#include "../xfce/sendinfo.h"
#include "../xfce/color.h"
#include "../xfce/my_goodies.h"

#include "../xfwm/configure.h"
#include "../xfwm/module.h"

#define KNOWN_MESSAGES          11
#define BUILTIN_STARTUP		KNOWN_MESSAGES
#define BUILTIN_SHUTDOWN	KNOWN_MESSAGES+1
#define BUILTIN_UNKNOWN		KNOWN_MESSAGES+2
#define KNOWN_BUILTIN		3

#define INTERNAL_PLAYER "internal"
#define DEFAULT_PLAYER  "xfplay 2>/dev/null"

#define BUTTONLEN	16
#define TITLELEN	64
#define TEXTLEN 	128

typedef struct
{
 	int   playsnd;
	char *playcmd;
	char *datafiles[KNOWN_MESSAGES + KNOWN_BUILTIN];
} XFSound;

typedef struct
{
	long  message_id;
        char *label;
} T_messages;

typedef struct
  {
    char *xfsoundtitle;
    char *oklabel;
    char *browselabel;
    char *applylabel;
    char *eventlabel;
    char *cancellabel;
    char *testlabel;
    char *commandlabel;
    char *playsndlabel;
    char *sndfilelabel;
    char *tinyfont;
    char *regularfont;
    char *boldfont;
    char *bigfont;
    char *titlefont;
  }
res_XFSound;

extern XFSound sndcfg;
extern T_messages messages[];
extern res_XFSound rxfsnd;

void allocXFSound (XFSound *);
void freeXFSound (XFSound *);
void loadcfg (XFSound *);
int  savecfg (XFSound *);
void audio_play(short);

/** Callbacks, globals and object handlers **/
void show_fd_xfsound (char *);

extern void doplay_cb(FL_OBJECT *, long);
extern void play_com_cb(FL_OBJECT *, long);
extern void eventlist_cb(FL_OBJECT *, long);
extern void soundfile_cb(FL_OBJECT *, long);
extern void browsefile_cb(FL_OBJECT *, long);
extern void internal_cb(FL_OBJECT *, long);
extern void defaultcmd_cb(FL_OBJECT *, long);
extern void ok_cb(FL_OBJECT *, long);
extern void apply_cb(FL_OBJECT *, long);
extern void cancel_cb(FL_OBJECT *, long);
extern void testfile_cb(FL_OBJECT *, long);


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *xfsound;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *test;
	FL_OBJECT *doplay;
	FL_OBJECT *play_com;
	FL_OBJECT *eventlist;
	FL_OBJECT *soundfile;
	FL_OBJECT *browsefile;
	FL_OBJECT *internal;
	FL_OBJECT *defaultcmd;
	FL_OBJECT *ok;
	FL_OBJECT *apply;
	FL_OBJECT *cancel;
} FD_xfsound;

extern FD_xfsound * create_form_xfsound(void);

extern FD_xfsound *fd_xfsound;

#endif /* FD_xfsound_h_ */
