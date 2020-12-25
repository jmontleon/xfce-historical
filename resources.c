
/*

   ORIGINAL FILE NAME : resources.c

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


#include <stdlib.h>
#include "resources.h"
#include "forms.h"

void 
create_resources (res_XFCE * r)
{
  r->loadlabel = (char *) malloc (BUTTONLEN);
  r->savelabel = (char *) malloc (BUTTONLEN);
  r->defaultlabel = (char *) malloc (BUTTONLEN);
  r->oklabel = (char *) malloc (BUTTONLEN);
  r->applylabel = (char *) malloc (BUTTONLEN);
  r->cancellabel = (char *) malloc (BUTTONLEN);
  r->agreementlabel = (char *) malloc (BUTTONLEN);
  r->browselabel = (char *) malloc (BUTTONLEN);
  r->removelabel = (char *) malloc (BUTTONLEN);
  r->startuptext = (char *) malloc (TEXTLEN);
  r->pulldownadd = (char *) malloc (TEXTLEN);
  r->additemtitle = (char *) malloc (TITLELEN);
  r->additemcommand = (char *) malloc (TEXTLEN);
  r->additemiconfile = (char *) malloc (TEXTLEN);
  r->additemlabel = (char *) malloc (TEXTLEN);
  r->additempreview = (char *) malloc (TEXTLEN);
  r->defcomtitle = (char *) malloc (TITLELEN);
  r->defcomcommand = (char *) malloc (TEXTLEN);
  r->defcomicon = (char *) malloc (TEXTLEN);
  r->screentitle = (char *) malloc (TITLELEN);
  r->screenlabel = (char *) malloc (TEXTLEN);
  r->infotitle = (char *) malloc (TITLELEN);
  r->modifytitle = (char *) malloc (TITLELEN);
  r->setuptitle = (char *) malloc (TITLELEN);
  r->setuprepaint = (char *) malloc (TEXTLEN);
  r->setupdetach = (char *) malloc (TEXTLEN);
  r->quit = (char *) malloc (TEXTLEN);
  r->menufull = (char *) malloc (TEXTLEN);
  r->FVWMonly = (char *) malloc (TEXTLEN);
  r->syntax = (char *) malloc (TEXTLEN);
  r->datamis = (char *) malloc (TEXTLEN);
  r->filetrunc = (char *) malloc (TEXTLEN);
  r->notcreate = (char *) malloc (TEXTLEN);
  r->notreset = (char *) malloc (TEXTLEN);
  r->notopen = (char *) malloc (TEXTLEN);
  r->notload = (char *) malloc (TEXTLEN);
  r->notsave = (char *) malloc (TEXTLEN);
  r->notwrite = (char *) malloc (TEXTLEN);
  r->notfound = (char *) malloc (TEXTLEN);
  r->toolong = (char *) malloc (TEXTLEN);
  r->overwrite = (char *) malloc (TEXTLEN);
  r->delentry = (char *) malloc (TEXTLEN);
  r->fillout = (char *) malloc (TEXTLEN);
  r->paltoload = (char *) malloc (TEXTLEN);
  r->paltosave = (char *) malloc (TEXTLEN);
  r->selexec = (char *) malloc (TEXTLEN);
  r->selicon = (char *) malloc (TEXTLEN);
}

void 
load_resources (res_XFCE * r)
{
  FL_resource res[] =
  {
    {"button.loadLabel",
     "XForm.loadLabel",
     FL_STRING,
     r->loadlabel,
     "Load...",
     BUTTONLEN
    },
    {
      "button.saveLabel",
      "XForm.saveLabel",
      FL_STRING,
      r->savelabel,
      "Save...",
      BUTTONLEN
    },
    {
      "button.defaultLabel",
      "XForm.defaultLabel",
      FL_STRING,
      r->defaultlabel,
      "Default",
      BUTTONLEN
    },
    {
      "button.okLabel",
      "XForm.okLabel",
      FL_STRING,
      r->oklabel,
      "Ok",
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
    {
      "button.cancelLabel",
      "XForm.cancelLabel",
      FL_STRING,
      r->cancellabel,
      "Cancel",
      BUTTONLEN
    },
    {
      "button.agreementLabel",
      "XForm.agreementLabel",
      FL_STRING,
      r->agreementlabel,
      "Agreement",
      BUTTONLEN
    },
    {
      "button.browseLabel",
      "XForm.browseLabel",
      FL_STRING,
      r->browselabel,
      "Browse...",
      BUTTONLEN
    },
    {
      "button.removeLabel",
      "XForm.removeLabel",
      FL_STRING,
      r->removelabel,
      "Remove",
      BUTTONLEN
    },
    {
      "startup.waitText",
      "XForm.waitText",
      FL_STRING,
      r->startuptext,
      "Please Wait, XFCE is loading...",
      TEXTLEN
    },
    {
      "pulldown.addiconLabel",
      "XForm.addiconLabel",
      FL_STRING,
      r->pulldownadd,
      "Add icon...",
      TEXTLEN
    },
    {
      "dialog.additemTitle",
      "XForm.additemTitle",
      FL_STRING,
      r->additemtitle,
      "Add item...",
      TITLELEN
    },
    {
      "additem.commandText",
      "XForm.commandText",
      FL_STRING,
      r->additemcommand,
      "Command line :",
      TEXTLEN
    },
    {
      "additem.iconfileText",
      "XForm.iconfileText",
      FL_STRING,
      r->additemiconfile,
      "Icon file :",
      TEXTLEN
    },
    {
      "additem.menulabelText",
      "XForm.menulabelText",
      FL_STRING,
      r->additemlabel,
      "Label :",
      TEXTLEN
    },
    {
      "additem.previewText",
      "XForm.previewText",
      FL_STRING,
      r->additempreview,
      "Preview Icon",
      TEXTLEN
    },
    {
      "dialog.defcomTitle",
      "XForm.defcomTitle",
      FL_STRING,
      r->defcomtitle,
      "Define action...",
      TITLELEN
    },
    {
      "defcom.commandText",
      "XForm.commandText",
      FL_STRING,
      r->defcomcommand,
      "Command line :",
      TEXTLEN
    },
    {
      "defcom.iconstyleText",
      "XForm.inconfileText",
      FL_STRING,
      r->defcomicon,
      "Icon style :",
      TEXTLEN
    },
    {
      "dialog.screenTitle",
      "XForm.screenTitle",
      FL_STRING,
      r->screentitle,
      "Screen name...",
      TITLELEN
    },
    {
      "screen.labelText",
      "XForm.labelText",
      FL_STRING,
      r->screenlabel,
      "Define new label :",
      TEXTLEN
    },
    {
      "dialog.infoTitle",
      "XForm.infoTitle",
      FL_STRING,
      r->infotitle,
      "Info...",
      TITLELEN
    },
    {
      "dialog.modifyTitle",
      "XForm.modifyTitle",
      FL_STRING,
      r->modifytitle,
      "Modify item...",
      TITLELEN
    },
    {
      "dialog.setupTitle",
      "XForm.setupTitle",
      FL_STRING,
      r->setuptitle,
      "Setup...",
      TITLELEN
    },
    {
      "setup.repaintText",
      "XForm.repaintText",
      FL_STRING,
      r->setuprepaint,
      "Repaint root window of workspace",
      TEXTLEN
    },
    {
      "setup.detachText",
      "XForm.detachText",
      FL_STRING,
      r->setupdetach,
      "Use tear-off menus",
      TEXTLEN
    },
    {
      "alert.quitText",
      "XForm.quitText",
      FL_STRING,
      r->quit,
      "Are you sure you want to quit ?\nThis might log you off",
      TEXTLEN
    },
    {
      "alert.menufullText",
      "XForm.menufullText",
      FL_STRING,
      r->menufull,
      "Warning\nThis menu is full !\nTry to add items to another one",
      TEXTLEN
    },
    {
      "alert.FVWMonlyText",
      "XForm.FVWMonlyText",
      FL_STRING,
      r->FVWMonly,
      "Warning\nThis function works only with FVWM !\nSee FVWM man page",
      TEXTLEN
    },
    {
      "alert.syntaxerrorText",
      "XForm.syntaxerrorText",
      FL_STRING,
      r->syntax,
      "Error\nSyntax error\nin configuration file",
      TEXTLEN
    },
    {
      "alert.datamismatchText",
      "XForm.datamismatchText",
      FL_STRING,
      r->datamis,
      "Error\nData mismatch error\nin configuration file",
      TEXTLEN
    },
    {
      "alert.filetruncText",
      "XForm.filetruncText",
      FL_STRING,
      r->filetrunc,
      "Error\nFile truncated",
      TEXTLEN
    },
    {
      "alert.createText",
      "XForm.createText",
      FL_STRING,
      r->notcreate,
      "Error\nUnable to create\nconfiguration file !",
      TEXTLEN
    },
    {
      "alert.resetText",
      "XForm.resetText",
      FL_STRING,
      r->notreset,
      "Error\nUnable to reset\nconfiguration file !",
      TEXTLEN
    },
    {
      "alert.openText",
      "XForm.openText",
      FL_STRING,
      r->notopen,
      "Error\nUnable to open\nconfiguration file !",
      TEXTLEN
    },
    {
      "alert.loadText",
      "XForm.loadText",
      FL_STRING,
      r->notload,
      "Warning\nUnable to load file !",
      TEXTLEN
    },
    {
      "alert.saveText",
      "XForm.saveText",
      FL_STRING,
      r->notsave,
      "Warning\nUnable to save file !",
      TEXTLEN
    },
    {
      "alert.writeText",
      "XForm.writeText",
      FL_STRING,
      r->notwrite,
      "Warning\nUnable to write file !",
      TEXTLEN
    },
    {
      "alert.notfoundText",
      "XForm.notfoundText",
      FL_STRING,
      r->notfound,
      "Warning\nFile not found !",
      TEXTLEN
    },
    {
      "alert.toolongText",
      "XForm.toolongText",
      FL_STRING,
      r->toolong,
      "Warning\nString too long !",
      TEXTLEN
    },
    {
      "alert.overwriteText",
      "XForm.overwriteText",
      FL_STRING,
      r->overwrite,
      "Warning\nFile exists : Overwrite ?",
      TEXTLEN
    },
    {
      "alert.delentryText",
      "XForm.delentryText",
      FL_STRING,
      r->delentry,
      "Warning\nAre you sure you want to remove\nthis entry ?",
      TEXTLEN
    },
    {
      "alert.filloutText",
      "XForm.filloutText",
      FL_STRING,
      r->fillout,
      "Warning\nPlease fill out command\nand label fields...",
      TEXTLEN
    },
    {
      "selector.loadpalText",
      "XForm.loadpalText",
      FL_STRING,
      r->paltoload,
      "Palette to load :",
      TEXTLEN
    },
    {
      "selector.savepalText",
      "XForm.savepalText",
      FL_STRING,
      r->paltosave,
      "Palette to save :",
      TEXTLEN
    },
    {
      "selector.execfileText",
      "XForm.execfileText",
      FL_STRING,
      r->selexec,
      "Choose file to execute :",
      TEXTLEN
    },
    {
      "selector.iconfileText",
      "XForm.iconfileText",
      FL_STRING,
      r->selicon,
      "Choose icon file :",
      TEXTLEN
    },
  };
  fl_get_app_resources (res, 47);
}

void 
free_resources (res_XFCE * r)
{
  free (r->loadlabel);
  free (r->savelabel);
  free (r->defaultlabel);
  free (r->oklabel);
  free (r->applylabel);
  free (r->cancellabel);
  free (r->agreementlabel);
  free (r->browselabel);
  free (r->removelabel);
  free (r->startuptext);
  free (r->pulldownadd);
  free (r->additemtitle);
  free (r->additemcommand);
  free (r->additemiconfile);
  free (r->additemlabel);
  free (r->additempreview);
  free (r->defcomtitle);
  free (r->defcomcommand);
  free (r->defcomicon);
  free (r->screentitle);
  free (r->screenlabel);
  free (r->infotitle);
  free (r->modifytitle);
  free (r->setuptitle);
  free (r->setuprepaint);
  free (r->setupdetach);
  free (r->quit);
  free (r->menufull);
  free (r->FVWMonly);
  free (r->syntax);
  free (r->datamis);
  free (r->filetrunc);
  free (r->notcreate);
  free (r->notreset);
  free (r->notopen);
  free (r->notload);
  free (r->notsave);
  free (r->notwrite);
  free (r->notfound);
  free (r->toolong);
  free (r->overwrite);
  free (r->delentry);
  free (r->fillout);
  free (r->paltoload);
  free (r->paltosave);
  free (r->selexec);
  free (r->selicon);
}
