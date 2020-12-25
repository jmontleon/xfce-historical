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
# Uncoment one of the following lines if you want fvwm or fvwm2 8-((
#exec fvwm -f /var/XFCE/sample.fvwmrc
#exec fvwm2 -f /var/XFCE/sample.fvwm2rc

# Offical WM for XFce is now XFwm ! GO FOR IT ;-))
exec xfwm
