#!/bin/sed -f

# Add header
1i\
| File types | Programs |\
| ---- | ---- |

# Format links
/^>/ {
  s/>\t/[/
  s/\t/]: /
  be
}

# Format rows
/^$/! {
  s/ / /g
  s/\t/ | /
  s/^/| /
  s/$/ |/
}

:e

# Add a newline at the end
$a\

