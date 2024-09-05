# svg: convert

svg() {
	magick "$f" "jpg:$cache_f"
}

convert_and_show_image svg
