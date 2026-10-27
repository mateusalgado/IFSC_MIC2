# Relatório técnico — Mini elevador didático

**Disciplina:** Microcontroladores II, com integração de Máquinas e Acionamentos.

**Instituição de referência dos materiais:** Instituto Federal de Santa Catarina.

**Entrega:** base de software anterior à definição estrutural e montagem.

**Data:** 27 de outubro de 2026.

**Alvo adotado:** STM32F411CEU6 + FreeRTOS; placa física a confirmar.

## 1. Objetivo

Desenvolver a base de firmware de uma maquete de elevador com térreo, primeiro
e segundo andar. A proposta aplica os conteúdos da disciplina em um sistema com
entradas analógicas e digitais, sequência operacional, concorrência, comunicação
e acionamento de motor. A lógica deve ser verificável antes da impressão da
estrutura e da montagem eletrônica, sem impor componentes ainda desconhecidos.

O escopo desta entrega inclui controle de um destino por vez, referência inicial,
seleção por joystick, interface LCD, eventos USB, rampa de comando, intertravamento
de reversão e tratamento de falhas. A primeira versão trabalha em baixa velocidade
e para por sensor. Não mede posição contínua, corrente ou velocidade.

## 2. Fundamentação nos materiais do projeto

Foram consultados os seguintes materiais locais:

- **SLIDES/uCII_01_Tecnicas_Programacao.pdf**, páginas 24 e 29: sequência de
  desenvolvimento por FSM, correspondência entre diagrama e código, separação
  do hardware, handlers por funções e testes também de comportamentos proibidos.
  Aplicação: tabela de funções de estado, guardas comuns, port separado e testes
  que verificam ausência de movimento após reset/falha.
- **SLIDES/uCII_04_RTOS.pdf**, páginas 24–29, 31–39 e 44–54: tick,
  escalonamento, estados de tarefa, comunicação, semáforos, mutex e inversão de
  prioridade. Aplicação: tarefas preemptivas com prioridades diferentes, filas
  por cópia, bloqueio das tarefas sem trabalho e propriedade única de periféricos.
- **SLIDES/uCII_02_ARM_CortexM3_M4.pdf**, páginas 51, 54 e 61–63: suporte a
  sistemas operacionais, DMA e NVIC. Aplicação: preservar integração de SysTick,
  SVC e PendSV, manter IRQ curtas e prever aquisição ADC não bloqueante no port.
- **labRTOS/README-labRTOS.md** e **Core/Src/lab_examples.c**: exemplos 5/6
  fundamentam periodicidade com `vTaskDelayUntil`; 10/11, troca de mensagens;
  12–14, trabalho diferido de interrupções; 15/16, centralização de saída.
  A aplicação utiliza os mecanismos pertinentes, sem reproduzir esperas ocupadas
  ou alternância de prioridades usada apenas para demonstração em outros exemplos.
- **washmachine/WASH_MACHINE_FSM.md** e **washmachine/Core**: referência de
  organização por estados e tratamento de erro. Sua configuração é STM32F103 e
  não foi transplantada para o F411.
- **MAQ/PWM/DriverDeMotor.pdsprj**: inspeção do contêiner e identificação textual
  de modelo de motor CC. A topologia elétrica não foi validada no Proteus.

