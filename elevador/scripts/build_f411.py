"""Reproducible non-actuating F411 build, reusing local lab HAL/FreeRTOS/USB.
No downloads, no writes to the lab, no board flashing. Python 3.9+.
"""
import argparse
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--base', type=Path, help='ex_Barry_USB_STM32F411CE directory')
parser.add_argument('--cc', default='arm-none-eabi-gcc')
args = parser.parse_args()
base = args.base or next((p for p in [ROOT.parent / 'ex_Barry_USB_STM32F411CE', ROOT.parent / 'labRTOS/ex_Barry_USB_STM32F411CE'] if p.is_dir()), None)
if base is None:
    parser.error('Provide --base with the existing STM32F411 laboratory directory')
base = base.resolve()
cc = shutil.which(args.cc) or args.cc
out = ROOT / 'build/f411'
out.mkdir(parents=True, exist_ok=True)
kernel = base / 'Middlewares/Third_Party/FreeRTOS/Source'
usb = base / 'Middlewares/ST/STM32_USB_Device_Library'
includes = [ROOT/'ports/stm32f411', ROOT/'include', base/'Core/Inc',
    base/'Drivers/STM32F4xx_HAL_Driver/Inc', base/'Drivers/CMSIS/Include',
    base/'Drivers/CMSIS/Device/ST/STM32F4xx/Include', kernel/'include',
    kernel/'portable/GCC/ARM_CM4F', kernel/'CMSIS_RTOS',
    usb/'Core/Inc', usb/'Class/CDC/Inc', base/'USB_DEVICE/App', base/'USB_DEVICE/Target']
sources = list((ROOT/'src').glob('*.c')) + list((ROOT/'ports/stm32f411').glob('*.c'))
sources += [base/'Core/Src'/name for name in ['stm32f4xx_hal_msp.c', 'stm32f4xx_hal_timebase_tim.c',
    'stm32f4xx_it.c', 'syscalls.c', 'sysmem.c', 'system_stm32f4xx.c']]
sources += list((base/'Core/Startup').glob('*.s'))
sources += list((base/'Drivers/STM32F4xx_HAL_Driver/Src').glob('*.c'))
sources += list(kernel.glob('*.c')) + [kernel/'portable/GCC/ARM_CM4F/port.c', kernel/'portable/MemMang/heap_4.c']
sources += list((kernel/'CMSIS_RTOS').glob('*.c'))
for folder in [usb/'Core/Src', usb/'Class/CDC/Src', base/'USB_DEVICE/App', base/'USB_DEVICE/Target']:
    sources += list(folder.glob('*.c'))
machine = ['-mcpu=cortex-m4', '-mthumb', '-mfpu=fpv4-sp-d16', '-mfloat-abi=hard']
common = machine + ['-std=gnu11', '-Os', '-g3', '-ffunction-sections', '-fdata-sections',
    '-Wall', '-Wextra', '-DUSE_HAL_DRIVER', '-DSTM32F411xE', '-DHSE_VALUE=25000000']
common += [f'-I{p}' for p in includes]
objects = []
for index, src in enumerate(sources):
    obj = out / f'{index:03d}_{src.stem}.o'
    flags = ['-Werror'] if ROOT in src.parents else []
    subprocess.run([cc, *common, *flags, '-c', str(src), '-o', str(obj)], check=True)
    objects.append(str(obj))
linker = base/'STM32F411CEUX_FLASH.ld'
elf = out/'elevador-unconfigured.elf'
subprocess.run([cc, *machine, *objects, f'-T{linker}', '--specs=nano.specs', '--specs=nosys.specs',
    '-Wl,--gc-sections', f'-Wl,-Map={out / "elevador.map"}', '-Wl,--start-group', '-lc', '-lm',
    '-Wl,--end-group', '-o', str(elf)], check=True)
size = str(cc).replace('gcc', 'size')
subprocess.run([size, str(elf)], check=True)
print(f'Linked {len(objects)} objects: {elf}')
print('UNCONFIGURED BOARD: motor, ADC, GPIO, LCD and watchdog drivers still required.')
