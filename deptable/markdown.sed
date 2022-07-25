#!/bin/sed -f

# Add header
1i\
| File types | Programs |\
| ---- | ---- |

# Format rows
s/ /` `/g
s/\t/ | `/
s/^/| /
s/$/` |/

# Add a newline at the end
$a\

