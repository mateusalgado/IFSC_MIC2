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
Também confirmar a placa física, controlador/variante do display e circuito de som. A pinagem para o ensaio
está definida; falta validá-la em bancada. Os componentes informados estão em [HARDWARE.md](HARDWARE.md).

## Decisão de hardware em 27/10/2026

O usuário definiu **somente tela, joystick e sons, sem motor** para MIC2.
A foto confirmou a inscrição “1.8 TFT SPI 128*160 V1.1” e os terminais do
joystick “MH preto”. O buzzer ainda não foi adquirido. O controlador ST7735
é a premissa do software, ainda a validar fisicamente.
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
ainda não foram escolhidas. O marco atual é o firmware de diagnóstico pronto para gravação; a validação
na placa continua pendente.

## Trabalho a executar e critérios de conclusão

- [x] Organizar materiais do professor e preservar a origem.
- [x] Definir a ligação SPI1: SDA A7, SCK A5, CS B12, RESET B13, DC B14;
  joystick VRY A0, VRX A1 e SW C15.
- [x] Preparar diagnóstico com uma tarefa responsável pela tela e gerar firmware.
- [ ] Executar o roteiro de bancada: cores, área útil, eixos, botão e estabilidade;
  registrar centro, extremos e orientação do joystick.
- [ ] Escolher e submeter ao professor a ideia do jogo. Proposta para discussão:
  **Missão Circuito** — mover um robô, coletar componentes e evitar faíscas.
  Ainda não é uma decisão aprovada.
- [ ] Fechar regras: objetivo, pontuação, vidas, vitória/derrota, reinício pelo
  botão e três dificuldades com diferenças mensuráveis (velocidade, quantidade
  de obstáculos e/ou tempo disponível).
- [ ] Selecionar buzzer e circuito; confirmar se PA8/TIM1_CH1 atende ao componente.
- [ ] Projetar tarefas de entrada, lógica, renderização e áudio, com filas de
  eventos/estado, prioridades justificadas e propriedade exclusiva do display.
- [ ] Implementar estados INICIO, JOGANDO e GAME_OVER, debounce, calibração e
  primeira partida completa; depois implementar a progressão dos três níveis.
- [ ] Testar regras fora da placa e medir tempos, memória, pilhas e resposta na
  montagem real; tratar falhas de ADC/display e reconciliar o `.ioc`.
- [ ] Concluir relatório com arquitetura, esquema, decisões, resultados medidos,
  fotos e limitações; anexar código e instruções de reprodução no GitHub.

Não registrar como realizado um teste físico ou aprovação do professor sem evidência.
