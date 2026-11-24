# Máquina de lavar

Laboratório de máquina de estados para STM32F103, separado dos exercícios
FreeRTOS e do projeto de jogo.

- [Estados, funcionamento e implementação](WASH_MACHINE_FSM.md).
- Projeto STM32CubeIDE: importe esta pasta, que contém `.project` e `.cproject`.
- Configuração CubeMX: `washmachine.ioc`.
- Simulação Proteus: `WashMachine.pdsprj`.
- Código da aplicação: `Core/Src/main.c` e `Core/Src/wash_machine_fsm.c`.

O nome interno `washmachine` foi mantido. Após importar no novo caminho,
execute Clean/Build. No Proteus, abra as propriedades do microcontrolador e
troque o **Program File** antigo (`MIC2\washmachine\Release\washmachine.elf`)
por `Release\washmachine.elf`, relativo a esta pasta, e salve o projeto.
Esse ajuste interno ficou pendente porque o controle automatizado do Proteus
não conseguiu editar o diálogo; o arquivo da simulação foi preservado.
As pastas `Debug/` e `Release/` e os backups existentes foram preservados.
