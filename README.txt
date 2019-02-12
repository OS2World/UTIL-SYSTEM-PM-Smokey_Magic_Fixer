SMFCODE: A program to help you work with the PM font system.

Smf was written in order to dynamically set the FATTRS structure
prior to GpiCreateLogFont(), to change the CHARBUNDLE of a PS, and
to see the resulting FONTMETRICS and output.  As such it is intended
to be a straightforward layer between the user and the font display
system. It does not protect against system R.I.P.s or ensure logical
results.

To use it for the first time, hit each of the three buttons starting
at the bottom and working up. <Select Font> will bring up a screen
listing all of the currently installed public fonts. Select one of
them with the mouse and the left button. This will set the face name
and lMatch values in the FATTRS structure. <Create Logical Font> will
take these FATTRS values and create a logical font which is displayed
in the upper left hand grey window. The FONTMETRICS values for this
logical font are displayed in the scrollable window below it.
<Set Char Bundle> will read the values out of the char bundle window
and set them for the PS used in the grey test window. The user may
change the values in the FATTRS or CHARBUNDLE window at any time.
This only has an effect, however, when one of the two afore mentioned
buttons are pressed.
