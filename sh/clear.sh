setup_image 1

[ "$image_method" = "$image_method_ueberzug" ] &&
	printf '{"action": "remove", "identifier": "preview"}\n' > "$fifo"

is_kitty && kitty_clear
