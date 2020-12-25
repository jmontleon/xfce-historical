/*

   ORIGINAL FILE NAME : xfsound_cb.c

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




#include "forms.h"
#include "xfsound.h"
#include "../xfce/constant.h"

int    prev;

void 
show_fd_xfsound (char *title)
{
  int i;

  prev = 0;
  for (i = 0; i < KNOWN_MESSAGES+KNOWN_BUILTIN; i++)
    fl_addto_choice (fd_xfsound->eventlist, messages[i].label);
  fl_set_choice (fd_xfsound->eventlist, 1);
  fl_set_input (fd_xfsound->soundfile, sndcfg.datafiles[0]);
  fl_set_input (fd_xfsound->play_com, sndcfg.playcmd);
  fl_set_button (fd_xfsound->doplay, (sndcfg.playsnd != 0));
  fl_show_form (fd_xfsound->xfsound, FL_PLACE_CENTER, FL_TRANSIENT, title);
  fl_set_cursor (fd_xfsound->xfsound->window, XFCE_CURS);
}

void
get_all_data (int i)
{
  if (fl_get_button(fd_xfsound->doplay))
     sndcfg.playsnd = 1;
  else
     sndcfg.playsnd = 0;
  strcpy (sndcfg.playcmd, cleanup ((char *) fl_get_input (fd_xfsound->play_com)));
  strcpy (sndcfg.datafiles[i], cleanup ((char *) fl_get_input (fd_xfsound->soundfile)));
}

void
update_all_data (void)
{
  int i;

  strcpy (sndcfg.datafiles[prev], cleanup ((char *) fl_get_input (fd_xfsound->soundfile)));
  i = fl_get_choice (fd_xfsound->eventlist) - 1;
  fl_set_input (fd_xfsound->soundfile, sndcfg.datafiles[i]);
  fl_set_input (fd_xfsound->play_com, sndcfg.playcmd);
  fl_set_button (fd_xfsound->doplay, (sndcfg.playsnd != 0));
  prev = i;
}

void doplay_cb(FL_OBJECT *ob, long data)
{
  if (fl_get_button(fd_xfsound->doplay))
     sndcfg.playsnd = 1;
  else
     sndcfg.playsnd = 0;
}

void play_com_cb(FL_OBJECT *ob, long data)
{
  strcpy (sndcfg.playcmd, cleanup ((char *) 
                                    fl_get_input (fd_xfsound->play_com)));
}

void eventlist_cb(FL_OBJECT *ob, long data)
{
  update_all_data();
}

void soundfile_cb(FL_OBJECT *ob, long data)
{
  update_all_data();
}

void browsefile_cb(FL_OBJECT *ob, long data)
{
  const char *fselect;
  int i;

  i = fl_get_choice (fd_xfsound->eventlist) - 1;
  fselect = skiphead ((char *) fl_show_fselector (rxfsnd.sndfilelabel,
                                                  XFCE_SOUNDS, "*.wav", ""));
  if (fselect)
    {
      if (strlen (fselect)) cleanup ((char *)fselect);
      fl_set_input (fd_xfsound->soundfile, fselect);
      strcpy (sndcfg.datafiles[i], fselect);
    }
}

void internal_cb(FL_OBJECT *ob, long data)
{
  fl_set_input (fd_xfsound->play_com, INTERNAL_PLAYER );
  strcpy (sndcfg.playcmd, INTERNAL_PLAYER );
}

void defaultcmd_cb(FL_OBJECT *ob, long data)
{
  fl_set_input (fd_xfsound->play_com, DEFAULT_PLAYER );
  strcpy (sndcfg.playcmd, DEFAULT_PLAYER );
}

void ok_cb(FL_OBJECT *ob, long data)
{
  int i;

  i = fl_get_choice (fd_xfsound->eventlist) - 1;
  get_all_data(i);
  savecfg(&sndcfg);
  fl_hide_form (fd_xfsound->xfsound);
  fl_finish ();
  exit (0);
}

void apply_cb(FL_OBJECT *ob, long data)
{
  int i;

  i = fl_get_choice (fd_xfsound->eventlist) - 1;
  get_all_data(i);
  savecfg(&sndcfg);
}

void cancel_cb(FL_OBJECT *ob, long data)
{
  fl_hide_form (fd_xfsound->xfsound);
  fl_finish ();
  exit (0);
}

void testfile_cb(FL_OBJECT *ob, long data)
{
  int i;

  i = fl_get_choice (fd_xfsound->eventlist) - 1;
  get_all_data(i);
  audio_play (i);
}



