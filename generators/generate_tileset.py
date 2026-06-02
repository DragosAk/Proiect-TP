from PIL import Image, ImageDraw
import random

TILE_SIZE = 64
GRID = 4
IMG_SIZE = TILE_SIZE * GRID

# ── Paletă de culori suprasaturată (Stil Stardew) ──────────────────────────
DIRT_BASE      = (139, 69, 19)   # SaddleBrown
DIRT_DARK      = (92, 46, 13)
DIRT_LIGHT     = (184, 115, 51)  # Copper/Warm

WET_BASE       = (80, 40, 10)
WET_DARK       = (50, 20, 5)
WET_LIGHT      = (120, 60, 20)

STONE_BASE     = (140, 145, 150)
STONE_DARK     = (90, 95, 100)
STONE_LIGHT    = (190, 195, 200)

WOOD_BASE      = (205, 133, 63)  # Peru / Warm Wood
WOOD_DARK      = (139, 69, 19)
WOOD_LIGHT     = (222, 184, 135) # Burlywood

GREEN_LIGHT    = (124, 252, 0)   # LawnGreen
GREEN_MID      = (50, 205, 50)   # LimeGreen
GREEN_DARK     = (0, 100, 0)     # DarkGreen
GREEN_OUTLINE  = (0, 50, 0)

WHEAT_GOLD     = (255, 215, 0)   # Gold
WHEAT_DARK     = (218, 165, 32)  # GoldenRod
WHEAT_OUTLINE  = (139, 101, 8)

CARROT_ORANGE  = (255, 140, 0)   # DarkOrange
CARROT_DARK    = (205, 92, 0)
CARROT_OUTLINE = (139, 69, 0)

PUMPKIN_ORANGE = (255, 117, 24)  # Pumpkin
PUMPKIN_DARK   = (210, 80, 0)
PUMPKIN_OUTLINE= (120, 40, 0)

SHADOW         = (70, 30, 5)     # Umbră falsă aplicată peste pământ

img = Image.new("RGB", (IMG_SIZE, IMG_SIZE), (0, 0, 0))
draw = ImageDraw.Draw(img)
rng = random.Random(42)

# ── 1. Generarea pământului de bază (Buffer) ────────────────────────────────
base_dirt = Image.new("RGB", (TILE_SIZE, TILE_SIZE), DIRT_BASE)
bd_draw = ImageDraw.Draw(base_dirt)

# Zgomot organic (pete, nu pixeli individuali pentru un look "chunky")
for _ in range(80):
    x = rng.randint(0, TILE_SIZE - 2)
    y = rng.randint(0, TILE_SIZE - 2)
    c = rng.choice([DIRT_DARK, DIRT_LIGHT])
    s = rng.choice([1, 2, 3]) # Dimensiunea petei
    bd_draw.rectangle([x, y, x+s, y+s], fill=c)

# Helper pentru aplicarea fundalului pe un tile
def paste_dirt(x0, y0):
    img.paste(base_dirt, (x0, y0))

# Helper pentru desenat umbre
def draw_shadow(d, cx, cy, radius_x, radius_y):
    d.ellipse([cx - radius_x, cy - radius_y, cx + radius_x, cy + radius_y], fill=SHADOW)

# ── RÂND 0: Teren ───────────────────────────────────────────────────────────
def tile_dirt_plain(d, ox, oy):
    paste_dirt(ox, oy)

def tile_dirt_tilled(d, ox, oy):
    for y in range(oy, oy + TILE_SIZE):
        for x in range(ox, ox + TILE_SIZE):
            img.putpixel((x, y), WET_BASE)
    
    # Desenăm 3 brazde mari și groase
    for i in range(3):
        y_center = oy + 12 + i * 20
        # Umbra brazdei (șanțul)
        d.rectangle([ox, y_center + 6, ox + TILE_SIZE, y_center + 10], fill=WET_DARK)
        # Highlight (partea de sus a brazdei)
        d.rectangle([ox, y_center - 2, ox + TILE_SIZE, y_center + 2], fill=WET_LIGHT)

def tile_path_stone(d, ox, oy):
    d.rectangle([ox, oy, ox + TILE_SIZE, oy + TILE_SIZE], fill=DIRT_DARK)
    # Generăm pietre mari rotunjite
    for _ in range(12):
        px = rng.randint(ox + 2, ox + TILE_SIZE - 16)
        py = rng.randint(oy + 2, oy + TILE_SIZE - 16)
        pw, ph = rng.randint(8, 18), rng.randint(8, 14)
        
        # Contur Piatră
        d.ellipse([px-1, py-1, px+pw+1, py+ph+1], fill=STONE_DARK)
        # Piatra
        d.ellipse([px, py, px+pw, py+ph], fill=STONE_BASE)
        # Highlight piatră (sus-stânga)
        d.ellipse([px+1, py+1, px+pw-4, py+ph-4], fill=STONE_LIGHT)

