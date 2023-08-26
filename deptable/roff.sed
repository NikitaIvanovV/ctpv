#!/bin/sed -f

# Add header
1i\
.TS\
allbox;\
lb lb\
l li .\
File type\tPrograms

# Format rows
/^$/! {
  # Remove links
  :a
  s/\[\(\S*\)\]\[\S*\]/\1/
  ta

  # Substitute '-' with '\-'
  :b
  s/\(\t.*[^\\]\)-/\1\\-/
  tb

  # Add data block to enable line wrapping
  s/\t/&T{\n/; s/$/\nT}/
}

# Add footer
$a\
.TE

# Delete links
/^>/d

# Delete empty lines
/^$/d
