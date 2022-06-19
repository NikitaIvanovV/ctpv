setup_fifo 1

# tell ctpv server to exit
use_ueberzug && printf '\0' > "$fifo"
