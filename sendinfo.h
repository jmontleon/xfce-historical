/*

   ORIGINAL FILE NAME : sendinfo.h

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

   The following code is mostly taken from Rob NATION's code of FVWM;
   So here comes his copyright notice :

   ***************************************************************************
   * This module, and the entire ModuleDebugger program, and the concept for *
   * interfacing this module to the Window Manager, are all original work    *
   * by Robert Nation                                                        *
   *                                                                         *
   * Copyright 1994, Robert Nation. No guarantees or warantees or anything   *
   * are provided or implied in any way whatsoever. Use this program at your *
   * own risk. Permission to use this program for any purpose is given,      *
   * as long as the copyright is kept intact.                                *
   ***************************************************************************

 */

#ifndef __SENDINFO_H__
#define __SENDINFO_H__

#include "module.h"

extern void sendinfo (int *, char *, unsigned long);
extern int readpacket (int, unsigned long *, unsigned long **);

int fd[2];

#endif
