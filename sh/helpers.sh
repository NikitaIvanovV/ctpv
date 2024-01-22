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

autochafa() {
	[ -n "$autochafa" ]
}

chafasixel() {
	[ -n "$chafasixel" ]
}

is_kitty() {
	[ -n "$KITTY_PID" ] && return 0

	case "$TERM" in
		*-kitty) return 0 ;;
		*)       return 1 ;;
	esac
}

kitty_clear() {
	kitty +kitten icat --clear --stdin no --silent --transfer-mode file < /dev/null > /dev/tty
}

fifo_open() {
	# https://unix.stackexchange.com/a/522940/183147
	dd oflag=nonblock conv=notrunc,nocreat count=0 of="$1" \
		>/dev/null 2>/dev/null
}

set_image_method() {
	image_method=

	[ -n "$forcekitty" ] && is_kitty && { image_method="$image_method_kitty"; return 0; }
	[ -n "$forcekittyanim" ] && is_kitty && is_anim_image && { image_method="$image_method_kitty"; return 0; }
	[ -n "$forcechafa" ] && exists chafa && { image_method="$image_method_chafa"; return 0; }

	[ -n "$DISPLAY" ] && [ -z "$WAYLAND_DISPLAY" ] && exists ueberzug &&
		[ -n "$fifo" ] && [ -e "$fifo" ] &&
		{ image_method="$image_method_ueberzug"; return 0; }

	is_kitty && { image_method="$image_method_kitty"; return 0; }

	exists chafa && { image_method="$image_method_chafa"; return 0; }
}

is_anim_image() {
	case "$m" in
		image/apng|image/gif|image/avif|image/webp)
			return 0 ;;
		*)
			return 1 ;;
	esac
}

chafa_run() {
	format='-f symbols'
	autochafa && format=
	chafasixel && format='-f sixels --polite on'
	chafa -s "${w}x${h}" $format "$1" | sed 's/#/\n#/g'
}

setup_fifo() {
	fifo_open "$fifo" || exit "${1:-127}"
}

setup_image() {
	set_image_method

	[ "$image_method" = "$image_method_ueberzug" ] && setup_fifo "$@"
}

kitty_icat_pid() {
	printf '/tmp/ctpvicat.%d' "$id"
}

send_image() {
	noimages && return 127

	case "$image_method" in
		"$image_method_ueberzug")
			path="$(printf '%s' "$1" | sed 's/\\/\\\\/g; s/"/\\"/g')"
			printf '{ "action": "add", "identifier": "preview", "x": %d, "y": %d, "width": %d, "height": %d, "scaler": "contain", "scaling_position_x": 0.5, "scaling_position_y": 0.5, "path": "%s" }\n' "$x" "$y" "$w" "$h" "$path" > "$fifo"
			return 1
			;;
		"$image_method_kitty")
			kitty +kitten icat --silent --stdin no --transfer-mode file \
				--place "${w}x${h}@${x}x${y}" "$1" < /dev/null > /dev/tty
			printf '%d\n' "$!" > "$(kitty_icat_pid)"
			wait
			return 1
			;;
		"$image_method_chafa")
			chafa_run "$1"
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
