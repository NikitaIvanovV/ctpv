echo_err() {
	echo "$@" >&2
}

is_kitty() {
	[ -n "$KITTY_PID" ]
}

use_ueberzug() {
	exists ueberzug
}

use_kitty() {
	[ -z "$forcekitty" ] && use_ueberzug && return 1
	is_kitty
}

noimages() {
	[ -n "$noimages" ]
}

fifo_open() {
	# https://unix.stackexchange.com/a/522940/183147
	dd oflag=nonblock conv=notrunc,nocreat count=0 of="$1" \
		>/dev/null 2>/dev/null
}

setup_fifo() {
	use_ueberzug || return 1

	exit_code="${1:-127}"
	[ -n "$fifo" ] || exit "$exit_code"
	[ -e "$fifo" ] || exit "$exit_code"
	fifo_open "$fifo" || exit "$exit_code"
}

exists() {
	command -v "$1" >/dev/null
}

check_exists() {
	exists "$@" || exit 127
}

send_image() {
	noimages && return 127

	if use_kitty; then
		kitty +kitten icat --transfer-mode file --align left \
			--place "${w}x${h}@${x}x${y}" "$1"
		return 1
	elif use_ueberzug; then
		path="$(printf '%s' "$1" | sed 's/\\/\\\\/g; s/"/\\"/g')"
		printf '{ "action": "add", "identifier": "preview", "x": %d, "y": %d, "width": %d, "height": %d, "scaler": "contain", "scaling_position_x": 0.5, "scaling_position_y": 0.5, "path": "%s"}\n' "$x" "$y" "$w" "$h" "$path" > "$fifo"
		return 1
	else
		return 127
	fi
}

convert_and_show_image() {
	noimages && exit 127
	setup_fifo
	[ -n "$cache_valid" ] || "$@" || exit "$?"
	send_image "$cache_f"
}
