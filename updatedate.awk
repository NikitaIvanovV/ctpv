#!/usr/bin/gawk -f

@include "inplace"

BEGIN {
    FS = " "
}

/^\.TH / {
    cmd = "date +%Y-%m-%d"
    cmd | getline $4
    close(cmd)
}

{ print }
