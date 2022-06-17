check_exists atool

atool -l -- "$f" | cut -d ' ' -f 6-