def tile_wood_floor(d, ox, oy):
    d.rectangle([ox, oy, ox + TILE_SIZE, oy + TILE_SIZE], fill=WOOD_BASE)
    # Desenăm scânduri orizontale groase
    plank_h = 16
    for i in range(4):
        py = oy + i * plank_h
        # Contur între scânduri
        d.line([(ox, py), (ox + TILE_SIZE, py)], fill=WOOD_DARK, width=2)
        # Highlight scândură
        d.line([(ox, py+2), (ox + TILE_SIZE, py+2)], fill=WOOD_LIGHT, width=1)
        # Noduri de lemn și cuie
        if rng.random() > 0.3:
            nx = ox + rng.randint(10, TILE_SIZE - 10)
            d.ellipse([nx, py+4, nx+4, py+8], fill=WOOD_DARK) # Nod
        d.point((ox + 4, py + 4), fill=DIRT_DARK) # Cui stânga
        d.point((ox + TILE_SIZE - 4, py + 4), fill=DIRT_DARK) # Cui dreapta

# ── RÂND 1: Grâu ────────────────────────────────────────────────────────────
def tile_wheat_seed(d, ox, oy):
    paste_dirt(ox, oy)
    cx, cy = ox + 32, oy + 32
    d.rectangle([cx-4, cy-4, cx+4, cy+4], fill=WHEAT_DARK)
    d.rectangle([cx-2, cy-2, cx+2, cy+2], fill=WHEAT_GOLD)

def tile_wheat_growing(d, ox, oy):
    paste_dirt(ox, oy)
    cx, cy = ox + 32, oy + 48
    draw_shadow(d, cx, cy, 6, 2)
    # Lăstar gros cu contur
    d.line([(cx-1, cy), (cx-1, cy-14)], fill=GREEN_OUTLINE, width=4)
    d.line([(cx, cy), (cx, cy-14)], fill=GREEN_MID, width=2)

def tile_wheat_almost(d, ox, oy):
    paste_dirt(ox, oy)
    cx, cy = ox + 32, oy + 48
    draw_shadow(d, cx, cy, 10, 3)
    # Tulpină mai groasă, galben-verzuie
    for off in [-4, 0, 4]:
        d.line([(cx+off, cy), (cx+off, cy-20)], fill=GREEN_OUTLINE, width=3)
        d.line([(cx+off, cy), (cx+off, cy-20)], fill=WHEAT_DARK, width=1)

def tile_wheat_mature(d, ox, oy):
    paste_dirt(ox, oy)
    cx, cy = ox + 32, oy + 50
    draw_shadow(d, cx, cy, 14, 4)
    # Tufă de grâu aurie
    for off in range(-12, 13, 6):
        h = rng.randint(24, 34)
        # Contur tulpină
        d.line([(cx+off, cy), (cx+off, cy-h)], fill=WHEAT_OUTLINE, width=3)
        # Interior tulpină
        d.line([(cx+off, cy), (cx+off, cy-h)], fill=WHEAT_GOLD, width=1)
        # Spic de grâu (elipse pentru volum)
        d.ellipse([cx+off-3, cy-h-8, cx+off+3, cy-h+2], fill=WHEAT_DARK)
        d.ellipse([cx+off-2, cy-h-7, cx+off+2, cy-h+1], fill=WHEAT_GOLD)

# ── RÂND 2: Morcov ──────────────────────────────────────────────────────────
def tile_carrot_seed(d, ox, oy):
    paste_dirt(ox, oy)
    cx, cy = ox + 32, oy + 32
    d.ellipse([cx-3, cy-3, cx+3, cy+3], fill=CARROT_DARK)

def tile_carrot_growing(d, ox, oy):
    paste_dirt(ox, oy)
    cx, cy = ox + 32, oy + 46
    draw_shadow(d, cx, cy, 6, 2)
    # Tulpini curbate
    d.arc([cx-10, cy-15, cx+5, cy], 0, 90, fill=GREEN_OUTLINE, width=3)
    d.arc([cx-10, cy-15, cx+5, cy], 0, 90, fill=GREEN_MID, width=1)

def tile_carrot_almost(d, ox, oy):
    paste_dirt(ox, oy)
    cx, cy = ox + 32, oy + 46
    draw_shadow(d, cx, cy, 10, 3)
    # Frunze mari
    d.ellipse([cx-12, cy-20, cx, cy-8], fill=GREEN_DARK, outline=GREEN_OUTLINE)
    d.ellipse([cx, cy-22, cx+10, cy-10], fill=GREEN_MID, outline=GREEN_OUTLINE)
    # Bază portocalie vizibilă
    d.ellipse([cx-4, cy-4, cx+4, cy+2], fill=CARROT_DARK)
    d.ellipse([cx-2, cy-2, cx+2, cy], fill=CARROT_ORANGE)

