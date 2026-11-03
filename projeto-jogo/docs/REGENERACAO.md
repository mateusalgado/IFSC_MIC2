# Configuração CubeMX e regeneração

Atualizado em 03/11/2026 para **TUTORIAL v0.4**. O `.ioc` agora descreve a
configuração utilizada pelo firmware: HSI/PLL de 96 MHz, ADC1 com ranks Y/X,
SPI1 a 12 MHz, DMA e parâmetros de diagnóstico do FreeRTOS. PH0/PH1 deixaram
de ser reservados para HSE. A pinagem usada na placa não mudou.

## O que foi alinhado

- PLL com fonte HSI, M=16, N=192, P=2 e Q=4; APB1 dividido por 2.
- ADC1: canal 0/rank 1 e canal 1/rank 2, 480 ciclos e prescaler PCLK/4.
- DMA2 Stream0 circular, prioridade NVIC 5; base de tempo HAL em TIM4.
- FreeRTOS: tick de 1 kHz, 56 prioridades CMSIS, heap de 15360 bytes,
  verificação de stack overflow 2 e hook de falha de alocação habilitado.
- Bootstrap com pilha de 512 palavras, criação das tarefas em região USER CODE
  e verificação do estado do kernel em região preservada.
- Hooks movidos de `app_game.c` para os blocos USER CODE 4 e 5 de `freertos.c`,
  evitando duplicação após gerar código.
- `KeepUserCode=true` e `DeletePrevious=false`; versão F4 fixada em 1.28.3,
  sem escolha automática da versão mais recente do pacote.

`check_config.py` confere o contrato entre `.ioc`, fontes, ranks ADC, handlers,
bootstrap e hooks. É uma checagem específica deste projeto, não um substituto
para compilação ou para a validação física do clock.

## Procedimento na outra máquina

Para simplesmente compilar/gravar o jogo, **não é preciso regenerar**.
Os fontes completos continuam no repositório.

Quando for necessário alterar a configuração no CubeMX:

1. Atualizar o clone e criar uma cópia/branch para a alteração.
2. Ter instalado o pacote **STM32Cube FW_F4 V1.28.3**. O `.ioc` mantém o formato
   da base 6.17; a verificação local usou CubeMX **6.18**, com migração da cópia.
3. Abrir `Game_Start2_STM32F411CE.ioc`, manter alvo F411CEUx, toolchain
   STM32CubeIDE, inicialização em `main.c` (`CoupleFile=false`), USER CODE
   preservado e arquivos próprios mantidos. Gerar o **projeto completo** pelo
   botão Generate Code; não usar o comando isolado `generate code` da CLI.
4. A partir da pasta `projeto-jogo`, executar:

```sh
python3 scripts/fix_cubemx_hooks.py
python3 scripts/check_config.py
python3 scripts/generate_map.py --check
python3 scripts/test_host.py --sanitize
python3 scripts/build_base.py
python3 scripts/build_base.py --diagnostic
```

No Windows, usar `python` e fornecer `--cc` ao build se o compilador não estiver
no PATH. Os testes de host podem ser executados em Linux/WSL com GCC/Clang.

5. Revisar o diff antes de incorporar: pinos, PLL, DMA, callbacks, hooks,
   `StartDefaultTask`, início ADC/DMA, `App_Start`, handlers e tarefas. Não aceitar
   mudanças de pinagem/clock como parte automática de uma migração.
6. No CubeIDE, Refresh → Clean → Build. Gravar e retestar tela, joystick,
   tutorial, pausa e campanha. Registrar versão e resultado.

## Compatibilidade do gerador

No ensaio com CubeMX 6.18, o template F4 emitiu um protótipo de
`vApplicationStackOverflowHook` com `signed char *`, enquanto o kernel 10.3.1 e
nossa implementação usam `char *`. Isso causava erro de compilação na cópia
regenerada. `fix_cubemx_hooks.py` corrige **somente esse protótipo conhecido**,
sem mudar a implementação ou as bibliotecas. É idempotente; não altera um
arquivo que já esteja compatível. `check_config.py` rejeita a assinatura antiga.

Essa etapa é necessária para esta combinação de template/versão; o arquivo
principal versionado já está pronto para compilar. Não foi ocultado um erro
de geração editando manualmente a cópia sem registrar a correção.

## Ensaio reproduzível realizado

Ambiente: Windows, CubeMX 6.18, GNU Arm GCC 14.3.1, firmware ST F4 v1.28.3.
A referência do pacote veio do [repositório oficial ST](https://github.com/STMicroelectronics/STM32CubeF4/tree/v1.28.3),
commit `94cae6e83f00e276a11957e7833c01ac3d0bd7af`, com os submódulos CMSIS Device
F4 e HAL fixados pelo próprio pacote. O pacote usado no ensaio fica em build
local ignorado pelo Git; não há caminho absoluto desta máquina no `.ioc`
publicado.

Foi feita uma cópia controlada de `Game_Start2_STM32F411CE`. Somente nela foram
ajustados versão de migração e caminho local do pacote. A CLI documentada no
[manual UM1718](https://www.st.com/resource/en/user_manual/um1718-stm32cubemx-for-stm32-configuration-and-initialization-c-code-generation-stmicroelectronics.pdf),
páginas 49–51 da revisão local 50, recebeu:

```text
config load <copia>/Game_Start2_STM32F411CE.ioc
project path <copia>
project name Game_Start2_STM32F411CE
project toolchain STM32CubeIDE
project generate
exit
```

Após a geração completa, aplicar o normalizador e verificador com `--project
<copia>`. O build aceita `--project <copia> --output <pasta-de-saida-separada>`
para compilar essa árvore sem substituir os binários normais. Usar uma cópia
nova, sem misturar saídas de tentativas com estrutura Src/Inc diferente de Core.

Registro do ensaio final em 03/11/2026:

- Geração completa finalizada pela CLI com `OK` e saída normal. O serviço de
  atualização dos arquivos CubeIDE levou aproximadamente cinco minutos.
- Normalizador aplicado; contrato de configuração aprovado na cópia gerada.
- Oito módulos próprios/driver comparados e preservados; todas as regiões
  USER CODE de `main.c` preservadas.
- Jogo regenerado: 48 objetos, text 76328, data 120, bss 24272 bytes.
- Diagnóstico regenerado: 48 objetos, text 32276, data 104, bss 21616 bytes.
- Ambos compilados com GCC 14.3.1, `-O0 -Wall`, sem avisos; ELF/HEX/BIN gerados.

Os tamanhos diferem ligeiramente dos fontes salvos (76360/120/24272 e
32292/104/21616), pois o gerador reescreve inicialização e bibliotecas da cópia;
não se espera identidade binária entre essas árvores. O teste compara configuração,
preservação da aplicação e compilação. A geração é validada localmente;
o GitHub Actions verifica os fontes salvos, o contrato de configuração, as regras
e os dois builds. O CI não executa o aplicativo proprietário CubeMX.

A aplicação publicada preserva as bibliotecas originais e os fontes revisados;
a cópia regenerada é evidência de compatibilidade, não uma substituição indiscriminada
do projeto. O teste de geração/compilação não comprova execução física da cópia
regenerada. Essa confirmação deve ser registrada pelo aluno.
