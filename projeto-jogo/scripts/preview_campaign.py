"""Compose actual C renderer frames; optional documentation dependency: Pillow."""
import json
from pathlib import Path
from PIL import Image, ImageDraw

root = Path(__file__).resolve().parents[1]
source = root/'build/host'
scale, gap = 2, 16
cell_w, cell_h = 160*scale, 128*scale+22


def sheet(frames, filename):
    output = Image.new('RGB', (3*cell_w+4*gap, 3*cell_h+4*gap), '#101827')
    draw = ImageDraw.Draw(output)
    for index, (frame_name, title) in enumerate(frames):
        frame = Image.open(source/f'{frame_name}.ppm').resize((160*scale,128*scale), Image.Resampling.NEAREST)
        x, y = gap+(index%3)*(cell_w+gap), gap+(index//3)*(cell_h+gap)
        draw.text((x,y), title, fill='white')
        output.paste(frame, (x,y+22))
    target = root/'docs/assets'/filename
    output.save(target)
    print(target)


maps = []
for mid in range(1, 10):
    data = json.loads((root/f'assets/maps/map{mid:02}.json').read_text())
    maps.append((f'mapa{mid:02}', f"M{mid:02} - {data['name']}"))
sheet(maps, 'campanha-renderizada.png')
sheet([('inicio','MENU'),('ajuda','INSTRUCOES'),('preparar04','PREPARACAO'),
       ('mapa05','JOGO'),('pausa','PAUSA'),('resultado06','RESULTADO'),
       ('resultado09','ULTIMO MAPA'),('concluido','VITORIA'),('derrota','DERROTA')],
      'interface-renderizada.png')

sheet([(f'tutorial{i}', title) for i,title in enumerate(('CENTRALIZAR','MOVER ATE O ALVO','EXPERIMENTAR COLISAO','FEEDBACK SEM PENALIDADE','CONTORNAR E CHEGAR','PRATICAR PAUSA','RETOMAR','CONCLUIR'))], 'tutorial-renderizado.png')
