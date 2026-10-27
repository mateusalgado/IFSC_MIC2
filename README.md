# Microcontroladores II — IFSC

Materiais, exemplos e projeto da disciplina. O trabalho foi reiniciado em
27/10/2026 a partir do enunciado e do projeto STM32F411CE enviados pelo professor.

## Projetos

- [Laboratório FreeRTOS no STM32F411](README-labRTOS.md): 16 exemplos de tarefas,
  prioridades, comunicação, sincronização e interrupções, com USB CDC.
- [Novo projeto: jogo eletrônico](projeto-jogo/README.md), ainda sem tema definido.
- [Requisitos e próximos passos](projeto-jogo/docs/ESCOPO.md).
- [Revisão da base do professor](projeto-jogo/docs/BASE_PROFESSOR.md).
- [Enunciado original](projeto-jogo/referencias/Projeto_uC_II_2026_02.pdf).

## Usar o repositório

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
