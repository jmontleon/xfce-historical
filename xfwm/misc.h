#ifndef _MISC_
#define _MISC_

#include <ctype.h>
#include "menus.h"
#include "utils.h"

/************************************************************************
 * ReapChildren - wait() for all dead child processes
 ************************************************************************/
#include <sys/wait.h>
#ifdef HAVE_WAITPID
#define ReapChildren()  while ((waitpid(-1, NULL, WNOHANG)) > 0);
#else
#define ReapChildren()  while ((wait3(NULL, WNOHANG, NULL)) > 0);
#endif

typedef struct name_list_struct
  {
    struct name_list_struct *next;	/* pointer to the next name */
    char *name;			/* the name of the window */
    char *value;		/* icon name */
    int Desk;			/* Desktop number */
    unsigned long on_flags;
    unsigned long off_flags;
    int border_width;
    int resize_width;
    char *ForeColor;
    char *BackColor;
    unsigned long on_buttons;
    unsigned long off_buttons;

  }
name_list;

/* used for parsing configuration */
struct config
  {
    char *keyword;
#ifdef __STDC__
    void (*action) (char *, FILE *, char **, int *);
#else
    void (*action) ();
#endif
    char **arg;
    int *arg2;
  };

/* used for parsing commands */
struct functions
  {
    char *keyword;
#ifdef __STDC__
    void (*action) (XEvent *, Window, XfwmWindow *, unsigned long, char *, int *);
#else
    void (*action) ();
#endif
    int code;
    int type;
  };

/* values for name_list flags */
/* The first 13 items are mapped directly into the XfwmWindow structures
 * flag value, so they MUST correspond to the first 13 entries in xfwm.h */
#define START_ICONIC_FLAG    (1<<0)
#define STAYSONTOP_FLAG      (1<<1)
#define STICKY_FLAG          (1<<2)
#define LISTSKIP_FLAG        (1<<3)
#define SUPPRESSICON_FLAG    (1<<4)
#define NOICON_TITLE_FLAG    (1<<5)
#define LENIENCE_FLAG        (1<<6)
#define STICKY_ICON_FLAG     (1<<7)
#define CIRCULATE_SKIP_ICON_FLAG  (1<<8)
#define CIRCULATESKIP_FLAG   (1<<9)
#define CLICK_FOCUS_FLAG     (1<<10)
#define SLOPPY_FOCUS_FLAG    (1<<11)
#define SHOW_MAPPING         (1<<12)

#define NOTITLE_FLAG         (1<<13)
#define NOBORDER_FLAG        (1<<14)
#define ICON_FLAG            (1<<15)
#define STARTSONDESK_FLAG    (1<<16)
#define BW_FLAG              (1<<17)
#define NOBW_FLAG            (1<<18)
#define FORE_COLOR_FLAG      (1<<19)
#define BACK_COLOR_FLAG      (1<<20)
#define RANDOM_PLACE_FLAG    (1<<21)
#define SMART_PLACE_FLAG     (1<<22)
#define MWM_BUTTON_FLAG      (1<<23)
#define MWM_DECOR_FLAG       (1<<24)
#define MWM_FUNCTIONS_FLAG   (1<<25)
#define MWM_OVERRIDE_FLAG    (1<<26)
#define NO_PPOSITION_FLAG    (1<<29)
#define OL_DECOR_FLAG        (1<<30)

/* some fancy font handling stuff */
#define NewFontAndColor(newfont,color,backcolor) {\
   Globalgcv.font = newfont;\
   Globalgcv.foreground = color;\
   Globalgcv.background = backcolor;\
   Globalgcm = GCFont | GCForeground | GCBackground; \
   XChangeGC(dpy,Scr.ScratchGC3,Globalgcm,&Globalgcv); \
}

#define ICON_HEIGHT (Scr.IconFont.height+6)

extern XGCValues Globalgcv;
extern unsigned long Globalgcm;
extern Time lastTimestamp;
extern XEvent Event;

extern char NoName[];
extern char NoClass[];
extern char NoResource[];

extern unsigned long LookInList (name_list *, char *, XClassHint *,
				 char **value,
				 int *Desk, int *bw, int *nobw,
				 char **forecolor, char **backcolor,
				 unsigned long *buttons);
