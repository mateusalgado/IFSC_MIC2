# Organização de MIC2

Revisão local de 24/11/2026. A pasta de trabalho é a raiz do repositório Git.

## Onde encontrar cada trabalho

- Jogo: [projeto-jogo](../projeto-jogo/README.md).
- Exercícios de RTOS: [laboratorios/freertos](../laboratorios/freertos/README.md).
- Máquina de lavar: [laboratorios/maquina-lavar](../laboratorios/maquina-lavar/README.md).
- Slides: [materiais/slides](../materiais/slides).
- Prova 2: [avaliacoes/prova-02](../avaliacoes/prova-02).

## Caminhos alterados

- `labRTOS/projeto-jogo/` passou para `projeto-jogo/`.
- `labRTOS/ex_Barry_USB_STM32F411CE/` passou para
  `laboratorios/freertos/ex_Barry_USB_STM32F411CE/`.
- `labRTOS/README-labRTOS.md` passou para `laboratorios/freertos/README.md`.
- `labRTOS/exemplos_FreeRTOS.zip` passou para `laboratorios/freertos/referencias/`.
- `washmachine/` passou para `laboratorios/maquina-lavar/`.
- `SLIDES/` passou para `materiais/slides/`.
- `PROVA2/` passou para `avaliacoes/prova-02/`.
- Os PDFs de aula passaram a `01-tecnicas-de-programacao.pdf`,
  `02-arm-cortex-m3-m4.pdf` e `04-rtos.pdf`; a avaliação passou a
  `prova-02-entrega.pdf`. A numeração original das aulas foi preservada.
- O ZIP original do jogo e a imagem de consulta do enunciado ficam em
  `projeto-jogo/referencias/`. O PDF canônico já estava nessa pasta.
- O PDF duplicado da raiz foi preservado em `arquivos/duplicados/`.
- A estrutura vazia do antigo elevador foi preservada em
  `arquivos/elevador-estrutura-antiga/`.
- `.git`, `.github`, `.gitignore` e o README geral passaram de `labRTOS/`
  para a raiz de `MIC2`. Branches, histórico e remoto foram preservados.

## Abrir os projetos após a mudança

No STM32CubeIDE, importe como projeto existente a pasta que contém `.project`:

- Jogo: `projeto-jogo/Game_Start2_STM32F411CE/`.
- FreeRTOS: `laboratorios/freertos/ex_Barry_USB_STM32F411CE/`.
- Máquina de lavar: `laboratorios/maquina-lavar/`.

Se a IDE ainda apontar para o caminho antigo, remova somente a entrada antiga
do workspace, sem marcar a opção de apagar arquivos, e importe o novo caminho.
Execute Clean/Build para atualizar arquivos gerados que guardem caminhos antigos.
No Proteus, abra `laboratorios/maquina-lavar/WashMachine.pdsprj`. Foi identificada
uma referência antiga a `MIC2\washmachine\Release\washmachine.elf` no componente.
Abra as propriedades do microcontrolador e, em **Program File**, selecione
`Release\washmachine.elf`, relativo à pasta do projeto, e salve.
A automação do Proteus falhou ao acessar os controles do diálogo, por isso
essa referência interna permanece pendente de ajuste. O projeto foi preservado
sem edição binária, com uma cópia adicional antes dos ajustes.

Os nomes internos dos projetos, `.ioc`, bibliotecas, fontes e arquivos do CubeIDE
foram preservados. `Debug/`, `Release/` e `projeto-jogo/build/` continuam locais,
ignorados pelo Git; não foram eliminados binários ou ensaios anteriores.

## Preservação

Antes das edições documentais, os hashes SHA-256 de 485 arquivos foram comparados
com seus destinos: todos permaneceram idênticos. Esse inventário exclui `.git`
e saídas `build/`, `Debug/`, `Release/`, movidas com suas pastas.

`arquivos/organizacao-2026-11-24/` contém o inventário, o mapa dos caminhos e
cópias dos documentos antes dos ajustes. Essa pasta é local e não entra no Git.
O histórico da reorganização pode ser consultado com `git log` na raiz do
repositório. O conteúdo de `arquivos/` permanece apenas neste computador.

## Verificações após a mudança

O contrato CubeMX/fontes, os nove mapas e os testes de host com sanitizadores
passaram no novo caminho. Campanha e diagnóstico foram compilados para ARM,
gerando ELF, HEX e BIN. Os links locais da documentação também foram conferidos.
Resultados e pendências do jogo: [entrega](../projeto-jogo/docs/ENTREGA.md).
