# Compilar e integrar no STM32CubeIDE

## Dependências e alvo

Núcleo: C11 e biblioteca C. Host: GCC/Make, sem HAL. ARM: GNU Arm Embedded,
Python 3.9+, HAL STM32F4/CMSIS/USB e FreeRTOS 10.3.1 presentes na base Barry.
O script não baixa dependências e não modifica nem grava o laboratório.
Use a revisão da base registrada em [VALIDACAO.md](VALIDACAO.md).

## Build ARM reproduzível

Da pasta `elevador`, com o compilador no PATH:

```sh
python3 scripts/build_f411.py
```

Se as ferramentas estiverem fora do PATH, passar os caminhos explicitamente:

```powershell
python scripts/build_f411.py --cc 'C:\caminho\tools\bin\arm-none-eabi-gcc.exe' --base '..\labRTOS\ex_Barry_USB_STM32F411CE'
```

Na estrutura clonada do GitHub, `--base` é `..\ex_Barry_USB_STM32F411CE`.
O script detecta as duas disposições se o argumento for omitido. Saída:
`build/f411/elevador-unconfigured.elf` e mapa de memória. Não é imagem autorizada
para movimentar a maquete: o port físico é deliberadamente inativo.

As fontes de laboratório usadas incluem startup F411, linker F411, timebase
TIM4, interrupções, sistema, syscalls, HAL, FreeRTOS e USB. Não são incluídos
`main.c`, `lab_examples.c`, `main1_1_4_ex1.c` nem `freertos.c` do laboratório.
As tarefas e o callback de memória idle desta entrega substituem os do exemplo.
Manter avisos de fontes externas visíveis; código novo é compilado com `-Werror`.

## Projeto no CubeIDE

1. Fazer uma cópia de trabalho do projeto Barry com outro nome, fora do original.
   Importar como projeto existente e confirmar STM32F411CEU6, Cortex-M4F,
   `STM32F411xE`, `USE_HAL_DRIVER`, hard-float e linker `STM32F411CEUX_FLASH.ld`.
2. Excluir do build o `main.c` original, `main1_1_4_ex1.c`, `lab_examples.c`
   e `freertos.c`. Garantir somente um `main`, um conjunto de hooks e uma
   definição da tarefa idle. Outros exemplos não devem iniciar tarefas.
3. Adicionar fontes de `src/` e `ports/stm32f411/`. Adicionar `include/` e
   `ports/stm32f411/` aos includes. **Usar uma única configuração FreeRTOS**:
   substituir a cópia do `Core/Inc/FreeRTOSConfig.h` na cópia do projeto pela
   configuração desta entrega; não deixar duas versões concorrendo.
4. Preservar startup, linker, USB, MSP, timebase e interrupções do F411. O `main`
   desta entrega inicializa HAL/clock, cria tarefas e chama `vTaskStartScheduler`.
   USB inicia dentro da tarefa `USBLog`, após o scheduler.
5. Compilar o port inativo. Verificar mapa de memória, ausência de símbolos
   duplicados e presença dos hooks. Não esperar LCD, ADC ou motor funcionais.
6. Quando hardware estiver definido, gerar/configurar ADC, GPIO, I²C e TIM1 na
   cópia do `.ioc`, revisar alterações e substituir `board_unconfigured.c`
   por implementação concreta de `board_port.h`.
7. A regeneração CubeMX não deve restaurar tarefas de laboratório, segundo
   `main` ou configurações de F103. Revisar o diff e repetir testes/build.

Não foi gerado `.ioc` com periféricos fictícios nem validada a importação pela
interface gráfica do CubeIDE. O build reproduzível prova compilação/ligação ARM
contra as dependências reais do laboratório, e não a configuração física futura.

## Contratos a implementar no port

- `board_safe_init`: logo após HAL_Init e antes da configuração de clock/RTOS,
  aplicar níveis de inibição seguros. Pull-down/up externos devem garantir
  inibição durante reset, boot, programação e MCU desconectado.
- `board_read_inputs`: leitura limitada e não bloqueante de GPIO + amostra ADC
  coerente, preferencialmente DMA. Definir validade e idade máxima da amostra;
  ADC antigo, DMA com erro ou amostra incompleta devem produzir `valid=false`.
- `board_motor_write`: única tarefa proprietária. Comutar direção somente com
  PWM/enable inativos e cumprir a tabela-verdade da ponte. Diferenciar parada
  normal, reversão e falha. Medir os sinais com analisador antes de ligar o motor.
- `board_lcd_write`: executar apenas na tarefa Display. Timeout I²C limitado,
  preferir envio incremental/IT/DMA. Falha no display não pode segurar o controle.
- `board_indicators`: LED conforme estado e buzzer de falha, sem atrasos.
- `board_fault_disable`: corte imediato para assert/panic. Acrescentar chamada
  nos handlers HardFault, MemManage, BusFault e UsageFault da cópia do projeto.
  Os handlers originais do laboratório apenas ficam em laço; isso permanece
  pendente até o port físico.
- `board_watchdog_refresh`: configurar IWDG independente e alimentá-lo somente
  depois de um ciclo de controle válido. Avaliar falta de execução, não apenas
  repetição de tick. Não alimentar dentro do idle hook ou da ISR de tick.

Configuração local habilita detecção de stack overflow e malloc failed, ambas
indo a panic. Pilhas e heap ainda exigem medição real. Configurar prioridades
NVIC: APIs RTOS `FromISR` só em prioridades permitidas por
`configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY=5`. Número NVIC menor significa
maior prioridade; não confundir com prioridades de tarefa FreeRTOS.

## USB CDC

A implementação usa `CDC_Transmit_FS`, `hUsbDeviceFS` e `TxState` da base.
Não usar `_write`/`printf` do laboratório como outro produtor CDC. Somente
USBLog transmite. Registros contêm tempo, estado, falha, último sensor, destino,
PWM percentual, aviso e contadores de descarte. Mensagens perdidas não são
reenviadas após reconectar; registrar o ensaio desde antes do início da viagem.
Reconexão, ocupação e prioridade real da IRQ ainda precisam de teste físico.
