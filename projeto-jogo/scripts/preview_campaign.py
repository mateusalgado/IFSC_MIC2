"""Compose the actual host renderer frames; optional documentation dependency: Pillow."""
from pathlib import Path
from PIL import Image, ImageDraw

root = Path(__file__).resolve().parents[1]
source = root/'build/host'
scale, gap = 2, 16
cell_w, cell_h = 160*scale, 128*scale+22
sheet = Image.new('RGB', (3*cell_w+4*gap, 3*cell_h+4*gap), '#101827')
draw = ImageDraw.Draw(sheet)
for col, (map_id, title) in enumerate(((1, 'FASE 1 - FACIL'), (4, 'FASE 2 - MEDIO'), (7, 'FASE 3 - DIFICIL'))):
    for row, prefix in enumerate(('preparar', 'mapa', 'resultado')):
        frame = Image.open(source/f'{prefix}{map_id:02}.ppm').resize((160*scale,128*scale), Image.Resampling.NEAREST)
        x, y = gap+col*(cell_w+gap), gap+row*(cell_h+gap)
        draw.text((x,y), title, fill='white')
        sheet.paste(frame, (x,y+22))
target = root/'docs/assets/campanha-renderizada.png'
sheet.save(target)
print(target)
