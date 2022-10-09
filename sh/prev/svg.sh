# video: convert

video() {
	convert "$f" "jpg:$cache_f"
}

convert_and_show_image video