extern void MoveOutline (Window, int, int, int, int);
extern void DoResize (int, int, XfwmWindow *);
extern void SetupFrame (XfwmWindow *, int, int, int, int, Bool);
extern void CreateGCs (void);
extern void InstallWindowColormaps (XfwmWindow *);
extern void InstallRootColormap (void);
extern void UninstallRootColormap (void);
extern void FetchWmProtocols (XfwmWindow *);
extern void FetchWmColormapWindows (XfwmWindow * tmp);
extern void PaintEntry (MenuRoot *, MenuItem *);
extern void PaintMenu (MenuRoot *);
extern void InitEventHandlerJumpTable (void);
extern void DispatchEvent (void);
extern void HandleEvents (void);
extern void HandleExpose (void);
extern void HandleFocusIn (void);
extern void HandleFocusOut (void);
extern void HandleDestroyNotify (void);
extern void HandleMapRequest (void);
extern void HandleMapRequestKeepRaised (Window keepraised);
extern void HandleMapNotify (void);
extern void HandleUnmapNotify (void);
extern void HandleMotionNotify (void);
extern void HandleButtonRelease (void);
extern void HandleButtonPress (void);
extern void HandleEnterNotify (void);
extern void HandleLeaveNotify (void);
extern void HandleConfigureRequest (void);
extern void HandleClientMessage (void);
extern void HandlePropertyNotify (void);
extern void HandleKeyPress (void);
extern void HandleVisibilityNotify (void);
extern void HandleColormapNotify (void);
extern void SetTitleBar (XfwmWindow *, Bool, Bool);
extern void RestoreWithdrawnLocation (XfwmWindow *, Bool);
extern void Destroy (XfwmWindow *);
extern void GetGravityOffsets (XfwmWindow *, int *, int *);
extern void MoveViewport (int newx, int newy, Bool);
extern XfwmWindow *AddWindow (Window w);
extern int MappedNotOverride (Window w);
extern void GrabButtons (XfwmWindow *);
extern void GrabKeys (XfwmWindow *);
extern void GetWindowSizeHints (XfwmWindow *);
extern void SwitchPages (Bool, Bool);
extern void NextPage (void);
extern void PrevPage (void);
extern void moveLoop (XfwmWindow *, int, int, int, int, int *, int *, Bool, Bool);

extern void Keyboard_shortcuts (XEvent *, int);
extern void RedoIconName (XfwmWindow *);
extern void DrawIconWindow (XfwmWindow *);
extern void CreateIconWindow (XfwmWindow * tmp_win, int def_x, int def_y);
extern inline void RelieveWindow (XfwmWindow *, Window,
				  int, int, int, int, GC, GC, int);
void RelieveParts (XfwmWindow * t, int i, GC hor, GC vert);
#define NO_HILITE     0x0000
#define TOP_HILITE    0x0001
#define RIGHT_HILITE  0x0002
#define BOTTOM_HILITE 0x0004
#define LEFT_HILITE   0x0008
#define FULL_HILITE   0x000F
#define HH_HILITE     0x0010

extern void sleep_a_little (int);
void Maximize (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	       unsigned long context, char *action, int *Module);
extern void RaiseWindow (XfwmWindow * t);
extern void LowerWindow (XfwmWindow * t);
extern Bool GrabEm (int);
extern void UngrabEm (void);
extern MenuRoot *NewMenuRoot (char *name, int function_or_popup);
extern void AddToMenu (MenuRoot *, char *, char *);
extern void MakeMenu (MenuRoot *);
extern void CaptureAllWindows (void);
extern void SetTimer (int);
extern int flush_expose (Window w);
void ExecuteFunction (char *Action, XfwmWindow * tmp_win, XEvent * eventp,
		      unsigned long context, int Module);
