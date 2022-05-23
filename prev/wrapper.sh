[ -L "$f" ] && printf 'Symlink: \n%s\n\n' "$(readlink "$f")"

# Pretend that preview failed so another is run
exit 127
