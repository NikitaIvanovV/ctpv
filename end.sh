fifo="$(get_fifo "$1")"

[ -e "$fifo" ] || exit 1

# sending zero byte tells listener to stop
fifo_open "$fifo" && printf '\0' > "$fifo"