void do_windowList (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		    unsigned long context, char *action, int *Module);
extern void RaiseThisWindow (int);
extern int GetContext (XfwmWindow *, XEvent *, Window * dummy);
extern void ConstrainSize (XfwmWindow *, int *, int *);
extern void HandlePaging (int, int, int *, int *, int *, int *, Bool);
extern void SetShape (XfwmWindow *, int);
extern void AutoPlace (XfwmWindow *);
void executeModule (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		    unsigned long context, char *action, int *Module);
extern void SetFocus (Window, XfwmWindow *, Bool FocusByMouse);
extern void CheckAndSetFocus (void);
extern void initModules (void);
extern void HandleModuleInput (Window w, int channel);
extern void match_string (struct config *, char *, char *, FILE *);
extern void no_popup (char *ptr);
extern void KillModule (int channel, int place);
extern void ClosePipes (void);
extern char *findIconFile (char *icon, char *pathlist, int mode);
int find_func_type (char *action);
extern void GetBitmapFile (XfwmWindow * tmp_win);
extern void GetXPMFile (XfwmWindow * tmp_win);
extern void GetIconWindow (XfwmWindow * tmp_win);
extern void GetIconBitmap (XfwmWindow * tmp_win);
extern void SmartPlacement (XfwmWindow * t, int width, int height, int *x, int *y);
extern void usage (void);
void Broadcast (unsigned long event_type, unsigned long num_datum,
		unsigned long data1, unsigned long data2,
		unsigned long data3, unsigned long data4,
		unsigned long data5, unsigned long data6,
		unsigned long data7);
void BroadcastConfig (unsigned long event_type, XfwmWindow * t);
void SendPacket (int channel, unsigned long event_type, unsigned long num_datum,
		 unsigned long data1, unsigned long data2,
		 unsigned long data3, unsigned long data4,
		 unsigned long data5, unsigned long data6,
		 unsigned long data7);
void SendConfig (int Module, unsigned long event_type, XfwmWindow * t);
void BroadcastName (unsigned long event_type, unsigned long data1,
		    unsigned long data2, unsigned long data3, char *name);
void SendName (int channel, unsigned long event_type, unsigned long data1,
	       unsigned long data2, unsigned long data3, char *name);
void SendStrToModule (XEvent * eventp, Window junk, XfwmWindow * tmp_win,
		      unsigned long context, char *action, int *Module);
void DeadPipe (int nonsense);
void GetMwmHints (XfwmWindow * t);
void GetExtHints (XfwmWindow * t);
void GetKDEHints (XfwmWindow *);
void SelectDecor (XfwmWindow *, unsigned long, int, int);
extern Bool PopUpMenu (MenuRoot *, int, int);
void ComplexFunction (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		      unsigned long context, char *action, int *Module);
extern int DeferExecution (XEvent *, Window *, XfwmWindow **, unsigned long *, int, int);
void SetBorder (XfwmWindow *, Bool, Bool, Bool, Window);
void move_window (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		  unsigned long context, char *action, int *Module);
void resize_window (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		    unsigned long context, char *action, int *Module);
void CreateIconWindow (XfwmWindow *, int, int);
void SetMapStateProp (XfwmWindow *, int);
void SetStickyProp (XfwmWindow *, int, int, int);
void SetClientProp (XfwmWindow *);
void Iconify (XfwmWindow *, int, int);
void DeIconify (XfwmWindow *);
void PopDownMenu (void);
void KeepOnTop (void);
void show_panner (void);
void WaitForButtonsUp (void);
void FocusOn (XfwmWindow * t, int DeIconifyOnly);
void WarpOn (XfwmWindow * t, int warp_x, int x_unit, int warp_y, int y_unit);
Bool PlaceWindow (XfwmWindow * tmp_win, unsigned long flags, int Desk);
void free_window_names (XfwmWindow * tmp, Bool nukename, Bool nukeicon);

int do_menu (MenuRoot * menu, int style);
int check_allowed_function (MenuItem * mi);
void ReInstallActiveColormap (void);
void ParsePopupEntry (char *, FILE *, char **, int *);
void ParseMouseEntry (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		      unsigned long context, char *tline, int *Module);
void ParseKeyEntry (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		    unsigned long context, char *tline, int *Module);
void SetOneStyle (char *text, FILE *, char **, int *);
void AddToList (char *name, char *icon_name,
		unsigned long off_flags,
		unsigned long on_flags, int desk, int bw, int nobw,
		char *forecolor, char *backcolor,
		unsigned long off_buttons, unsigned long on_buttons);

