xrdb  -load  $HOME/.Xresources
#
# Reads default keyboard mapping if exists
#
if [ -f /usr/X11/lib/X11/Xmodmap ]; then
	xmodmap /usr/X11/lib/X11/Xmodmap
fi
#
# Then reads user's keyboard mapping if exists
#
if [ -f ~/.Xmodmap ]; then
	xmodmap ~/.Xmodmap
fi
#
# Do we have xbackdrops installed ?
#
if [ -x /usr/X11R6/bin/xbackdrop ]; then
	xbackdrop &
fi
# Or, what about Xearth ?
#if [ -x /usr/X11R6/bin/xearth ]; then
#	xearth &
#fi
# Start up fvwm or fvwm2
exec fvwm -f /var/XFCE/sample.fvwmrc
#exec fvwm2 -f /var/XFCE/sample.fvwm2rc
