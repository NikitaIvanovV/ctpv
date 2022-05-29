setup_fifo "$1" 1

printf '{"action": "remove", "identifier": "preview"}\n' > "$fifo"
