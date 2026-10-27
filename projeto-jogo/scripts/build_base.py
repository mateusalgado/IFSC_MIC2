"""Build the unmodified teacher starter; Python 3 + GNU Arm Embedded required."""
import argparse
from pathlib import Path
import shutil
import subprocess

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--cc', default='arm-none-eabi-gcc')
args = parser.parse_args()
compiler = shutil.which(args.cc)
if compiler is None:
    parser.error('Compiler not found. Pass --cc with its executable path.')
base = root / 'Game_Start2_STM32F411CE'
kernel = base / 'Middlewares/Third_Party/FreeRTOS/Source'
output = root / 'build'
output.mkdir(exist_ok=True)
folders = [base/'Core/Src', base/'Drivers/STM32F4xx_HAL_Driver/Src',
           kernel, kernel/'CMSIS_RTOS_V2']
sources = [p for folder in folders for p in sorted(folder.glob('*.c'))]
sources += [kernel/'portable/GCC/ARM_CM4F/port.c', kernel/'portable/MemMang/heap_4.c']
sources += sorted((base/'Core/Startup').glob('*.s'))
include = [base/'Core/Inc', base/'Drivers/STM32F4xx_HAL_Driver/Inc',
           base/'Drivers/STM32F4xx_HAL_Driver/Inc/Legacy',
           base/'Drivers/CMSIS/Device/ST/STM32F4xx/Include', base/'Drivers/CMSIS/Include',
           kernel/'include', kernel/'CMSIS_RTOS_V2', kernel/'portable/GCC/ARM_CM4F']
target = ['-mcpu=cortex-m4', '-mthumb', '-mfpu=fpv4-sp-d16', '-mfloat-abi=hard']
options = target + ['-std=gnu11', '-O0', '-g3', '-Wall', '-DDEBUG', '-DUSE_HAL_DRIVER',
                    '-DSTM32F411xE', '-ffunction-sections', '-fdata-sections', '--specs=nano.specs']
options += [f'-I{path}' for path in include]
objects = []
for number, source in enumerate(sources):
    obj = output / f'{number:03d}_{source.stem}.o'
    subprocess.run([compiler, *options, '-c', str(source), '-o', str(obj)], check=True)
    objects.append(str(obj))
elf = output / 'base-professor.elf'
subprocess.run([compiler, *target, *objects, f'-T{base / "STM32F411CEUX_FLASH.ld"}',
                '--specs=nano.specs', '--specs=nosys.specs', '-Wl,--gc-sections',
                f'-Wl,-Map={output / "base-professor.map"}', '-Wl,--start-group', '-lc', '-lm',
                '-Wl,--end-group', '-o', str(elf)], check=True)
subprocess.run([compiler.replace('gcc', 'size'), str(elf)], check=True)
print(f'Linked {len(objects)} objects: {elf}')
print('Teacher starter only. No hardware execution or complete game validation performed.')
