# Escopo do novo projeto

## Requisitos do enunciado

- Projeto individual, com ideia submetida à aprovação do professor.
- Jogo eletrônico em microcontrolador ARM empregando RTOS.
- Display matricial e joystick.
- Sons durante o jogo.
- Tela inicial e tela final de game over.
- Reinício através do joystick.
- Pelo menos três níveis de dificuldade.

A avaliação considera funcionamento, originalidade, criatividade, jogabilidade,
imagens gráficas, montagem, qualidade e domínio de código/hardware e
desenvolvimento em sala. Fonte: [enunciado original](../referencias/Projeto_uC_II_2026_02.pdf).

## Decisões ainda abertas

Tema, objetivo do jogador, comandos, vitória/derrota, pontuação e progressão.
“Missão Circuito” foi sugerido, mas ainda não foi escolhido pelo usuário.
Também confirmar a placa física, controlador/variante do display, pinagem dos
módulos e circuito de som. Os componentes informados estão em [HARDWARE.md](HARDWARE.md).

## Decisão de hardware em 27/10/2026

O usuário definiu **somente tela, joystick e sons, sem motor** para MIC2.
Informou um display “1.8 TFT SPI 128*60 V1.1” e joystick “MH preto”. O buzzer
ainda não foi adquirido. A identificação da tela foi registrada literalmente;
não se assume resolução, controlador ou alimentação apenas por esse nome.
A ideia do elevador segue separadamente em MAQ.

## Sequência de desenvolvimento

1. Definir uma ideia simples, jogável e compatível com os requisitos; levar ao professor.
2. Validar individualmente display, joystick e som na placa.
3. Organizar a aplicação: estado do jogo, entrada, renderização e áudio, com
   tarefas de responsabilidades claras e comunicação por filas quando necessária.
4. Implementar uma primeira fase completa com início, interação, fim e reinício.
5. Acrescentar três níveis com diferenças observáveis nas regras/dificuldade.
6. Separar lógica independente de HAL e escrever testes das regras definidas.
7. Medir resposta, uso de memória e interação das tarefas; registrar ensaios.
8. Elaborar relatório final com decisões e evidências reais de implementação.

Nesta etapa não há testes de jogabilidade ou relatório final, porque as regras
ainda não foram escolhidas. O primeiro marco é a base do professor organizada
e com compilação reproduzível.
