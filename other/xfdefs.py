#!/usr/bin/env python
##
## Set X apps defaults to match xfce settings
##

import string, os, re

## Palette to resources mapping
resmap = ( [], \
	   ['*Menu*background'], \
	   [], \
	   [], \
	   ['*text*background', '*list*background', \
	    '*Text*background', '*List*background'], \
	   ['*Scrollbar*foreground'], \
	   [], \
	   ['*background', '*Background', 'OpenWindows*WindowColor', \
	    'Window.Color.Background', 'XTerm*background', \
	    'Rxvt*background', 'netscape*background', 'Netscape*background', \
	    '.netscape*background', 'Ddd*background', \
	    'Emacs*Background', 'Emacs*backgroundToolBarColor'] \
	   )
fntmap = ['*fontList', 'Netscape*fontList', '.netscape*fontList']

## Convert an rgb triplet to an X color string ((R, G, B) -> "#rrggbb")
def triplet2rgb(t):
    r = map(hex, t)
    for i in (0, 1, 2):
	r[i] = r[i][2:]
	while len(r[i]) < 2:
	    r[i] = '0'+r[i]
    return '#'+string.join(r, '')
    
## Load palette and font from ~/.xfcolors
colorfile = os.path.expanduser('~/.xfcolors')
palette = []
font = ''
f = open(colorfile, 'r')
for i in range(8):
    palette.append(map(string.atoi, string.split(f.readline())))
font = f.readline()
f.close()

## Update X resources database
pipe = os.popen('xrdb -merge', 'w')
for i in range(8):
    rgb = triplet2rgb(palette[i])
    for j in resmap[i]:
	pipe.write(j+': '+rgb+'\n')
for j in fntmap:
    pipe.write(j+': '+font+'\n')
pipe.close()

## Dump database from memory to ~/.Xdefaults (needed by rxvt)
#pipe = os.popen('xrdb -query', 'r')
#l = pipe.readlines()
#pipe.close()
#f = open(os.path.expanduser('~/.Xdefaults'), 'w')
#for i in l:
#	f.write(i)
#f.close()

## Fix netscape's default page background color
pattern = re.compile('user_pref\s*\(\s*\"browser.background_color\"\s*,\s*\"(?P<col>[^\"]*)\"\s*\)\s*;\s*')

def fixnetscapebg(fname, bgcolor):
    f = open(fname, 'r')
    t = open(fname+'.tmp', 'w')
    while 1:
	l = f.readline()
	if l is None or len(l)==0:
	    break
	sl = pattern.sub('user_pref("browser.background_color", "'+
			 bgcolor+'");', l)
	l = string.strip(sl)+'\n'
	t.write(l)
    f.close()
    t.close()

    os.unlink(fname)
    os.rename(fname+'.tmp', fname)

try:
    bg = triplet2rgb(palette[7])
    fixnetscapebg(os.path.expanduser('~/.netscape/preferences.js'), bg)
    fixnetscapebg(os.path.expanduser('~/.netscape/liprefs.js'), bg)
except IOError:
    pass
