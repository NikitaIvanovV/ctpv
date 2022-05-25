get_fifo() {
	printf '/tmp/ctpvfifo.%s' "$1"
}

exists() {
	command -v "$1" > /dev/null
}

fifo_open() {
	# https://unix.stackexchange.com/a/522940/183147
	dd oflag=nonblock conv=notrunc,nocreat count=0 of="$1" \
		>/dev/null 2>/dev/null
}
