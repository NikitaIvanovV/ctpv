# ctpv

Terminal previewer

![showcase](doc/showcase.gif)

----

ctpv is an utility for previewing various files
(including images with [Überzug](https://github.com/seebye/ueberzug)).

ctpv is a remake of an awesome program named
[stpv](https://github.com/Naheel-Azawy/stpv) written in C.
stpv worked perfectly for me, except it was kinda sluggish because
it was written in POSIX shell.
ctpv is an attempt to make a faster version of stpv and add some
new features.

Originally it was made for [lf](https://github.com/gokcehan/lf)
file manager but I believe that it can be easily integrated into
other programs as well if they support previews provided by
external programs like lf does.

## Installation

### Manual

```sh
git clone https://github.com/NikitaIvanovV/ctpv
cd ctpv
sudo make install
```

Uninstall with `sudo make uninstall`

## Integration

### lf file manager

Add these lines to your lf config
(usually located at `~/.config/lf/lfrc`):

```
set previewer ctpv
set cleaner ctpvclear
&ctpv -s $id
cmd on-quit $ctpv -e $id
```

## Documentation

Full documentation for the program can be found here:
https://nikitaivanovv.github.io/ctpv/
