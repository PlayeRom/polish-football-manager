# Manager of the Polish League 2002 - DOS version

Here you find original DOS version of the game.
In the *release-v.1.04* directory are binaries which are still able to run by DOSBox. There simple run a Manager.exe file.

## Interesting facts

The game is splitted into two files: Manager and Match. The reason was limitation of file size created by Borland C++ IDE. I figured out that I have two possibilities to fix this issue. Moved all strings/texts to the separate file (which should offload the exe file) or leave the hardcoded texts in the source code but split the project into two exe files. Unfortunalty I decided to second option :)

In 2002 I haven't idea about data structure like vectors, maps, sets, etc. So my solution for this lack was handle the data by rewriting the files. Instead of keep the all needed data in the RAM, I stored these in the disk files. For this reason the game has a feature of auto saving the game state :)

On the Linux port, these bad solutions have been reduced. The game is contained in one file and the STL is using to storing the data in RAM, reduced disk operations.
