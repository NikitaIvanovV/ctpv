setup_fifo "$1" 1

# tell ctpv server to exit
printf '\0' > "$fifo"
