"""Check the saved CubeMX contract against the C sources (no board required)."""
import argparse
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--project', type=Path, default=root/'Game_Start2_STM32F411CE')
a = p.parse_args()
base = a.project
settings = dict(line.split('=',1) for line in (base/'Game_Start2_STM32F411CE.ioc').read_text().splitlines() if '=' in line)
expected = {
    'RCC.PLLSourceVirtual':'RCC_PLLSOURCE_HSI', 'RCC.PLLM':'16', 'RCC.PLLN':'192',
    'RCC.PLLP':'RCC_PLLP_DIV2', 'RCC.PLLQ':'4', 'RCC.SYSCLKFreq_VALUE':'96000000',
    'RCC.APB1CLKDivider':'RCC_HCLK_DIV2', 'RCC.APB1Freq_Value':'48000000',
    'RCC.APB2Freq_Value':'96000000', 'SPI1.BaudRatePrescaler':'SPI_BAUDRATEPRESCALER_8',
    'SPI1.Direction':'SPI_DIRECTION_2LINES', 'PA5.Signal':'SPI1_SCK','PA7.Signal':'SPI1_MOSI',
    'PA0-WKUP.Signal':'ADCx_IN0','PA1.Signal':'ADCx_IN1', 'PC15-OSC32_OUT.GPIO_PuPd':'GPIO_PULLUP',
    'PB12.GPIO_Label':'CS','PB13.GPIO_Label':'RST','PB14.GPIO_Label':'DC',
    'ADC1.ClockPrescaler':'ADC_CLOCK_SYNC_PCLK_DIV4','ADC1.NbrOfConversion':'2',
    'ADC1.ContinuousConvMode':'ENABLE','ADC1.DMAContinuousRequests':'ENABLE',
    'Dma.ADC1.0.Mode':'DMA_CIRCULAR','Dma.ADC1.0.Instance':'DMA2_Stream0',
    'NVIC.TimeBaseIP':'TIM4','FREERTOS.configCHECK_FOR_STACK_OVERFLOW':'2',
    'FREERTOS.configUSE_MALLOC_FAILED_HOOK':'1','FREERTOS.configTOTAL_HEAP_SIZE':'15360',
    'FREERTOS.configMAX_PRIORITIES':'56','FREERTOS.configTICK_RATE_HZ':'1000',
    'ProjectManager.KeepUserCode':'true','ProjectManager.DeletePrevious':'false',
}
for k,v in expected.items():
    assert settings.get(k)==v, f'{k}: expected {v}, got {settings.get(k)}'
assert not any(k.startswith(('PH0','PH1')) for k in settings), 'HSE pins must remain released'
assert settings['NVIC.DMA2_Stream0_IRQn'].startswith('true\\:5\\:0')
assert settings['FREERTOS.Tasks01'].split(',')[:4]==['defaultTask','24','512','StartDefaultTask']
main=(base/'Core/Src/main.c').read_text(encoding='latin1')
compact=re.sub(r'\s+','',main)
for value in ['RCC_OscInitStruct.OscillatorType=RCC_OSCILLATORTYPE_HSI;',
              'RCC_OscInitStruct.PLL.PLLSource=RCC_PLLSOURCE_HSI;',
              'RCC_OscInitStruct.PLL.PLLM=16;','RCC_OscInitStruct.PLL.PLLN=192;',
              'RCC_OscInitStruct.PLL.PLLP=RCC_PLLP_DIV2;','RCC_OscInitStruct.PLL.PLLQ=4;',
              'hspi1.Init.BaudRatePrescaler=SPI_BAUDRATEPRESCALER_8;',
              'hspi1.Init.Direction=SPI_DIRECTION_2LINES;',
              'hadc1.Init.ClockPrescaler=ADC_CLOCK_SYNC_PCLK_DIV4;',
              'HAL_NVIC_SetPriority(DMA2_Stream0_IRQn,5,0);',
              'HAL_ADC_Start_DMA(&hadc1,(uint32_t*)ADC_buffer,64)',
              'osKernelGetState()!=osKernelReady','App_Start();','vTaskDelete(NULL);']:
    assert value in compact, f'C initialization or bootstrap changed: {value}'
channel=rank=None; pairs=[]
for m in re.finditer(r'sConfig\.(Channel|Rank)\s*=\s*(\w+)\s*;|HAL_ADC_ConfigChannel\(',main):
    if m.group(1)=='Channel':channel=m.group(2)
    elif m.group(1)=='Rank':rank=m.group(2)
    else:pairs.append((channel,rank))
assert sorted(pairs)==[('ADC_CHANNEL_0','1'),('ADC_CHANNEL_1','2')], pairs
config=(base/'Core/Inc/FreeRTOSConfig.h').read_text()
for key,val in [('configCHECK_FOR_STACK_OVERFLOW','2'),('configUSE_MALLOC_FAILED_HOOK','1'),
                ('configTOTAL_HEAP_SIZE','15360'),('configMAX_PRIORITIES','56'),
                ('configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY','5')]:
    line=re.search(r'^#define\s+'+key+r'\s+(.+)$',config,re.M)
    assert line and re.search(r'\b'+val+r'\b',line.group(1)), key
assert not re.search(r'vApplicationStackOverflowHook\([^;]*signed char[^;]*;', (base/'Core/Src/freertos.c').read_text()), 'Run scripts/fix_cubemx_hooks.py after CubeMX 6.18 regeneration'
sources='\n'.join(f.read_text(encoding='latin1') for f in (base/'Core/Src').glob('*.c'))
for hook in ['vApplicationMallocFailedHook','vApplicationStackOverflowHook']:
    assert len(re.findall(r'void\s+'+hook+r'\s*\([^)]*\)\s*\{',sources))==1, hook
for handler in ['SVC_Handler','PendSV_Handler','SysTick_Handler']:
    assert not re.search(r'void\s+'+handler+r'\s*\([^)]*\)\s*\{',sources), 'Kernel handlers must remain in FreeRTOS/CMSIS'
print('CubeMX/source contract passed: HSI 96 MHz, SPI1, ADC ranks/DMA, IRQs, RTOS and preserved bootstrap/hooks.')