def tile_carrot_mature(d, ox, oy):
    paste_dirt(ox, oy)
    cx, cy = ox + 32, oy + 46
    draw_shadow(d, cx, cy, 12, 4)
    # Morcovi ieșiți (2 bucăți)
    d.ellipse([cx-10, cy-12, cx-2, cy+4], fill=CARROT_ORANGE, outline=CARROT_OUTLINE)
    d.ellipse([cx+2, cy-14, cx+10, cy+2], fill=CARROT_ORANGE, outline=CARROT_OUTLINE)
    # Frunze stufoase peste morcovi
    d.ellipse([cx-16, cy-28, cx-4, cy-10], fill=GREEN_MID, outline=GREEN_OUTLINE)
    d.ellipse([cx+4, cy-30, cx+16, cy-12], fill=GREEN_MID, outline=GREEN_OUTLINE)
    d.ellipse([cx-6, cy-34, cx+6, cy-14], fill=GREEN_LIGHT, outline=GREEN_OUTLINE)

# ── RÂND 3: Dovleac ─────────────────────────────────────────────────────────
def tile_pumpkin_seed(d, ox, oy):
    paste_dirt(ox, oy)
    cx, cy = ox + 32, oy + 32
    d.ellipse([cx-4, cy-2, cx+4, cy+2], fill=(220, 200, 150), outline=STONE_DARK)

def tile_pumpkin_growing(d, ox, oy):
    paste_dirt(ox, oy)
    cx, cy = ox + 32, oy + 44
    draw_shadow(d, cx, cy, 8, 3)
    # Frunze pe pământ
    d.ellipse([cx-12, cy-8, cx-2, cy], fill=GREEN_MID, outline=GREEN_OUTLINE)
    d.ellipse([cx+2, cy-10, cx+12, cy-2], fill=GREEN_MID, outline=GREEN_OUTLINE)

def tile_pumpkin_almost(d, ox, oy):
    paste_dirt(ox, oy)
    cx, cy = ox + 32, oy + 46
    draw_shadow(d, cx, cy, 18, 5)
    # Vrej întins
    d.line([(cx-20, cy), (cx+20, cy)], fill=GREEN_OUTLINE, width=4)
    d.line([(cx-20, cy), (cx+20, cy)], fill=GREEN_DARK, width=2)
    # Frunze mari rotunde
    d.ellipse([cx-22, cy-12, cx-6, cy+4], fill=GREEN_MID, outline=GREEN_OUTLINE)
    d.ellipse([cx+6, cy-14, cx+24, cy+2], fill=GREEN_MID, outline=GREEN_OUTLINE)

def tile_pumpkin_mature(d, ox, oy):
    paste_dirt(ox, oy)
    cx, cy = ox + 32, oy + 46
    draw_shadow(d, cx, cy, 22, 6)
    
    # Frunze de fundal
    d.ellipse([cx-26, cy-10, cx-10, cy+6], fill=GREEN_DARK, outline=GREEN_OUTLINE)
    d.ellipse([cx+10, cy-12, cx+26, cy+4], fill=GREEN_DARK, outline=GREEN_OUTLINE)

    # Dovleac (3 elipse suprapuse pentru un look segmentat, chunky)
    body_y = cy - 6
    # Segment stânga și dreapta
    d.ellipse([cx-18, body_y-12, cx-2, body_y+12], fill=PUMPKIN_DARK, outline=PUMPKIN_OUTLINE)
    d.ellipse([cx+2, body_y-12, cx+18, body_y+12], fill=PUMPKIN_DARK, outline=PUMPKIN_OUTLINE)
    # Segment centru (peste celelalte)
    d.ellipse([cx-10, body_y-14, cx+10, body_y+14], fill=PUMPKIN_ORANGE, outline=PUMPKIN_OUTLINE)
    
    # Highlight pe segmentul central
    d.ellipse([cx-4, body_y-10, cx+2, body_y+4], fill=(255, 180, 80))
    
    # Codiță groasă
    d.rectangle([cx-2, body_y-20, cx+2, body_y-12], fill=PUMPKIN_OUTLINE)
    d.rectangle([cx-1, body_y-19, cx+1, body_y-13], fill=GREEN_DARK)

funcs = [
    [tile_dirt_plain, tile_dirt_tilled, tile_path_stone, tile_wood_floor],
    [tile_wheat_seed, tile_wheat_growing, tile_wheat_almost, tile_wheat_mature],
    [tile_carrot_seed, tile_carrot_growing, tile_carrot_almost, tile_carrot_mature],
    [tile_pumpkin_seed, tile_pumpkin_growing, tile_pumpkin_almost, tile_pumpkin_mature]
]

for ry, row in enumerate(funcs):
    for rx, fn in enumerate(row):
        fn(draw, rx * TILE_SIZE, ry * TILE_SIZE)

img.save("tileset_stardew.png")