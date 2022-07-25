#!/usr/bin/gawk -f

@include "inplace"

BEGIN {
    while ((getline line < "-") > 0)
        input = input line "\n"
    close("-")
}

/TABLEEND/ {
    table = 0
    printf "%s", input
}

!table

/TABLESTART/ {
    table = 1
}
