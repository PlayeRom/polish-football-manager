# Manager of the Polish League 2002

## Beginning

In 2002, when I was learing C/C++ programming, I created football manager game for DOS (in Borland C++ compiler).

You can find this original DOS version (sources and binaries) in *dos-original* folder (ugly fromatted and structural code) and you can still run the binaries by DOSBox.

## Porting to Linux

17 years later, when I found my old sources files in backup stored in CD-ROM, I decided to porting this game to Linux console with refactoring the code. In this repo you will find results of my porting.

### Supported langauges

* English (en_US, default)
* Polish (pl_PL)

For run the game with another language than English, add `-l` flag with language code, e.g.

```
./managerligipolskiej2002 -l pl_PL
```

## Images

Determining tactics:

![Tactics](https://github.com/PlayeRom/polish-football-manager/blob/master/linux-port/img/tactics.png)

League table:

![Table](https://github.com/PlayeRom/polish-football-manager/blob/master/linux-port/img/table.png)

End match:

![End match](https://github.com/PlayeRom/polish-football-manager/blob/master/linux-port/img/end-match.png)
