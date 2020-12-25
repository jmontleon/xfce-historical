/*

   ORIGINAL FILE NAME : resources.h

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



#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#define BUTTONLEN	16
#define TITLELEN	64
#define TEXTLEN 	128

#define NB_RESOURCES    60

typedef struct
  {
    char *loadlabel;
    char *savelabel;
    char *defaultlabel;
    char *oklabel;
    char *applylabel;
    char *cancellabel;
    char *agreementlabel;
    char *browselabel;
    char *removelabel;
    char *startuptext;
    char *pulldownadd;
    char *additemtitle;
    char *additemcommand;
    char *additemiconfile;
    char *additemlabel;
    char *additempreview;
    char *defcomtitle;
    char *defcomcommand;
    char *defcomicon;
    char *screentitle;
    char *screenlabel;
    char *infotitle;
    char *modifytitle;
    char *setuptitle;
    char *setuprepaint;
    char *setupgradient;
    char *setupdetach;
    char *clicktofocus;
    char *autoraise;
    char *opaquemove;
    char *quit;
    char *menufull;
    char *FVWMonly;
    char *syntax;
    char *datamis;
    char *filetrunc;
    char *notcreate;
    char *notreset;
    char *notopen;
    char *notload;
    char *notsave;
    char *notwrite;
    char *notfound;
    char *toolong;
    char *overwrite;
    char *delentry;
    char *fillout;
    char *paltoload;
    char *paltosave;
    char *selexec;
    char *selicon;
    char *tinyfont;
    char *regularfont;
    char *boldfont;
    char *bigfont;
    char *titlefont;
    int   iconwidth;
    int   iconheight;
    int   labelwidth;
    int   detachheight;
  }
res_XFCE;

extern void create_resources (res_XFCE *);
extern void load_resources (res_XFCE *);
extern void free_resources (res_XFCE *);

#endif
