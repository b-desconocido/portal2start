# Portal2Start

Used to launch Portal 2 game on modern CPUs with more than 28 hardware threads. At the time of writing it is impossible to load any level if you have good enough CPU - the game will crash.
This tool simply limits the number of hardware threads reported by Windows and prevents the crash.

# How To Use

1) Try to start p2scn.exe
2) If you got any error - put this exe into the game folder and try to start it again

# Tested on
Xeon E5-2683v4 16C/32T, Windows 10 x86-64 (under WoW64)
