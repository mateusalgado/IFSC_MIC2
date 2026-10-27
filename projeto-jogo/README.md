# Projeto de Microcontroladores II — Labirinto de Precisão

**Ponto de partida reiniciado em 27/10/2026.** Baseado em
`Projeto_uC_II_2026_02.pdf` e `Game_Start2_STM32F411CE.zip`, enviados pelo professor.
Hardware escolhido: tela, joystick e sons, sem motor. Ideia definida pelo aluno:
conduzir um ponto por diferentes mapas de labirinto sem tocar nas paredes.
O diagnóstico foi gravado e relatado como funcionando pelo aluno. A versão atual
é **M01 v0.1: primeiro mapa jogável**, com três vidas, tempo, pausa, colisão,
pontuação e reinício. Esta nova versão aguarda teste na placa pelo aluno.

## Testar agora

Siga [PRIMEIRO_MAPA.md](docs/PRIMEIRO_MAPA.md) para atualizar o clone, compilar,
gravar e testar. O build padrão executa o jogo; o diagnóstico continua disponível
com `APP_DIAGNOSTIC=1` ou `--diagnostic` no script. A tela inicial identifica
**LABIRINTO / M01 v0.1**. Ainda não há áudio nem a campanha de nove mapas.

## Plano para implementação

O conceito foi escolhido pelo aluno; a aprovação do professor ainda está pendente.
Regras e parâmetros abaixo constituem a proposta v1 para implementação e testes.

- [Plano completo do jogo](docs/PLANO_LABIRINTO.md): objetivo, vidas, pontuação,
  três dificuldades, mapas, FSM, tarefas, filas, testes e marcos de entrega.
- [Interface e assets](docs/PLANO_VISUAL.md): telas, ícones, fontes e animações.
- [Prévia das telas](docs/assets/proposta-visual.png): composição ilustrativa em
  160 × 128; não é uma captura do firmware rodando.

## Conteúdo

- `Game_Start2_STM32F411CE/`: base do professor adaptada para o primeiro mapa e diagnóstico opcional.
- `referencias/Projeto_uC_II_2026_02.pdf`: enunciado original.
- [Escopo](docs/ESCOPO.md): requisitos, decisões e situação das entregas.
- [Base do professor](docs/BASE_PROFESSOR.md): recursos existentes e pontos de revisão.
- [Hardware](docs/HARDWARE.md): peças informadas, ligações definidas e identificação pendente.
- `scripts/build_base.py`: compilação do jogo ou diagnóstico pela linha de comando.
- [Teste de bancada](docs/TESTE_BANCADA.md): gravação, resultado esperado e registro de ensaios.
- [Origem](referencias/ORIGEM.md): identificação dos materiais e integridade das fontes.

Não foram importados código, testes, pinagem ou regras do elevador. O exemplo
do professor permanece como referência inicial; seus comportamentos de
demonstração não representam um jogo já concluído.

## STM32CubeIDE

1. Importar como projeto existente a pasta `Game_Start2_STM32F411CE`.
2. Confirmar alvo STM32F411CEU6 e compilar a configuração Debug.
3. Revisar a divergência HSE/HSI descrita em `docs/BASE_PROFESSOR.md` antes de
   regenerar código a partir do `.ioc`.
4. Confirmar placa, display, joystick e ligações antes de gravar/testar.

As fontes originais estão preservadas no histórico indicado em `referencias/ORIGEM.md`.
A versão atual calibra o joystick e abre a tela inicial do labirinto. Entrada,
lógica e desenho têm tarefas separadas; só uma tarefa acessa o ST7735.
As saídas antigas `Debug/Release` do ZIP foram excluídas da importação.

## Compilação pela linha de comando

Com Python 3 e GNU Arm Embedded no PATH:

```sh
python3 scripts/build_base.py
```

No Windows, se o compilador não estiver no PATH:

```powershell
python scripts/build_base.py --cc 'C:\caminho\tools\bin\arm-none-eabi-gcc.exe'
```

O resultado padrão vai para `build/mapa01/labirinto-mapa01.elf`, `.hex` e `.bin`.
Use `--diagnostic` para gerar `build/diagnostico/diagnostico-spi1.*`. O script
não grava o MCU. O GitHub Actions disponibiliza os artefatos
`mic2-labirinto-mapa01` e `mic2-diagnostico-spi1`. Binários antigos
`base-professor.*` não são a versão atual.

## Testes no computador

Com GCC/Clang e Python 3 (Linux/WSL):

```sh
python3 scripts/generate_map.py --check
python3 scripts/test_host.py --sanitize
```

Os testes cobrem regras, percurso completo, entrada, desenho e o driver ST7735
com periférico simulado. Não substituem o teste físico da nova versão.
