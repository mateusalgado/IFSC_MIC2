# Projeto de Microcontroladores II — Jogo eletrônico

**Ponto de partida reiniciado em 27/10/2026.** Baseado em
`Projeto_uC_II_2026_02.pdf` e `Game_Start2_STM32F411CE.zip`, enviados pelo professor.
Não há tema, regras, motor adicional ou jogo completo definidos nesta etapa.

## Conteúdo

- `Game_Start2_STM32F411CE/`: fontes e projeto CubeIDE originais, extraídos do ZIP.
- `referencias/Projeto_uC_II_2026_02.pdf`: enunciado original.
- [Escopo](docs/ESCOPO.md): exigências, decisões pendentes e sequência de trabalho.
- [Base do professor](docs/BASE_PROFESSOR.md): recursos existentes e pontos de revisão.
- `scripts/build_base.py`: compilação da base original pela linha de comando.
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

As fontes extraídas foram preservadas. Saídas antigas `Debug/Release` do ZIP
foram excluídas da importação para não apresentar binários antigos como novos.

## Compilação pela linha de comando

Com Python 3 e GNU Arm Embedded no PATH:

```sh
python3 scripts/build_base.py
```

No Windows, se o compilador não estiver no PATH:

```powershell
python scripts/build_base.py --cc 'C:\caminho\tools\bin\arm-none-eabi-gcc.exe'
```

O resultado vai para `build/base-professor.elf`. O script não grava o MCU e não
altera as fontes do professor. A compilação confirma integração de código;
funcionamento físico e requisitos completos do jogo exigem ensaios futuros.
