#!/bin/bash

if [[ $# != 1 ]]; then
  echo "Call with 1 arguments"
  echo "$ $0 <png>"
  exit 1
fi

magick $1 -channel alpha -threshold 50% clean-png-temp-file-alpha-fix.png
mv "$1" "$(basename $1 .png)-original.png"
magick clean-png-temp-file-alpha-fix.png -crop "${sprite_width}x${sprite_height}" +repage +adjoin clean-png-temp-file-sprites-%03d.png
magick -delay 8 -loop 0 clean-png-temp-file-sprites-* "$1"
rm clean-png-temp-file-*
