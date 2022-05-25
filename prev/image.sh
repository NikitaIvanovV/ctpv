fifo="$(get_fifo "$id")"

# tell ctpv to fallback to another preview
[ -e "$fifo" ] || exit 127

path="$(printf '%s' "$f" | sed 's/\\/\\\\/g; s/"/\\"/g')"

fifo_open "$fifo" && {
	printf '{ "action": "add", "identifier": "preview", "x": %d, "y": %d, "width": %d, "height": %d, "scaler": "contain", "scaling_position_x": 1, "scaling_position_y": 1, "path": "%s"}\n' "$x" "$y" "$w" "$h" "$path" > "$fifo"
}

# tell lf to disable preview caching
exit 1