Os PDFs permanecem nos diretórios originais. Esta documentação registra os
conceitos utilizados sem republicar os materiais completos. Para apresentação,
foi observado o padrão de README por exemplo, descrição de funcionamento e
roteiro de simulação do [MIC1](https://github.com/mateusalgado/IFSC_MICROCONTROLADORESI),
especialmente `apps/oled/README.md` e `sim/simular.md`.

## 3. Método de desenvolvimento

Primeiro foram separados os requisitos independentes do hardware e as decisões
que exigem motor, ponte H e mecânica. Em seguida, foram implementados módulos C
sem HAL, com relógio e sensores fornecidos como entradas. Isso permite executar
a mesma lógica em testes no computador e na tarefa FreeRTOS.

O projeto original foi preservado. A integração do novo firmware utiliza as
bibliotecas locais do F411 e um port explicitamente não configurado. Um script
compila e liga o firmware ARM completo, sem programar a placa. A documentação
mantém todas as pendências físicas identificadas, em vez de codificar uma
pinagem ou características elétricas como se estivessem confirmadas.

## 4. Implementação

O núcleo contém sete estados: inicialização, referência, parado, subida, descida,
chegada e falha. Parada, limites, validade das entradas, incoerência de sensores
e prazo de execução são verificados antes do tratamento de comandos. A recuperação
exige reconhecimento explícito e uma nova solicitação de referência; não restaura
o destino anterior. A seleção admite apenas um destino e informa recusa de novos
pedidos durante movimento.

O joystick passa por filtragem e zona morta, com eventos discretos e rearme no
centro. Sensores e botão têm debounce; parada e limites são considerados sem
atraso de confirmação em software. O display preserva a distinção entre último
andar confirmado e posição atual desconhecida entre sensores.

O acionamento tem teto de duty, rampa de partida e intervalo neutro após zerar
antes de inverter. Parada de chegada e parada por falha são separadas da rampa
de reversão. O comportamento elétrico de frenagem é responsabilidade do port,
conforme a ponte e a transmissão escolhidas.

A aplicação FreeRTOS contém três tarefas. `Control` concentra o estado e as
saídas do motor; `Display` recebe o último retrato do sistema; `USBLog` recebe
eventos e registros periódicos por fila. Não se usa mutex no motor porque não
há múltiplos proprietários. Isso aplica exclusão por arquitetura e reduz caminhos
de inversão de prioridade, sem acrescentar mecanismos sem necessidade.

As tarefas e filas da aplicação são estáticas; verificações de alocação, hooks
de falha e detecção de stack overflow foram configuradas. O watchdog e a resposta
física de corte ainda dependem da placa e estão descritos como requisitos do port.

## 5. Integração com Máquinas e Acionamentos

PWM representa comando de tensão média no contexto do acionamento, sem ser uma
medida de velocidade. O efeito efetivo depende da ponte, alimentação, carga e
características do motor. Rampa e teto de duty não equivalem a controle fechado
de corrente, proteção de bloqueio ou garantia de aceleração mecânica.

O dimensionamento futuro deve relacionar massa da cabine, carga, gravidade,
atrito, transmissão, redução e rendimento com torque/corrente do motor. A seleção
da ponte/fonte deve considerar partida e bloqueio, dissipação e energia de
frenagem conforme a topologia. Não foram aplicadas fórmulas de motores trifásicos
ao motor CC.

O plano experimental compara subida/descida e cargas distintas, registrando
tempo, duty, corrente medida externamente, temperatura e ultrapassagem no sensor.
Encoder e sensor de corrente são uma segunda etapa. PI de velocidade somente
após definir e calibrar a medição.

## 6. Resultados desta entrega

Os 24 grupos automatizados passaram no computador, incluindo integração das
entradas brutas até a FSM. A execução com AddressSanitizer e UndefinedBehaviorSanitizer
também passou. O firmware foi compilado e ligado para Cortex-M4F/STM32F411 com as
dependências reais do laboratório. Os comandos, ambiente e limites estão em
[VALIDACAO.md](VALIDACAO.md).

Os resultados demonstram comportamento da lógica nos cenários simulados e
compatibilidade de compilação da integração. **Não demonstram** temporização
real do scheduler, qualidade da alimentação, resposta elétrica da ponte, retenção,
precisão de parada, comunicação física I²C/USB ou segurança da mecânica.
Nenhum ensaio físico ou simulação de circuito Proteus foi realizado.

## 7. Limitações e continuidade

A placa, motor, ponte, sensores, fonte, módulo LCD e estrutura não estão fechados.
A referência do Printables é descrita como NEMA 17; detalhes da transmissão e
STLs não puderam ser inspecionados. Não se recomenda ainda adaptação direta para
motor CC. A interface substituível permite decidir o acionamento posteriormente,
sem implementar dois drivers completos nesta etapa.

Os sensores pontuais não permitem desaceleração antecipada precisa nem
identificação de todas as falhas físicas. Fio rompido, sensor deslocado ou saída
travada podem escapar de determinadas verificações lógicas. Corte independente,
retenção e validação em bancada são partes necessárias do projeto físico.

Próximas entregas, em ordem:

1. Fechar estrutura, transmissão, retenção, motor e carga.
2. Selecionar componentes elétricos e revisar o mapa provisório no CubeMX.
3. Implementar os drivers físicos e watchdog; validar com motor desacoplado.
4. Integrar sensores e mecânica, calibrar parâmetros e executar todos os ensaios.
5. Anexar medidas, fotos, logs e revisão dos parâmetros a este relatório.
6. Avaliar encoder, corrente e controle fechado em uma segunda versão.

## 8. Referências e documentos complementares

- [README e instruções de uso](../README.md).
- [Arquitetura e FSM](ARQUITETURA.md).
- [Integração STM32/FreeRTOS](INTEGRACAO.md).
- [Hardware, pinagem e mecânica](HARDWARE.md).
- [Roteiro de montagem e ensaio](BANCADA.md).
- [Evidências de validação](VALIDACAO.md).
- [Laboratório MIC2](https://github.com/mateusalgado/IFSC_MIC2).
- [Datasheet STM32F411](https://www.st.com/resource/en/datasheet/stm32f411ce.pdf).
- [Referência mecânica indicada](https://www.printables.com/model/1353500-elevator-lift-arduino).
