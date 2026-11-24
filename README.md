<img src="projeto-jogo/docs/assets/ifsc-logo.png" alt="IFSC - Câmpus Florianópolis" width="30%" style="padding: 10px">

# Microcontroladores II — IFSC

Materiais, exemplos e projeto da disciplina. O trabalho foi reiniciado em
27/10/2026 a partir do enunciado e do projeto STM32F411CE enviados pelo professor.

## Projetos

- [Laboratório FreeRTOS no STM32F411](laboratorios/freertos/README.md): 16 exemplos de tarefas,
  prioridades, comunicação, sincronização e interrupções, com USB CDC.
- [Jogo: Labirinto de Precisão](projeto-jogo/README.md), com nove mapas jogáveis,
  três por dificuldade, tutorial interativo e diagnóstico opcional.
- [Gravar e testar TUTORIAL v0.4](projeto-jogo/docs/CAMPANHA.md).
- [Relatório técnico](projeto-jogo/docs/RELATORIO.md).
- [Entrega e apresentação](projeto-jogo/docs/ENTREGA.md).
- [Configuração e regeneração CubeMX](projeto-jogo/docs/REGENERACAO.md).
- [Plano do jogo e arquitetura RTOS](projeto-jogo/docs/PLANO_LABIRINTO.md).
- [Interface, ícones e fontes](projeto-jogo/docs/PLANO_VISUAL.md).
- [Requisitos e próximos passos](projeto-jogo/docs/ESCOPO.md).
- [Revisão da base do professor](projeto-jogo/docs/BASE_PROFESSOR.md).
- [Enunciado original](projeto-jogo/referencias/Projeto_uC_II_2026_02.pdf).

## Usar o repositório

### Organização das pastas

- `projeto-jogo/`: projeto independente do Labirinto de Precisão, com firmware,
  mapas, documentação, testes, scripts e materiais originais do professor.
- `laboratorios/freertos/`: exemplos FreeRTOS, instruções e ZIP de referência.
- `laboratorios/maquina-lavar/`: firmware STM32F103 e simulação Proteus da máquina de lavar.
- `materiais/slides/`: apresentações da disciplina.
- `avaliacoes/prova-02/`: entrega da segunda avaliação.
- `arquivos/`: cópias preservadas, estrutura antiga e registro da organização;
  conteúdo local, ignorado pelo Git.

O Git agora fica na raiz de `MIC2`, preservando o histórico e o remoto existentes.
Os nomes internos `Game_Start2_STM32F411CE`, `ex_Barry_USB_STM32F411CE` e
`washmachine` foram mantidos por serem identificadores dos projetos STM32.
Detalhes da mudança: [organização e caminhos](docs/ORGANIZACAO.md).

Para trabalhar apenas no jogo, abra a pasta `projeto-jogo`. Ela contém tudo
que os scripts de compilação e teste precisam; o histórico Git e o workflow
do GitHub ficam na raiz da disciplina.

A documentação do projeto segue o padrão de PROGOO, com capa, sumário e
navegação entre quatro páginas:

[**1. Análise**](projeto-jogo/analise.md) ·
[**2. Projeto**](projeto-jogo/projeto.md) ·
[**3. Implementação**](projeto-jogo/implementacao.md) ·
[**4. Testes**](projeto-jogo/testes.md).

```sh
git clone https://github.com/mateusalgado/IFSC_MIC2.git
cd IFSC_MIC2/projeto-jogo
```

Importar `Game_Start2_STM32F411CE` no STM32CubeIDE, conforme o README do projeto.
Não compilar os exemplos do laboratório junto com o jogo: cada aplicação tem
seu próprio `main` e sua própria configuração RTOS.

## Projeto anterior

O mini elevador foi transferido para a pasta local `MAQ/elevador` e deixou de
ser a proposta de MIC2. Código, testes e documentação anteriores continuam
recuperáveis no [histórico](https://github.com/mateusalgado/IFSC_MIC2/tree/d0b7a7770b527711113d6e359cdf41d48d8bc83b/elevador).
O jogo novo não parte do firmware do elevador.