void ParseStyle (char *text, FILE *, char **, int *);
void assign_string (char *text, FILE * fd, char **arg, int *);
void SetFlag (char *text, FILE * fd, char **arg, int *);
void SetCursor (char *text, FILE * fd, char **arg, int *);
void SetInts (char *text, FILE * fd, char **arg, int *);
void SetBox (char *text, FILE * fd, char **arg, int *);
void set_func (char *, FILE *, char **, int *);
void copy_config (FILE ** config_fd);
void SetEdgeScroll (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		    unsigned long context, char *action, int *Module);
void SetEdgeResistance (XEvent * eventp, Window w, XfwmWindow * tmp_win,
			unsigned long context, char *action, int *Module);
void CursorColor (XEvent * eventp, Window junk, XfwmWindow * tmp_win,
		  unsigned long context, char *action, int *Module);
void SetColormapFocus (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		       unsigned long context, char *action, int *Module);
inline void DrawPattern (Window, GC, GC, int, int, int);
Pixel GetShadow (Pixel);
Pixel GetHilite (Pixel);


#define UP 1
#define DOWN 0
void MapIt (XfwmWindow * t);
void UnmapIt (XfwmWindow * t);
void do_save (void);
void checkPanFrames (void);
void raisePanFrames (void);
void initPanFrames (void);
Bool StashEventTime (XEvent * ev);
int My_XNextEvent (Display * dpy, XEvent * event);
void FlushQueue (int Module);
void QuickRestart (void);
void AddFuncKey (char *, int, int, int, char *, int, int, MenuRoot *,
		 char, char);
char *GetNextPtr (char *ptr);

void InteractiveMove (Window * w, XfwmWindow * tmp_win, int *FinalX, int *FinalY,
		      XEvent * eventp);

MenuRoot *FindPopup (char *action);

void Bell (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	   unsigned long context, char *action, int *Module);
void scroll (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	     unsigned long context, char *action, int *Module);
void movecursor (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		 unsigned long context, char *action, int *Module);
void iconify_function (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		       unsigned long context, char *action, int *Module);
void raise_function (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		     unsigned long context, char *action, int *Module);
void lower_function (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		     unsigned long context, char *action, int *Module);
void destroy_function (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		       unsigned long context, char *action, int *Module);
void delete_function (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		      unsigned long context, char *action, int *Module);
void close_function (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		     unsigned long context, char *action, int *Module);
void restart_function (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		       unsigned long context, char *action, int *Module);
void exec_function (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		    unsigned long context, char *action, int *Module);
void exec_setup (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		 unsigned long context, char *action, int *Module);
void refresh_function (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		       unsigned long context, char *action, int *Module);
void refresh_win_function (XEvent * eventp, Window w, XfwmWindow * tmp_win,
			   unsigned long context, char *action, int *Module);
void stick_function (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		     unsigned long context, char *action, int *Module);

void changeDesks_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		       unsigned long context, char *action, int *Module);
void changeDesks (int val1, int val2);
void changeWindowsDesk (XEvent * eventp, Window w, XfwmWindow * tmp_win,
			unsigned long context, char *action, int *Module);

int GetTwoArguments (char *action, int *val1, int *val2, int *val1_unit, int *val2_unit);
int GetOneArgument (char *action, long *val1, int *val1_unit);
void goto_page_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		     unsigned long context, char *action, int *Module);

void wait_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		unsigned long context, char *action, int *Module);
void flip_focus_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		      unsigned long context, char *action, int *Module);
void focus_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		 unsigned long context, char *action, int *Module);
void warp_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		unsigned long context, char *action, int *Module);
void SendDataToModule (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		       unsigned long context, char *action, int *Module);
void send_list_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		     unsigned long context, char *action, int *Module);
void popup_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		 unsigned long context, char *action, int *Module);
void staysup_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		   unsigned long context, char *action, int *Module);
void quit_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		unsigned long context, char *action, int *Module);
void quit_screen_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		       unsigned long context, char *action, int *Module);
void echo_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		unsigned long context, char *action, int *Module);
void raiselower_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		      unsigned long context, char *action, int *Module);
void Nop_func (XEvent * eventp, Window w, XfwmWindow * tmp_win, unsigned long context,
	       char *action, int *Module);
