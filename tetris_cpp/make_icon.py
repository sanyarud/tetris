"""Generate a beautiful Tetris .icns icon for macOS."""
from PIL import Image, ImageDraw, ImageFilter, ImageFont
import os
import math

# Кольори фігур
COLORS = {
    'I': (0, 220, 220),
    'O': (220, 220, 0),
    'T': (160, 0, 220),
    'S': (0, 220, 0),
    'Z': (220, 0, 0),
    'J': (0, 0, 220),
    'L': (220, 150, 0),
}

HIGHLIGHT = {
    'I': (100, 255, 255),
    'O': (255, 255, 100),
    'T': (210, 100, 255),
    'S': (100, 255, 100),
    'Z': (255, 100, 100),
    'J': (100, 100, 255),
    'L': (255, 200, 80),
}

SHADOW = {
    'I': (0, 150, 150),
    'O': (150, 150, 0),
    'T': (100, 0, 150),
    'S': (0, 150, 0),
    'Z': (150, 0, 0),
    'J': (0, 0, 150),
    'L': (150, 100, 0),
}


def rounded_rect_mask(size, radius):
    """Create a rounded rectangle mask."""
    mask = Image.new('L', size, 0)
    draw = ImageDraw.Draw(mask)
    draw.rounded_rectangle([0, 0, size[0]-1, size[1]-1], radius=radius, fill=255)
    return mask


def draw_3d_block(draw, x, y, w, color, highlight, shadow, gap=2):
    """Draw a single 3D-style Tetris block."""
    x1, y1 = x + gap, y + gap
    x2, y2 = x + w - gap, y + w - gap
    bevel = max(2, w // 8)

    # Main face
    draw.rectangle([x1, y1, x2, y2], fill=color)

    # Top highlight
    draw.polygon([(x1, y1), (x2, y1), (x2 - bevel, y1 + bevel), (x1 + bevel, y1 + bevel)], fill=highlight)
    # Left highlight
    draw.polygon([(x1, y1), (x1, y2), (x1 + bevel, y2 - bevel), (x1 + bevel, y1 + bevel)], fill=highlight)

    # Bottom shadow
    draw.polygon([(x1, y2), (x2, y2), (x2 - bevel, y2 - bevel), (x1 + bevel, y2 - bevel)], fill=shadow)
    # Right shadow
    draw.polygon([(x2, y1), (x2, y2), (x2 - bevel, y2 - bevel), (x2 - bevel, y1 + bevel)], fill=shadow)

    # Small shine dot
    shine_size = max(2, w // 6)
    sx, sy = x1 + bevel + 2, y1 + bevel + 2
    draw.ellipse([sx, sy, sx + shine_size, sy + shine_size],
                 fill=(255, 255, 255, 120))


def make_icon(size):
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # macOS-style rounded background
    margin = int(size * 0.05)
    corner_r = int(size * 0.22)
    bg_rect = [margin, margin, size - margin, size - margin]

    # Gradient background (dark blue-purple)
    for y in range(margin, size - margin):
        t = (y - margin) / (size - 2 * margin)
        r = int(20 + t * 15)
        g = int(15 + t * 10)
        b = int(45 + t * 25)
        draw.line([(margin, y), (size - margin, y)], fill=(r, g, b, 255))

    # Apply rounded mask to background
    mask = rounded_rect_mask((size, size), corner_r)
    bg_only = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    bg_only.paste(img, mask=mask)
    img = bg_only
    draw = ImageDraw.Draw(img)

    # Subtle grid pattern on background
    cell = size / 10
    grid_margin = margin + int(size * 0.02)
    for gx in range(11):
        x = int(grid_margin + gx * (size - 2 * grid_margin) / 10)
        draw.line([(x, margin), (x, size - margin)], fill=(255, 255, 255, 15), width=1)
    for gy in range(11):
        y = int(grid_margin + gy * (size - 2 * grid_margin) / 10)
        draw.line([(margin, y), (size - margin, y)], fill=(255, 255, 255, 15), width=1)

    # Клітинка для фігур
    block_size = int(size * 0.1)
    base_x = int(size * 0.12)
    base_y = int(size * 0.08)
    gap = max(1, int(block_size * 0.08))

    # Фігури розміщені красиво на іконці
    pieces = {
        # T-подібна зверху по центру
        'T': [(2, 0), (1, 1), (2, 1), (3, 1)],
        # J-подібна зліва
        'J': [(5, 0), (5, 1), (4, 2), (5, 2)],
        # L-подібна справа
        'L': [(0, 2), (0, 3), (1, 3), (2, 3)],
        # S-подібна
        'S': [(3, 2), (4, 2), (2, 3), (3, 3)],
        # Z-подібна
        'Z': [(1, 4), (2, 4), (2, 5), (3, 5)],
        # I-подібна горизонтально внизу
        'I': [(1, 7), (2, 7), (3, 7), (4, 7)],
        # O-подібна
        'O': [(5, 4), (6, 4), (5, 5), (6, 5)],
    }

    for piece_name, cells in pieces.items():
        col = COLORS[piece_name]
        hl = HIGHLIGHT[piece_name]
        sh = SHADOW[piece_name]
        for cx, cy in cells:
            bx = base_x + cx * block_size
            by = base_y + cy * block_size
            draw_3d_block(draw, bx, by, block_size, col, hl, sh, gap)

    # Тонка лінія що падає (ефект руху)
    fall_x = base_x + int(2.5 * block_size)
    for i in range(4):
        alpha = 180 - i * 40
        y_start = base_y + int(1.3 * block_size) + i * int(block_size * 0.15)
        line_len = int(block_size * 0.3) - i * int(block_size * 0.05)
        draw.line([(fall_x, y_start), (fall_x, y_start + line_len)],
                  fill=(255, 255, 255, max(0, alpha)), width=max(1, size // 256))

    # Apply rounded mask again
    final = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    final.paste(img, mask=mask)

    return final


# Генеруємо іконку
icon_1024 = make_icon(1024)
icon_1024.save('/Users/sanyarud/Documents/Proxmox/tetris_cpp/icon_1024.png')
print("Icon 1024x1024 saved!")

# Створюємо iconset
iconset_path = '/Users/sanyarud/Documents/Proxmox/tetris_cpp/Tetris.iconset'
os.makedirs(iconset_path, exist_ok=True)

sizes = [16, 32, 64, 128, 256, 512]
for s in sizes:
    img = icon_1024.resize((s, s), Image.LANCZOS)
    img.save(f'{iconset_path}/icon_{s}x{s}.png')
    img2x = icon_1024.resize((s*2, s*2), Image.LANCZOS)
    img2x.save(f'{iconset_path}/icon_{s}x{s}@2x.png')

print("Iconset created!")
