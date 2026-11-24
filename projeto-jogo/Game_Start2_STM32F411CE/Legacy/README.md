# Exemplos originais do professor

Estes seis arquivos foram movidos de `Core/Inc` e `Core/Src`, preservando seus
bytes. O jogo atual não os utiliza:

- `Src/PRNG_LFSR.c` e `Inc/PRNG_LFSR.h`: gerador pseudoaleatório por LFSR.
  Os labirintos atuais são fixos, definidos nos JSONs e gerados em C.
- `Src/atraso.c` e `Inc/atraso.h`: atrasos por laços de instruções.
  A constante de 36 MHz desse exemplo não corresponde ao clock atual de 96 MHz.
  Nas tarefas, usamos `vTaskDelayUntil` para liberar o processador entre ciclos.
- `Inc/defPrincipais.h`: macros e definições auxiliares dos exemplos antigos.
- `Src/figuras.c`: dados das figuras da demonstração original.

O CubeIDE inclui como pastas de fontes somente `Core`, `Drivers` e
`Middlewares`, tanto em Debug como em Release. `scripts/build_base.py` também
não compila esta pasta. Não adicione `Legacy` às fontes do jogo.

`Core/Inc/figuras.h` permanece no Core porque declara `FigDef`, usado na
interface do ST7735. `fonts.c` e `fonts.h` continuam ativos para desenhar texto.

O [inventário comparado](../../docs/comparacao/INVENTARIO.md) confirma a
igualdade dos arquivos movidos com o ZIP original. Esta pasta é material de
consulta; não é outra versão do jogo nem um projeto compilável isoladamente.
