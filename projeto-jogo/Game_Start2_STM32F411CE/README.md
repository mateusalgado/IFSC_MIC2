# Firmware do Labirinto de Precisão

Importe esta pasta no STM32CubeIDE. Ela contém o código C do firmware,
as bibliotecas embarcadas, o startup, o linker script e a configuração CubeMX.
O jogo inicia com `APP_DIAGNOSTIC=0`; `1` seleciona o diagnóstico de bancada.

## Onde estudar e editar

1. `Core/Inc/maze_game.h`: estados, comandos e dados da partida.
2. `Core/Src/maze_game.c`: uma atualização da máquina de estados.
3. `Core/Src/joy_input.c`: calibração, filtro e botão.
4. `Core/Src/app_game.c`: três tarefas e cinco filas FreeRTOS.
5. `Core/Src/game_renderer.c`: desenho de cada estado na tela.
6. `Core/Src/main.c`: inicialização de periféricos e callbacks ADC/DMA.

`Core/Src/maze_map.c` consulta paredes/saída e verifica colisões.
`Core/Src/maze_mapdata.c` é gerado: edite os JSONs em `../assets/maps` e execute
`../scripts/generate_map.py`, em vez de alterar máscaras C manualmente.
`Core/Src/app_diagnostic.c` contém o teste de display, joystick e botão.

## O que é infraestrutura

- `Drivers`: CMSIS e HAL da ST, preservados da base do professor.
- `Middlewares`: FreeRTOS e adaptador CMSIS-RTOS v2, preservados da base.
- `Core/Startup`, arquivos `stm32f4xx_*`, `system_*`, `syscalls.c` e `sysmem.c`:
  suporte do microcontrolador e da biblioteca C.
- `Legacy`: exemplos antigos preservados, excluídos da compilação.
- `.ioc`, `.project`, `.cproject` e scripts `.ld`: configuração da ferramenta
  e da memória; mantenha-os ao copiar o projeto para outro computador.

O repositório externo a esta pasta contém os mapas editáveis, testes, scripts,
documentação e o ZIP do professor. Clone o repositório inteiro para ter esses
materiais também.

Leia a [comparação e guia didático](../docs/COMPARACAO_PROFESSOR.md), o
[inventário de funções](../docs/comparacao/INVENTARIO.md) e o
[procedimento para testar em outro PC](../docs/ENSAIO_FINAL.md#teste-em-outro-computador).
