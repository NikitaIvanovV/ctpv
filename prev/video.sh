# video: ffmpegthumbnailer

video() {
	ffmpegthumbnailer -i "$f" -o "$cache_f" -s 0 -t 50% 2>/dev/null
}

convert_and_show_image video
