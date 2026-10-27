# Projeto de Microcontroladores II — Labirinto de Precisão

**Ponto de partida reiniciado em 27/10/2026.** Baseado em
`Projeto_uC_II_2026_02.pdf` e `Game_Start2_STM32F411CE.zip`, enviados pelo professor.
Hardware escolhido: tela, joystick e sons, sem motor. Ideia definida pelo aluno:
conduzir um ponto por diferentes mapas de labirinto sem tocar nas paredes.
O diagnóstico foi gravado e relatado como funcionando pelo aluno. O firmware
atual continua sendo esse diagnóstico; a implementação do jogo está planejada.

## Plano para implementação

O conceito foi escolhido pelo aluno; a aprovação do professor ainda está pendente.
Regras e parâmetros abaixo constituem a proposta v1 para implementação e testes.

- [Plano completo do jogo](docs/PLANO_LABIRINTO.md): objetivo, vidas, pontuação,
  três dificuldades, mapas, FSM, tarefas, filas, testes e marcos de entrega.
- [Interface e assets](docs/PLANO_VISUAL.md): telas, ícones, fontes e animações.
- [Prévia das telas](docs/assets/proposta-visual.png): composição ilustrativa em
  160 × 128; não é uma captura do firmware rodando.

## Conteúdo

- `Game_Start2_STM32F411CE/`: base do professor adaptada para diagnóstico de hardware.
- `referencias/Projeto_uC_II_2026_02.pdf`: enunciado original.
- [Escopo](docs/ESCOPO.md): requisitos, decisões e situação das entregas.
- [Base do professor](docs/BASE_PROFESSOR.md): recursos existentes e pontos de revisão.
- [Hardware](docs/HARDWARE.md): peças informadas, ligações definidas e identificação pendente.
- `scripts/build_base.py`: compilação do diagnóstico pela linha de comando.
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
A versão atual inicia diretamente um diagnóstico de tela, eixos e botão sob FreeRTOS.
Não espera o botão para começar nem executa as três tarefas gráficas do exemplo.
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

O resultado vai para `build/base-professor.elf`, `.hex` e `.bin`; o nome do arquivo
foi mantido por compatibilidade, mas o conteúdo é o diagnóstico atual. O script
não grava o MCU. Os mesmos arquivos são disponibilizados pelo GitHub Actions
no artefato `mic2-diagnostico-spi1`. Funcionamento físico e requisitos completos
do jogo exigem ensaios futuros. Siga o [roteiro de teste](docs/TESTE_BANCADA.md).
