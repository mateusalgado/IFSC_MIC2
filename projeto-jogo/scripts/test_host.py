"""Run portable game/renderer tests with GCC or Clang; no STM32 connected."""
import argparse
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--cc', default='cc')
parser.add_argument('--sanitize', action='store_true')
args = parser.parse_args()
base = root/'Game_Start2_STM32F411CE/Core'
output = root/'build/host'
output.mkdir(parents=True, exist_ok=True)
flags = ['-std=c11', '-g', '-O1', '-Wall', '-Wextra', '-Werror', '-pedantic']
if args.sanitize:
    flags += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer']
flags += ['-I'+str(root/'tests/stubs'), '-I'+str(base/'Inc')]
common = [base/'Src'/name for name in ['maze_game.c','maze_map.c','maze_mapdata.c','joy_input.c']]
for name, sources in [
    ('test_game', common+[root/'tests/test_game.c']),
    ('test_renderer', common+[root/'tests/test_renderer.c',base/'Src/game_renderer.c',base/'Src/fonts.c'])
]:
    executable = output/name
    subprocess.run([args.cc,*flags,*map(str,sources),'-o',str(executable)],check=True)
    subprocess.run([str(executable),str(output)],check=True)

driver_flags = [flag for flag in flags if flag != '-I'+str(root/'tests/stubs')]
driver_flags.insert(0, '-I'+str(root/'tests/driver_stubs'))
driver = output/'test_driver'
subprocess.run([args.cc,*driver_flags,str(root/'tests/test_driver.c'),
                str(base/'Src/st7735.c'),str(base/'Src/fonts.c'),'-o',str(driver)],check=True)
subprocess.run([str(driver)],check=True)

# Execute the application task bodies using deterministic queue/time stubs.
# This does not simulate the FreeRTOS scheduler or replace a board test.
rtos_flags = [flag for flag in flags if flag != '-I'+str(root/'tests/stubs')]
rtos_flags.insert(0, '-I'+str(root/'tests/rtos_stubs'))
app = output/'test_app'
subprocess.run([args.cc,*rtos_flags,*map(str,common),str(base/'Src/app_game.c'),
                str(root/'tests/test_app.c'),'-o',str(app)],check=True)
for scenario in ['flow','no-adc','stale-input','display-error','click-overflow',
                 'queue-allocation','task-allocation']:
    subprocess.run([str(app),scenario],check=True)
