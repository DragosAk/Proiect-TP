from PIL import Image, ImageDraw

# Dimensiunile cerute de engine-ul jocului tau
TILE_SIZE = 64
COLS = 4
ROWS = 4
IMG_WIDTH = TILE_SIZE * COLS
IMG_HEIGHT = TILE_SIZE * ROWS

# Paleta de culori
COLOR_SKIN = (255, 204, 153)
COLOR_OVERALLS = (30, 80, 200)
COLOR_SHIRT = (200, 50, 50)
COLOR_HAT = (240, 200, 50)
COLOR_BOOTS = (100, 50, 20)
COLOR_EYE = (0, 0, 0)

def draw_farmer(draw, cx, cy, direction, frame):
    """
    Deseneaza fermierul centrat in (cx, cy)
    direction: 0=Jos, 1=Sus, 2=Dreapta, 3=Stanga
    frame: 0=Stand, 1=Pas Drept, 2=Stand, 3=Pas Stang
    """
    
    # 1. Calcularea "Bobbing-ului" (oscilatia pe axa Y in timpul mersului)
    # Cand jucatorul paseste (cadrele 1 si 3), corpul coboara putin.
    bob_y = 2 if frame % 2 != 0 else 0
    
    # 2. Desenarea Picioarelor (cu logica de miscare animata)
    leg_l_y, leg_r_y = cy + 15 + bob_y, cy + 15 + bob_y
    if frame == 1: leg_l_y -= 4  # Ridica piciorul stang
    if frame == 3: leg_r_y -= 4  # Ridica piciorul drept
    
    # Daca merge pe laterala, picioarele se suprapun diferit
    if direction == 2: # Dreapta
        draw.rectangle([cx-6, leg_l_y, cx-2, leg_l_y+8], fill=COLOR_BOOTS) # Picior spate
        draw.rectangle([cx+2, leg_r_y, cx+6, leg_r_y+8], fill=COLOR_BOOTS) # Picior fata
    elif direction == 3: # Stanga
        draw.rectangle([cx+2, leg_r_y, cx+6, leg_r_y+8], fill=COLOR_BOOTS) # Picior spate
        draw.rectangle([cx-6, leg_l_y, cx-2, leg_l_y+8], fill=COLOR_BOOTS) # Picior fata
    else: # Sus / Jos
        draw.rectangle([cx-8, leg_l_y, cx-4, leg_l_y+8], fill=COLOR_BOOTS)
        draw.rectangle([cx+4, leg_r_y, cx+8, leg_r_y+8], fill=COLOR_BOOTS)

    # 3. Desenarea Corpului (Salopeta si camasa)
    draw.rectangle([cx-10, cy-5+bob_y, cx+10, cy+15+bob_y], fill=COLOR_OVERALLS)
    draw.rectangle([cx-10, cy-12+bob_y, cx+10, cy-5+bob_y], fill=COLOR_SHIRT)
    
    # Bratele (balansare inversa fata de picioare)
    arm_l_y, arm_r_y = cy-5+bob_y, cy-5+bob_y
    if frame == 1: arm_l_y += 4; arm_r_y -= 4
    if frame == 3: arm_l_y -= 4; arm_r_y += 4
    
    if direction in [0, 1]: # Jos sau Sus
        draw.rectangle([cx-14, arm_l_y, cx-10, arm_l_y+10], fill=COLOR_SKIN)
        draw.rectangle([cx+10, arm_r_y, cx+14, arm_r_y+10], fill=COLOR_SKIN)
    elif direction == 2: # Dreapta
        draw.rectangle([cx-2, arm_r_y, cx+2, arm_r_y+10], fill=COLOR_SKIN)
    elif direction == 3: # Stanga
        draw.rectangle([cx-2, arm_l_y, cx+2, arm_l_y+10], fill=COLOR_SKIN)

    # 4. Capul si Palaria
    draw.ellipse([cx-10, cy-24+bob_y, cx+10, cy-6+bob_y], fill=COLOR_SKIN) # Fata
    
    if direction == 0: # Fata
        draw.rectangle([cx-4, cy-18+bob_y, cx-2, cy-16+bob_y], fill=COLOR_EYE)
        draw.rectangle([cx+2, cy-18+bob_y, cx+4, cy-16+bob_y], fill=COLOR_EYE)
    elif direction == 2: # Dreapta
        draw.rectangle([cx+4, cy-18+bob_y, cx+6, cy-16+bob_y], fill=COLOR_EYE)
    elif direction == 3: # Stanga
        draw.rectangle([cx-6, cy-18+bob_y, cx-4, cy-16+bob_y], fill=COLOR_EYE)
        
    # Palarie de paie (bor si calota)
    draw.ellipse([cx-18, cy-26+bob_y, cx+18, cy-18+bob_y], fill=COLOR_HAT)
    draw.ellipse([cx-10, cy-32+bob_y, cx+10, cy-22+bob_y], fill=COLOR_HAT)

def generate_spritesheet():
    # Creare imagine cu fundal transparent (RGBA)
    img = Image.new('RGBA', (IMG_WIDTH, IMG_HEIGHT), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Iterare prin grila matematica a cadrelor
    for row in range(ROWS):
        for col in range(COLS):
            # Centrul local (cx, cy) al tile-ului curent
            cx = (col * TILE_SIZE) + (TILE_SIZE // 2)
            cy = (row * TILE_SIZE) + (TILE_SIZE // 2)
            
            draw_farmer(draw, cx, cy, direction=row, frame=col)
            
    # Salvare imagine
    img.save("player.png")
    print(f"S-a generat cu succes player.png la {IMG_WIDTH}x{IMG_HEIGHT}px!")

if __name__ == "__main__":
    generate_spritesheet()