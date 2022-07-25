#!/bin/sed -f

# Add header
1i\
.TS\
allbox;\
lb lb\
l li .\
File type\tPrograms

# Format rows
s/\t/&T{\n/; s/$/\nT}/

# Add footer
$a\
.TE
