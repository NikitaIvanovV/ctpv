# ctpv

Terminal previewer

![showcase](doc/showcase.gif)

----

ctpv is an utility for previewing various files (including images with [Überzug](https://github.com/seebye/ueberzug)).

ctpv is a remake of awesome program https://github.com/NikitaIvanovV/ctpv [stpv](https://github.com/Naheel-Azawy/stpv)
but it's written in C.
stpv worked perfectly for me, except it was kinda sluggish because
it was written in POSIX shell.
ctpv is an attempt to make a faster version of stpv and add some new features.

Originally it was made for [lf](https://github.com/gokcehan/lf)
but I assume it should not be hard to integrate ctpv into other
programs (if they support external programs for providing previews like lf, of course).

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

Full documentation for the program can be found here: https://nikitaivanovv.github.io/ctpv/
