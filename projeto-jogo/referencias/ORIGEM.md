# Origem dos materiais

Fornecidos pelo usuário no workspace MIC2 em 27/10/2026:

- `Game_Start2_STM32F411CE.zip`
  SHA-256: `0130a2994b2002d8e184d69a591b900be8716b7981480139348a8c594157b3cb`.
- `Projeto_uC_II_2026_02.pdf`
  SHA-256: `897e9cbee2145e0020de77f9e9681d1fb63def59afa298387af9893afa1a39cd`.

Foram extraídos 154 arquivos de projeto, fontes, bibliotecas e configuração.
Foram omitidos diretórios Debug/Release e metadados de sistema, preservando as
fontes originais e avisos de licença. O ZIP original permanece na raiz local de MIC2.
Não se atribui autoria das bibliotecas ou exemplos do professor ao aluno.

A documentação e o script de compilação nesta pasta são novos. O conteúdo
do elevador anterior não integra a base deste jogo.

## Adaptação para bancada

A importação sem alterações está no commit
[`02aed85`](https://github.com/mateusalgado/IFSC_MIC2/tree/02aed85f6b7a55d8cf7fbac0616d6681c7d9a5e9/projeto-jogo/Game_Start2_STM32F411CE).
A versão atual substitui a demonstração de soldados por um diagnóstico em uma
única tarefa FreeRTOS, com leitura ADC/DMA e botão. A pinagem SPI1 foi mantida;
o driver usa os nomes de CS/RST/DC definidos em `main.h`. O `.ioc` acompanha o
aumento da pilha da tarefa para 2048 bytes. Não foi feita regeneração pelo CubeMX.

## Implementação M01 v0.1

A partir da base e do diagnóstico, foram adicionados módulos próprios de mapa,
colisão, regras, entrada, renderização e integração RTOS, além de testes e
instruções. As fontes de terceiros permanecem atribuídas aos autores; o driver
ST7735 foi adaptado com limites, timeout e buffer fixo. A pinagem testada foi
mantida. O diagnóstico original é recuperável no commit `4382347` e há opção
de diagnóstico na versão atual. Não se atribui autoria do kernel/HAL ao aluno.
