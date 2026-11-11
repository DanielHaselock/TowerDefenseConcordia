#!/usr/bin/env python3
"""Copies the critter sprite sheet and traces a yellow outline exactly along
each sprite's silhouette, producing the variant shown while a critter is
selected. The sheet's background is the renderer's transparency color key
(32,32,32), so the silhouette is every non-background pixel."""
from PIL import Image

SRC = "Assets/smallSpriteSheetV2.png"
DST = "Assets/selectedSpriteSheetV2.png"
CELLS = [(0, 255), (128, 255), (128, 319)]  # walker, speedy, boss (64x64 cells)
BG = (32, 32, 32, 255)
OUTLINE = (255, 215, 0, 255)
THICKNESS = 4

img = Image.open(SRC).convert("RGBA")
for cx, cy in CELLS:
    cell = img.crop((cx, cy, cx + 64, cy + 64))
    mask = [p != BG for p in cell.getdata()]

    def at(x, y):
        return 0 <= x < 64 and 0 <= y < 64 and mask[y * 64 + x]

    # ring 1: silhouette pixels touching the background
    ring = [(x, y) for y in range(64) for x in range(64)
            if at(x, y) and not (at(x - 1, y) and at(x + 1, y) and at(x, y - 1) and at(x, y + 1))]
    painted = set(ring)
    # ring 2+: pixels adjacent to the previous ring, extending the outline inward
    for _ in range(THICKNESS - 1):
        ring = [(nx, ny) for x, y in ring
                for nx, ny in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1))
                if at(nx, ny) and (nx, ny) not in painted]
        painted.update(ring)

    for x, y in painted:
        cell.putpixel((x, y), OUTLINE)
    img.paste(cell, (cx, cy), cell)
    print(f"cell ({cx},{cy}): {len(painted)} outline pixels")
img.save(DST)
print("wrote", DST)
