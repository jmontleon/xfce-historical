#!/bin/sh
#
#
# Start-up stuff from ~/Desktop/Autostart (as it seems to be the new standard)
#
for i in `ls ${HOME}/Desktop/Autostart/`; do
  if test -x $HOME/Desktop/Autostart/$i; then
     $HOME/Desktop/Autostart/$i &
  fi
done
#
# launch xscreensaver (if available)
#
xscreensaver  -no-splash -lock-mode &
#
# And launch XFce window manager
#
exec xfwm
