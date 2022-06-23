setup_image 1

case "$image_method" in
	"$image_method_ueberzug")
		printf '{"action": "remove", "identifier": "preview"}\n' > "$fifo"
		;;
	"$image_method_kitty")
		kitty +kitten icat --clear --transfer-mode file
		;;
esac
