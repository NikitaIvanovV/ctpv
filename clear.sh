fifo="$(get_fifo "$id")"

[ -e "$fifo" ] || exit 1

fifo_open "$fifo" && {
	printf '{"action": "remove", "identifier": "preview"}\n' > "$fifo"
}
