#!/usr/bin/env python3
"""
Convert an image (PNG/JPG) to raw RGB565 little-endian suitable for the XIAO example.
Usage:
    python tools/to_rgb565.py input.jpg output.rgb565 --width 160 --height 120
"""
import sys
from PIL import Image
import argparse

def to_rgb565(infile, outfile, w, h):
    im = Image.open(infile).convert('RGB').resize((w,h))
    with open(outfile, 'wb') as f:
        for y in range(h):
            for x in range(w):
                r,g,b = im.getpixel((x,y))
                r5 = (r >> 3) & 0x1F
                g6 = (g >> 2) & 0x3F
                b5 = (b >> 3) & 0x1F
                rgb565 = (r5 << 11) | (g6 << 5) | b5
                f.write(bytes([rgb565 & 0xFF, (rgb565 >> 8) & 0xFF]))

if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('input')
    p.add_argument('output')
    p.add_argument('--width', type=int, default=160)
    p.add_argument('--height', type=int, default=120)
    args = p.parse_args()
    to_rgb565(args.input, args.output, args.width, args.height)
    print(f'Wrote {args.output} ({args.width}x{args.height} rgb565)')