void set_mask_function (XEvent * eventp, Window w, XfwmWindow * tmp_win,
			unsigned long context, char *action, int *Module);
void DestroyMenu (MenuRoot * mr);
void GetColors (void);
XColor GetXColor (char *);
Pixel GetColor (char *);
Pixel *AllocLinearGradient (char *s_from, char *s_to, int npixels);
Pixel *AllocNonlinearGradient (char *s_colors[], int clen[],
			       int nsegs, int npixels);
void bad_binding (int num);
void nocolor (char *note, char *name);
void MakeMenus (void);
void GetMenuXPMFile (char *name, MenuItem * it);
void GetMenuBitmapFile (char *name, MenuItem * it);
void add_item_to_menu (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		       unsigned long context,
		       char *action, int *Module);
void destroy_menu (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		   unsigned long context,
		   char *action, int *Module);
void add_another_item (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		       unsigned long context,
		       char *action, int *Module);
void add_item_to_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		       unsigned long context,
		       char *action, int *Module);
void setModulePath (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		    unsigned long context, char *action, int *Module);
void ProcessNewStyle (XEvent * eventp, Window w, XfwmWindow * tmp_win, unsigned long context,
		      char *action, int *Module);
void SetHiColor (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		 unsigned long context, char *action, int *Module);
void SetLoColor (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		 unsigned long context, char *action, int *Module);
void SetMenuColor (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		   unsigned long context, char *action, int *Module);
void LoadIconFont (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		   unsigned long context, char *action, int *Module);
void LoadWindowFont (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		     unsigned long context, char *action, int *Module);
void SetMenuFont (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		  unsigned long context, char *action, int *Module);
void SetTitleStyle (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		    unsigned long context, char *action, int *Module);
/*
 * void AddTitleStyle(XEvent *eventp,Window w,XfwmWindow *tmp_win,
 * unsigned long context, char *action,int* Module);
 */
void SetDeskSize (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		  unsigned long context, char *action, int *Module);
void SetXOR (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	     unsigned long context, char *action, int *Module);
void SetClick (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	       unsigned long context, char *action, int *Module);
void NextFunc (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	       unsigned long context, char *action, int *Module);
void PrevFunc (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	       unsigned long context, char *action, int *Module);
void NoneFunc (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	       unsigned long context, char *action, int *Module);
void CurrentFunc (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		  unsigned long context, char *action, int *Module);
void WindowIdFunc (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		   unsigned long context, char *action, int *Module);
void ReadFile (XEvent * eventp, Window junk, XfwmWindow * tmp_win,
	       unsigned long context, char *action, int *Module);
void PipeRead (XEvent * eventp, Window junk, XfwmWindow * tmp_win,
	       unsigned long context, char *action, int *Module);
void module_zapper (XEvent * eventp, Window junk, XfwmWindow * tmp_win,
		    unsigned long context, char *action, int *Module);
char *expand (char *input, char *arguments[], XfwmWindow * tmp_win);
void Recapture (XEvent * eventp, Window junk, XfwmWindow * tmp_win,
		unsigned long context, char *action, int *Module);
void HandleHardFocus (XfwmWindow * t);
void DestroyModConfig (XEvent * eventp, Window junk, XfwmWindow * tmp_win,
		       unsigned long context, char *action, int *Module);
void AddModConfig (XEvent * eventp, Window junk, XfwmWindow * tmp_win,
		   unsigned long context, char *action, int *Module);

/*
 * ** message levels for xfwm_msg:
 */
#define DBG  -1
#define INFO 0
#define WARN 1
#define ERR  2
void xfwm_msg (int type, char *id, char *msg,...);

#ifdef BROKEN_SUN_HEADERS
#include "sun_headers.h"
#endif
#ifdef NEEDS_ALPHA_HEADER
#include "alpha_header.h"
#endif /* NEEDS_ALPHA_HEADER */
#endif /* _MISC_ */
