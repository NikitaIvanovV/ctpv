image_method_ueberzug='U'
image_method_kitty='K'
image_method_chafa='C'

echo_err() {
	echo "$@" >&2
}

exists() {
	command -v "$1" >/dev/null
}

check_exists() {
	exists "$@" || exit 127
}

noimages() {
	[ -n "$noimages" ]
}

is_kitty() {
	[ -n "$KITTY_PID" ]
}

fifo_open() {
	# https://unix.stackexchange.com/a/522940/183147
	dd oflag=nonblock conv=notrunc,nocreat count=0 of="$1" \
		>/dev/null 2>/dev/null
}

set_image_method() {
	image_method=

	[ -n "$forcekitty" ] && is_kitty && { image_method="$image_method_kitty"; return 0; }
	[ -n "$forcechafa" ] && exists chafa && { image_method="$image_method_chafa"; return 0; }

	[ -n "$DISPLAY" ] && [ -z "$WAYLAND_DISPLAY" ] && exists ueberzug &&
		[ -n "$fifo" ] && [ -e "$fifo" ] &&
		{ image_method="$image_method_ueberzug"; return 0; }

	is_kitty && { image_method="$image_method_kitty"; return 0; }

	exists chafa && { image_method="$image_method_chafa"; return 0; }
}

setup_fifo() {
	fifo_open "$fifo" || exit "${1:-127}"
}

setup_image() {
	set_image_method

	[ "$image_method" = "$image_method_ueberzug" ] && setup_fifo "$@"
}

send_image() {
	noimages && return 127

	case "$image_method" in
		"$image_method_ueberzug")
			path="$(printf '%s' "$1" | sed 's/\\/\\\\/g; s/"/\\"/g')"
			printf '{ "action": "add", "identifier": "preview", "x": %d, "y": %d, "width": %d, "height": %d, "scaler": "contain", "scaling_position_x": 0.5, "scaling_position_y": 0.5, "path": "%s"}\n' "$x" "$y" "$w" "$h" "$path" > "$fifo"
			return 1
			;;
		"$image_method_kitty")
			kitty +kitten icat --transfer-mode file --align left \
				--place "${w}x${h}@${x}x${y}" "$1"
			return 1
			;;
		"$image_method_chafa")
			chafa --animate off -s "${w}x${h}" "$1"
			;;
		*)
			return 127
			;;
	esac
}

convert_and_show_image() {
	noimages && return 127
	setup_image
	[ -n "$cache_valid" ] || "$@" || exit "$?"
	send_image "$cache_f"
}
