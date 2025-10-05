#!/usr/bin/env python3
"""
Convert raw RGB565 little-endian to PNG (inverse of to_rgb565)
Usage:
    python tools/rgb565_to_png.py input.rgb565 output.png --width 160 --height 120
"""
from PIL import Image
import argparse

def rgb565_to_png(infile, outfile, w, h):
    with open(infile,'rb') as f:
        data = f.read()
    im = Image.new('RGB', (w,h))
    i = 0
    for y in range(h):
        for x in range(w):
            lo = data[i]; hi = data[i+1]; i+=2
            pix = lo | (hi<<8)
            r = ((pix >> 11) & 0x1F) << 3
            g = ((pix >> 5) & 0x3F) << 2
            b = (pix & 0x1F) << 3
            im.putpixel((x,y),(r,g,b))
    im.save(outfile)
    print(f'Wrote {outfile} ({w}x{h})')

if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('input')
    p.add_argument('output')
    p.add_argument('--width', type=int, default=160)
    p.add_argument('--height', type=int, default=120)
    args = p.parse_args()
    rgb565_to_png(args.input, args.output, args.width, args.height)
