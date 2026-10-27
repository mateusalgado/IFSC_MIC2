# Projeto de Microcontroladores II — Jogo eletrônico

**Ponto de partida reiniciado em 27/10/2026.** Baseado em
`Projeto_uC_II_2026_02.pdf` e `Game_Start2_STM32F411CE.zip`, enviados pelo professor.
Hardware escolhido: tela, joystick e sons, sem motor. Tema e regras ainda serão
definidos; não há jogo completo nesta etapa.

## Conteúdo

- `Game_Start2_STM32F411CE/`: base do professor adaptada para diagnóstico de hardware.
- `referencias/Projeto_uC_II_2026_02.pdf`: enunciado original.
- [Escopo](docs/ESCOPO.md): exigências, decisões pendentes e sequência de trabalho.
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
