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
# Start the window manager 
mwm&
# Start "xfbd" for backdrop management
xfbd&
# And at last, start "xfce"
xfce
