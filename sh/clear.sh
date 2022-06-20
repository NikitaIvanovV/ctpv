setup_fifo 1

if use_kitty; then
	kitty +kitten icat --clear --transfer-mode file
elif use_ueberzug; then
	printf '{"action": "remove", "identifier": "preview"}\n' > "$fifo"
fi
