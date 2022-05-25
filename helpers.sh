fifo_open() {
	# https://unix.stackexchange.com/a/522940/183147
	dd oflag=nonblock conv=notrunc,nocreat count=0 of="$1" \
		>/dev/null 2>/dev/null
}

setup_fifo() {
	exit_code="${2:-127}"
	fifo="$(printf '/tmp/ctpvfifo.%s' "${1:-$id}")"
	[ -e "$fifo" ] || exit "$exit_code"
	fifo_open "$fifo" || exit "$exit_code"
}

exists() {
	command -v "$1" > /dev/null
}

check_exist() {
	[ $? = 127 ] && exit 127
}

cache() {
	cache_d="${XDG_CACHE_HOME:-$HOME/.cache}/ctpv"
	mkdir -p "$cache_d"
	cache_f="$cache_d/$(printf '%s' "$f" | md5sum - | cut -d' ' -f1)"
	test -e "$cache_f" || return
	ctpv -n "$cache_f" "$f"
}

show_image() {
	path="$(printf '%s' "$1" | sed 's/\\/\\\\/g; s/"/\\"/g')"
	printf '{ "action": "add", "identifier": "preview", "x": %d, "y": %d, "width": %d, "height": %d, "scaler": "contain", "scaling_position_x": 0.5, "scaling_position_y": 0.5, "path": "%s"}\n' "$x" "$y" "$w" "$h" "$path" > "$fifo"
}

convert_and_show_image() {
	setup_fifo
	cache || "$@" || check_exist
	show_image "$cache_f"
	exit 1
}
