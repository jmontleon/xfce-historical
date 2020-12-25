# Keyboard stuff and user preferences...
#
# Reads default keyboard mapping if exists
#
#if [ -f /usr/X11R6/lib/X11/Xmodmap ]; then
#	xmodmap /usr/X11R6/lib/X11/Xmodmap
#fi
#
# Then reads user's keyboard mapping if exists
#
#if [ -f ~/.Xmodmap ]; then
#	xmodmap ~/.Xmodmap
#fi

# Which language do you wish to use (user configurable) ?
#
#export XFCE_LANG="fr"
export XFCE_LANG="en"
#export XFCE_LANG="it"
#export XFCE_LANG="de"
#export XFCE_LANG="hu"
#export XFCE_LANG="es"
#export XFCE_LANG="sk"
#export XFCE_LANG="pt"

# Uncomment one of the following lines if you still want to use fvwm or fvwm2
#
#exec fvwm -f /var/XFCE/sample.fvwmrc
#exec fvwm2 -f /var/XFCE/sample.fvwm2rc

# Offical WM for XFce is now XFwm ! GO FOR IT ;-))
#
exec xfwm
