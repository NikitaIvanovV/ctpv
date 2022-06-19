setup_fifo 1

if use_ueberzug; then
	printf '{"action": "remove", "identifier": "preview"}\n' > "$fifo"
elif use_kitty; then
	kitty +kitten icat --clear --transfer-mode file
fi
