# ctpv

File previewer for a terminal

![showcase](doc/showcase.gif)

----

ctpv is a file previewer utility for a terminal.

It was made with integration into [lf file manager][lf] in mind,
but I believe that it can be easily integrated into other programs
as well.

It supports previews for source code, archives, PDF files, images,
videos, etc.
See [Previews](#previews) for more info.

Image previews are powered by one of these programs:

* [Überzug][ueberzug] (X11 only)
* [Chafa][chafa] (X11 and Wayland)
* [Kitty terminal][kitty]

ctpv is a remake of an awesome program named
[stpv](https://github.com/Naheel-Azawy/stpv).
stpv did everything I wanted, except it was a bit sluggish because
it was written in POSIX shell.
ctpv is written in C and is an attempt to make a faster version of
stpv with a few new features.

## Previews

Previewing each file type requires specific programs installed on
a system.
If a program is not found on the system, ctpv
will try to use another one.
Only one program is required for each file type.
For example, you only need either `elinks`, `lynx` or
`w3m` installed on your system to view HTML files.

<!-- This table is auto generated! -->
<!--TABLESTART-->
| File types | Programs |
| ---- | ---- |
| any | [exiftool][exiftool] cat |
| archive | [atool][atool] |
| audio | [ffmpegthumbnailer][ffmpegthumbnailer] [ffmpeg][ffmpeg] |
| diff | [colordiff][colordiff] [delta][delta] [diff-so-fancy][diff-so-fancy] |
| directory | ls |
| font | fontimage |
| gpg-encrypted | [gpg][gpg] |
| html | [elinks][elinks] [lynx][lynx] [w3m][w3m] |
| image | [ueberzug][ueberzug] [chafa][chafa] |
| json | [jq][jq] |
| markdown | [glow][glow] [mdcat][mdcat] |
| office | [libreoffice][libreoffice] |
| pdf | pdftoppm |
| svg | convert |
| text | bat cat [highlight][highlight] [source-highlight][source-highlight] |
| torrent | transmission-show |
| video | [ffmpegthumbnailer][ffmpegthumbnailer] |

[ffmpegthumbnailer]: https://github.com/dirkvdb/ffmpegthumbnailer
[w3m]: https://w3m.sourceforge.net/
[elinks]: http://elinks.cz/
[fontforge]: https://fontforge.org
[exiftool]: https://github.com/exiftool/exiftool
[highlight]: https://gitlab.com/saalen/highlight
[chafa]: https://github.com/hpjansson/chafa
[gpg]: https://www.gnupg.org/
[transmission]: https://transmissionbt.com/
[delta]: https://github.com/dandavison/delta
[colordiff]: https://www.colordiff.org/
[source-highlight]: https://www.gnu.org/software/src-highlite/
[ueberzug]: https://github.com/seebye/ueberzug
[mdcat]: https://github.com/swsnr/mdcat
[glow]: https://github.com/charmbracelet/glow
[atool]: https://www.nongnu.org/atool/
[lynx]: https://github.com/jpanther/lynx
[libreoffice]: https://www.libreoffice.org/
[diff-so-fancy]: https://github.com/so-fancy/diff-so-fancy
[imagemagick]: https://imagemagick.org/
[poppler]: https://poppler.freedesktop.org/
[jq]: https://github.com/jqlang/jq
[ffmpeg]: https://ffmpeg.org/

<!--TABLEEND-->

## Installation

### Manual

If you are building from source, make sure to install these libraries!
Depending on your system, you probably will also need "devel" versions
of the same libraries.

* `libcrypto`
* `libmagic`

Install:

```console
git clone https://github.com/NikitaIvanovV/ctpv
cd ctpv
make
sudo make install
```

Uninstall:

```console
sudo make uninstall
```

### AUR

If you are an Arch Linux user, you can install
[`ctpv-git`](https://aur.archlinux.org/packages/ctpv-git)
AUR package.

```console
yay -S ctpv-git
```

### MacPorts

With MacPorts, you can install the
[`ctpv`](https://ports.macports.org/port/ctpv)
package.

```console
sudo port install ctpv
```

### Homebrew

With Homebrew, you can install the
[`ctpv`](https://formulae.brew.sh/formula/ctpv)
package.

```console
brew install ctpv
```

### Nix

#### Nix package

```console
nix-env -ivf cptv
nix profile install nixpkgs#cptv # with flakes enabled
```

#### NixOS and HomeManager

If you don't need to call it directly and
just want to use it through lf:

```nix
programs.lf = {
  previewer = {
    keybinding = "i";
    source = "${pkgs.ctpv}/bin/ctpv";
  };
  extraConfig = ''
    &${pkgs.ctpv}/bin/ctpv -s $id
    cmd on-quit %${pkgs.ctpv}/bin/ctpv -e $id
    set cleaner ${pkgs.ctpv}/bin/ctpvclear
  '';
}
```

### Gentoo
Add this
[ctpv-9999.ebuild](https://github.com/Sneethe/sneethe-overlay/blob/main/app-misc/ctpv/ctpv-9999.ebuild)
to your own
[repository](https://wiki.gentoo.org/wiki/Creating_an_ebuild_repository).

Or alternatively:

```console
eselect repository add sneethe-overlay git https://github.com/Sneethe/sneethe-overlay.git
emaint sync --repo sneethe-overlay
emerge --ask --verbose app-misc/ctpv
```

## Integration

### lf file manager

Add these lines to your lf config
(usually located at `~/.config/lf/lfrc`).

```
set previewer ctpv
set cleaner ctpvclear
&ctpv -s $id
&ctpvquit $id
```

#### Wayland

If you use Wayland, follow these steps:

* Make sure you use one of the [terminals that support sixel][sixel]
* Install [this fork of lf][lf-sixel]
* Install [Chafa][chafa]
* Add `set chafasixel` to `~/.config/ctpv/config`

As of 2023-03-19, original lf does not support sixel protocol,
which is why you need use the fork.

## Documentation

Full documentation on command line options,
configuration and how to define custom previews can be found here:
<https://www.nikitaivanov.com/man1/ctpv>

[ueberzug]: https://github.com/seebye/ueberzug
[kitty]: https://github.com/kovidgoyal/kitty
[chafa]: https://github.com/hpjansson/chafa
[lf]: https://github.com/gokcehan/lf
[lf-sixel]: https://github.com/horriblename/lf
[sixel]: https://www.arewesixelyet.com
