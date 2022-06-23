setup_image 1

# tell ctpv server to exit
[ "$image_method" = "$image_method_ueberzug" ] && printf '\0' > "$fifo"
