"""Normalize the legacy stack-hook prototype emitted by CubeMX 6.18 for F4."""
import argparse
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--project', type=Path, default=root/'Game_Start2_STM32F411CE')
a = p.parse_args()
file = a.project/'Core/Src/freertos.c'
raw = file.read_bytes()
text = raw.decode('utf-8')
pattern = r'void vApplicationStackOverflowHook\(xTaskHandle xTask, signed char \*pcTaskName\);'
updated, count = re.subn(pattern, 'void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);', text)
assert count <= 1, 'Unexpected duplicate hook prototypes; inspect generation manually'
assert re.search(r'void\s+vApplicationStackOverflowHook\(TaskHandle_t\s+\w+,\s*char\s*\*\w+\)\s*\{',updated), 'Preserved hook implementation is missing'
if count:
    file.write_bytes(updated.encode('utf-8'))
print('CubeMX hook prototype normalized.' if count else 'CubeMX hook prototype already compatible.')
