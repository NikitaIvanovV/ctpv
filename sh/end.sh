setup_image 1

# tell ctpv server to exit
printf '\0' > "$fifo"

# Kill running icat
icat_pid="$(kitty_icat_pid)"
[ -e "$icat_pid" ] && pid="$(cat "$icat_pid")" && [ -e "/proc/$pid" ] && kill "$pid"

# A dirty hack to fix lf issue where ctpv runs before quit
if is_kitty; then
	kitty_clear &
	{ sleep 1; kitty_clear; } &
	wait
fi
