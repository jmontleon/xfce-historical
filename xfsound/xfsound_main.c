/*

   ORIGINAL FILE NAME : xfsound_main.c

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




#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/Xlocale.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include "forms.h"
#include "xfsound.h"
#include "xfdsp.h"
#include "../xfwm/utils.h"

#define RCFILE                  ".xfsoundrc"

T_messages messages[KNOWN_MESSAGES+KNOWN_BUILTIN] =
{
	{ M_NEW_DESK,         "New Desk"         },
	{ M_ADD_WINDOW,       "Add Window"       },
	{ M_RAISE_WINDOW,     "Raise Window"     },
	{ M_LOWER_WINDOW,     "Lower Window"     },
	{ M_CONFIGURE_WINDOW, "Configure Window" },
	{ M_FOCUS_CHANGE,     "Focus Change"     },
	{ M_DESTROY_WINDOW,   "Destroy Window"   },
	{ M_ICONIFY,          "Iconify"          },
	{ M_DEICONIFY,        "De-iconify"       },
	{ M_MAXIMIZE,         "Maximize"         },
	{ M_DEMAXIMIZE,       "De-maximize"      },
	{ 0,                  "Startup"          },
	{ 0,                  "Shutdown"         },
	{ 0,                  "Unknown"          }
};

char         *homedir,
             *tempstr;
XFSound       sndcfg;
FD_xfsound   *fd_xfsound;
int           fd_width;
int           fd[2];
res_XFSound   rxfsnd;
XFCE_palette *palette;

void 
create_resources (res_XFSound * r)
{
  r->xfsoundtitle    = (char *) malloc (TITLELEN);
  r->oklabel         = (char *) malloc (BUTTONLEN);
  r->browselabel     = (char *) malloc (BUTTONLEN);
  r->applylabel      = (char *) malloc (BUTTONLEN);
  r->eventlabel      = (char *) malloc (BUTTONLEN);
  r->cancellabel     = (char *) malloc (BUTTONLEN);
  r->testlabel       = (char *) malloc (BUTTONLEN);
  r->commandlabel    = (char *) malloc (BUTTONLEN);
  r->playsndlabel    = (char *) malloc (TEXTLEN);
  r->sndfilelabel    = (char *) malloc (TEXTLEN);
  r->tinyfont        = (char *) malloc (TEXTLEN);
  r->regularfont     = (char *) malloc (TEXTLEN);
  r->boldfont        = (char *) malloc (TEXTLEN);
  r->bigfont         = (char *) malloc (TEXTLEN);
  r->titlefont       = (char *) malloc (TEXTLEN);
}


void 
load_resources (res_XFSound * r)
{
  FL_resource res[] =
  {
    {
      "dialog.xfsoundTitle",
      "XForm.xfsoundTitle",
      FL_STRING,
      r->xfsoundtitle,
      "XFSound - XFce Sound Manager",
      TITLELEN
    },
    {"button.okLabel",
     "XForm.okLabel",
     FL_STRING,
     r->oklabel,
     "Ok",
     BUTTONLEN
    },
    {"button.browseLabel",
     "XForm.browseLabel",
     FL_STRING,
     r->browselabel,
     "Browse",
     BUTTONLEN
    },
    {
      "button.applyLabel",
      "XForm.applyLabel",
      FL_STRING,
      r->applylabel,
      "Apply",
      BUTTONLEN
    },
    {"button.eventLabel",
     "XForm.eventLabel",
     FL_STRING,
     r->eventlabel,
     "Event",
     BUTTONLEN
    },
    {
      "button.cancelLabel",
      "XForm.cancelLabel",
      FL_STRING,
      r->cancellabel,
      "Cancel",
      BUTTONLEN
    },
    {
      "button.testLabel",
      "XForm.testLabel",
      FL_STRING,
      r->testlabel,
      "Test",
      BUTTONLEN
    },
    {
      "xfsound.commandText",
      "XForm.commandText",
      FL_STRING,
      r->commandlabel,
      "Command",
      BUTTONLEN
    },
    {
      "button.playsndLabel",
      "XForm.playsndLabel",
      FL_STRING,
      r->playsndlabel,
      "Play Sound",
      TEXTLEN
    },
    {
      "xfsound.sndfileLabel",
      "XForm.sndfileLabel",
      FL_STRING,
      r->sndfilelabel,
      "Sound file",
      TEXTLEN
    },
    {
      "tiny.font",
      "XForm.tiny.font",
      FL_STRING,
      r->tinyfont,
      "-adobe-helvetica-medium-r-*-*-10-*-75-75-*-*-*-*",
      TEXTLEN
    },
    {
      "regular.font",
      "XForm.regular.font",
      FL_STRING,
      r->regularfont,
      "-adobe-helvetica-medium-r-*-*-12-*-75-75-*-*-*-*",
      TEXTLEN
    },
    {
      "bold.font",
      "XForm.bold.font",
      FL_STRING,
      r->boldfont,
      "-adobe-helvetica-bold-r-*-*-12-*-75-75-*-*-*-*",
      TEXTLEN
    },
    {
      "big.font",
      "XForm.big.font",
      FL_STRING,
      r->bigfont,
      "-adobe-helvetica-medium-r-*-*-14-*-75-75-*-*-*-*",
      TEXTLEN
    },
    {
      "title.font",
      "XForm.title.font",
      FL_STRING,
      r->titlefont,
      "-adobe-helvetica-bold-r-*-*-14-*-75-75-*-*-*-*",
      TEXTLEN
    },
  };
  fl_get_app_resources (res, 15);
}

void 
free_resources (res_XFSound * r)
{
  free (r->xfsoundtitle);	
  free (r->oklabel);	
  free (r->browselabel);	
  free (r->applylabel);
  free (r->eventlabel);
  free (r->cancellabel);
  free (r->testlabel);
  free (r->commandlabel);
  free (r->playsndlabel);
  free (r->sndfilelabel);
  free (r->tinyfont);
  free (r->regularfont);
  free (r->boldfont);
  free (r->bigfont);
  free (r->titlefont);
}

void
display_error(char *s)
{
  fprintf (stderr, "%s\n", s);
  my_alert (s, 1);
}

void
allocXFSound (XFSound *s)
{
  int i;
  s->playcmd = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
  for (i=0; i< (KNOWN_MESSAGES+KNOWN_BUILTIN); i++)
    s->datafiles[i] = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
}

void
freeXFSound (XFSound *s)
{
  int i;
  free (s->playcmd);
  for (i=0; i< (KNOWN_MESSAGES+KNOWN_BUILTIN); i++)
    free(s->datafiles[i]);
}

void
readstr (int i, char *str, FILE *f)
{
  if (str) 
    if (f) {
      fgets (str, i - 1, f);
      if (strlen (str))
        str[strlen (str) - 1] = 0;
    }
    else
      strcpy(str, "\0");
}

void
loadcfg (XFSound *s)
{
  int i;
  FILE *f = NULL;

  strcpy (homedir, (char *) getenv ("HOME"));
  strcat (homedir, "/");
  strcat (homedir, RCFILE);
  if (existfile (homedir))
    {
      f = fopen (homedir, "r");
    }
  else
    {
      strcpy (homedir, XFCE_DIR);
      strcat (homedir, "/");
      strcat (homedir, RCFILE);
      if (existfile (homedir))
	{
	  f = fopen (homedir, "r");
	}
      else
        {
          strcpy (homedir, XFCE_DIR);
          strcat (homedir, "/");
          strcat (homedir, "/sample");
          strcat (homedir, RCFILE);
          if (existfile (homedir))
   	    {
	      f = fopen (homedir, "r");
	    }
        }
    }

  if (f) 
    {
      readstr (15, tempstr, f);
      s->playsnd = (my_strncasecmp (tempstr, "Play", strlen ("Play")) == 0);
      readstr (MAXSTRLEN, s->playcmd, f);
      for (i=0; i< (KNOWN_MESSAGES+KNOWN_BUILTIN); i++)
        readstr(MAXSTRLEN, s->datafiles[i], f);
      fclose (f);
    }
  else
    {
      s->playsnd = 0;
      strcpy(s->playcmd, "\0");
      for (i=0; i< (KNOWN_MESSAGES+KNOWN_BUILTIN); i++)
	strcpy (s->datafiles[i], "\0");
    }
}

int
savecfg (XFSound *s)
{
  int i;
  FILE *f;

  strcpy (homedir, (char *) getenv ("HOME"));
  strcat (homedir, "/");
  strcat (homedir, RCFILE);

  if ((f = fopen (homedir, "w")))
    {
      fprintf (f, "%s\n", ((s->playsnd) ? "Play" : "NoPlay"));
      fprintf (f, "%s\n", s->playcmd);
      for (i=0; i< (KNOWN_MESSAGES + KNOWN_BUILTIN); i++)
        fprintf (f, "%s\n", s->datafiles[i]);
      fclose (f);
    }
  return ((f != NULL));
}

int
value_table(long id)
{
  int i;
  for (i = 0; i < KNOWN_MESSAGES; i++)
    if (messages[i].message_id == id) return i;
  return -1;
}

void
audio_play(short code)
{
  char *command;

  if ((sndcfg.playsnd) && strlen (sndcfg.playcmd) && strlen (sndcfg.datafiles[code]))
  {
    if (my_strncasecmp (sndcfg.playcmd, INTERNAL_PLAYER, strlen (INTERNAL_PLAYER)) == 0) {
      i_play(sndcfg.datafiles[code]);
    }
    else  {
      command = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
      strcpy (command, sndcfg.playcmd);
      strcat (command, " ");
      strcat (command, sndcfg.datafiles[code]);
      strcat (command, " &");
      system (command);
      free (command);
    }
  }
}

void done(int n)
{
   audio_play(BUILTIN_SHUTDOWN);
   exit(n);
}

void DeadPipe(int nonsense)
{
   done(0);
}

void Loop(int *fd)
{
  unsigned long	header[HEADER_SIZE];
  unsigned long *body = NULL;
  unsigned long	count;
  long code;

  while (1)
  {
      if ((count = ReadXfwmPacket(fd[1], header, &body)) <=0 ) {
         free(body);
         done(0);
      }
      free(body);
      code = value_table(header[1]);

      loadcfg (&sndcfg);

      if (code >= 0 && code < MAX_MESSAGES)
	  audio_play(code);
      else if (code >= MAX_MESSAGES)
          audio_play(BUILTIN_UNKNOWN);
  }
}

void 
startmodule(int argc, char *argv[])
{
   signal (SIGPIPE, DeadPipe);  

   fd[0] = atoi(argv[1]);
   fd[1] = atoi(argv[2]);

   audio_play(BUILTIN_STARTUP);
   sendinfo(fd,"Nop",0);
          audio_play(BUILTIN_UNKNOWN);
   Loop(fd);
}

void
startgui(int argc, char *argv[])
{
   FL_IOPT flopt;
   int version, revision;
   char *xfce_lang, *env_value, *classname;


   if (FL_INCLUDE_VERSION != fl_library_version (&version, &revision))
     {
       display_error("XFBD : Header and library version don't match\n");
       fprintf (stderr, "   Header: %d.%d\n", FL_VERSION, FL_REVISION);
       fprintf (stderr, "  Library: %d.%d\n", version, revision);
       exit (1);
     }

   flopt.borderWidth = -BORDERWIDTH;
   fl_set_defaults (FL_PDBorderWidth, &flopt);
   xfce_lang = (char *) malloc (3 * sizeof (char));
   env_value = (char *) getenv ("XFCE_LANG");
   if (env_value) {
     strncpy (xfce_lang, env_value, 2);
     xfce_lang[2] = '\0';
   }
   else
     xfce_lang[0] = '\0';

   classname = (char *) malloc (8 * sizeof (char));
   strcpy (classname, "XFce");
   if ((xfce_lang) && (strlen (xfce_lang))) {
     strcat (classname, "-");
     strcat (classname, xfce_lang);
   }

   if (!fl_initialize (&argc, argv, classname, 0, 0)) 
     exit (1);

   setlocale( LC_ALL, "");

   free (xfce_lang);
   free (classname);

   create_resources (&rxfsnd);
   load_resources (&rxfsnd);

   /*
      New in v. 2.0.2 : Fonts are taken from the resources
    */
   fl_set_font_name (XFTINY, rxfsnd.tinyfont);
   fl_set_font_name (XFREG,  rxfsnd.regularfont);
   fl_set_font_name (XFBOLD, rxfsnd.boldfont);
   fl_set_font_name (XFBIG,  rxfsnd.bigfont);
   fl_set_font_name (XFTIT,  rxfsnd.titlefont);


   palette = newpal ();
   initpal (palette);
   defpal (palette);
   loadpal (palette);
   applypal (palette);
   fl_set_cursor_color(XFCE_CURS, palcolor(palette, 0), FL_WHITE);
   fl_set_cursor_color(MENU_CURS, palcolor(palette, 0), FL_WHITE);

   fl_set_goodies_font (FL_BOLD_STYLE, FL_NORMAL_SIZE);
   fl_set_oneliner_font (FL_NORMAL_STYLE, FL_NORMAL_SIZE);
   fl_set_oneliner_color (FL_BLUE, FL_WHEAT);
   fl_setpup_default_cursor(MENU_CURS);

   fd_xfsound = create_form_xfsound();

   /* show the first form */
   show_fd_xfsound(rxfsnd.xfsoundtitle);
   fl_do_forms();
}

int main(int argc, char *argv[])
{
   homedir = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
   tempstr = (char *) malloc (16 * sizeof (char));
   allocXFSound(&sndcfg);
   loadcfg(&sndcfg);

   if ((argc != 6)&&(argc != 7))
     startgui(argc, argv);
   else
     startmodule(argc, argv);
   return 0;
}
