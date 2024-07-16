#!/bin/bash

if [[ $# != 3 ]]; then
  echo "Call with 3 arguments"
  echo "$ $0 <png> <rows> <colums>"
  exit 1
fi

spritesheet="$1"
row_count="$2"
col_count="$3"


width=$(identify -format "%w" $1)
height=$(identify -format "%h" $1)

sprite_width=$(($width/$col_count))
sprite_height=$(($height/$row_count))

magick $1 -channel alpha -threshold 50% add-spritesheet-temp-file-alpha-fix.png
magick add-spritesheet-temp-file-alpha-fix.png -crop "${sprite_width}x${sprite_height}" +repage +adjoin add-spritesheet-temp-file-sprites-%03d.png
magick -delay 8 -loop 0 add-spritesheet-temp-file-sprites-* "$(basename $1 .png).gif"
rm add-spritesheet-temp-file-*
