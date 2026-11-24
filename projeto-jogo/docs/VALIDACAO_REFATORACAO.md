# Validação da refatoração didática — 24/11/2026

Referência anterior: commit `821b5e0`. Código refatorado na branch
`refactor/rtos-didatico`. Os testes abaixo foram executados localmente, sem
placa conectada. Não são resultados de uma execução nova do GitHub Actions.

## Testes de comportamento

Em Ubuntu/WSL, GCC com `-Wall -Wextra -Werror -pedantic`, AddressSanitizer e
UndefinedBehaviorSanitizer:

```sh
python3 scripts/test_host.py --sanitize
python3 scripts/check_config.py
python3 scripts/generate_map.py --check
```

- `test_game`: **8348 verificações aprovadas**. Incluem regras, nove mapas,
  tutorial, entrada, vidas, pontuação, colisão e temporização.
- `test_renderer`: estados, mapas/resultados, menus, pausa, avisos, limites
  de texto e igualdade entre desenho parcial e completo aprovados.
- Comparação SHA-256 das **45 imagens PPM** produzidas antes/depois:
  **nenhuma diferença**. A lista anterior foi capturada antes das alterações
  em `build/refatoracao/imagens-antes.json`, artefato local ignorado pelo Git.
- `test_driver`: driver ST7735 real com HAL/SPI simulados aprovado. O novo
  caso com `INT_MIN` detectou estouro no buffer antigo; passou após substituir
  dez bytes/`sprintf` por doze bytes/`snprintf`. `INT_MAX` também foi exercitado.
- `test_app`: sete processos independentes exercitam as funções reais de
  `app_game.c`: fluxo normal, ADC ausente, entrada envelhecida, erro de SPI,
  fila de cliques cheia, falha ao criar fila e falha ao criar tarefa.
- `check_config`: contrato de clock HSI 96 MHz, SPI1, ranks ADC/DMA,
  interrupções, configuração RTOS, bootstrap e hooks aprovado.
- `generate_map --check`: nove máscaras e percursos coerentes com os JSONs;
  dados gerados não precisaram ser alterados.

As filas dos testes copiam mensagens e o relógio é controlado. Esses testes
não implementam o scheduler FreeRTOS, preempção nem temporização elétrica;
confirmam a integração e os caminhos de falha cobertos.

## Compilação para a placa

GNU Arm GCC **14.3.1**, `-O0 -Wall`, Cortex-M4F, 46 objetos por configuração.
Compilação e ligação concluídas, sem avisos emitidos, gerando ELF, HEX e BIN.

```sh
python scripts/build_base.py --cc <caminho-do-arm-none-eabi-gcc>
python scripts/build_base.py --cc <caminho-do-arm-none-eabi-gcc> --diagnostic
```

- **Campanha:** text **76968**, data **120**, bss **24272** bytes.
  Saída: `build/campanha/labirinto-campanha.elf` e equivalentes `.hex`/`.bin`.
- **Diagnóstico:** text **32404**, data **104**, bss **21624** bytes.
  Saída: `build/diagnostico/diagnostico-spi1.elf` e equivalentes `.hex`/`.bin`.

Antes: campanha 76360/120/24272; diagnóstico 32292/104/21616.
A refatoração acrescentou 608 bytes de text à campanha; data e bss ficaram
iguais. No diagnóstico, text cresceu 112 bytes e bss, 8 bytes.
Tamanhos de pilha, prioridades e capacidade das filas foram preservados.
Esses números não medem uso máximo de pilha nem margem de heap em execução.

A redução de 48 para 46 objetos vem da retirada dos três fontes de exemplo
e da adição de `app_diagnostic.c`. `Legacy` não entra no build por script;
as configurações Debug/Release do `.cproject` também não incluem essa pasta.
Não foi executado um novo ensaio de regeneração pela interface do CubeMX;
o contrato de configuração foi verificado por script. O ensaio histórico
permanece documentado em [REGENERACAO.md](REGENERACAO.md).

## Comparação de origem

`compare_professor.py` compara o ZIP byte a byte e gera o inventário C:

- 138 arquivos preservados no mesmo caminho.
- 6 arquivos originais modificados.
- 6 arquivos movidos para Legacy com bytes idênticos.
- 16 arquivos acrescentados: 14 fontes/headers e 2 READMEs.
- Nenhum arquivo original removido sem correspondência.

Os 120 arquivos de HAL, CMSIS/dispositivo e FreeRTOS/adaptador permanecem
idênticos. O inventário reconhece 2678 definições sintáticas na base e 2754
na versão atual. Isso inclui alternativas condicionais, funções inline de
headers e código que não é ligado ao firmware. Em ambas as versões, 89
arquivos contêm construções não completamente resolvidas pelo parser;
o JSON registra essas limitações. Os seis módulos escritos da aplicação
(`app_game`, `app_diagnostic`, `joy_input`, `maze_game`, `maze_map` e
`game_renderer`) não apresentaram erros sintáticos no inventário.

## O que precisa da placa

Gravar e executar a revisão; testar tutorial, campanha, pausa/reinício,
centralização e polaridade do joystick; medir resposta e fluidez; consultar
`game_input_stack_free`, `game_logic_stack_free`, `game_render_stack_free`,
`game_min_free_heap` e `game_late_ticks`. O roteiro está em
[ENSAIO_FINAL.md](ENSAIO_FINAL.md). Essa etapa pode ser feita no outro PC;
não é necessária conexão de hardware para reproduzir os testes de host.
