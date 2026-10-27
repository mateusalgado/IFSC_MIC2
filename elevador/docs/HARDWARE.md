# Hardware e decisões para a montagem

## Confirmado no material, ainda não na bancada

O projeto do laboratório seleciona STM32F411CEU6/UFQFPN48, HSE de 25 MHz,
SYSCLK de 96 MHz, USB de 48 MHz e TIM4 como timebase da HAL. São premissas
extraídas do `.ioc` e de `main.c`, não identificação da placa que será montada.
FreeRTOS usa SysTick. Não utilizar configuração F103 de `washmachine`.

## Componentes por função

- **Controle:** placa STM32F411CEU6 e ST-LINK/SWD; confirmar MCU, cristal,
  regulador, pinout dos conectores e USB de dados real.
- **Movimento:** motor CC com redução é a proposta. Definir tensão nominal,
  corrente em vazio, nominal, partida e bloqueio, torque, redução e eixo.
- **Potência:** ponte H com entradas compatíveis com 3,3 V, proteção e
  dissipação adequadas. Modelo, número de sinais e brake/coast pendentes.
- **Alimentação:** fonte do motor dimensionada após a escolha do motor;
  alimentação regulada da lógica, desacoplamento, proteção e retorno de corrente.
- **Interface:** joystick analógico com botão; LCD 16×2 e módulo I²C a confirmar.
  Conferir tensão, endereço e mapeamento do expansor; não assumir 0x27/PCF8574.
- **Posição:** três sensores pontuais, um por andar; escolher contato, óptico ou
  magnético, faixa de detecção, polaridade, repetibilidade e montagem.
- **Limites:** dois fins de curso independentes dos sensores dos andares.
- **Parada:** botão acessível; definir circuito de corte da potência independente
  do software e interface de diagnóstico para o MCU.
- **Sinalização:** LED com resistor e buzzer com acionamento compatível com sua
  corrente. Não alimentar cargas desconhecidas diretamente pelo GPIO.
- **Mecânica:** guias, transmissão, fixadores, cabine leve e sistema de retenção
  contra queda; definir massa, curso, carga máxima e ponto de referência.
- **Etapa futura:** sensor de corrente e encoder. Não estão medindo nem
  protegendo contra sobrecorrente nesta versão.

## Pinagem candidata — não é esquema para ligação

A comparação inicial foi feita com o `.ioc` existente e com as tabelas de pinos
e funções alternativas do [datasheet ST, DS10314 Rev. 8](https://www.st.com/resource/en/datasheet/stm32f411ce.pdf).
As funções abaixo não se sobrepõem na proposta. A exposição nos headers da placa,
pull-ups, polaridades e conflitos do esquema físico continuam pendentes.

- PA0: eixo usado do joystick, ADC1 canal 0; PA1 reservado ao segundo eixo.
- PA2: botão do joystick, GPIO.
- PA3, PA4, PA5: sensores dos andares 0, 1 e 2, GPIO.
- PB0 e PB1: limites inferior e superior, GPIO/EXTI0 e EXTI1 se necessário.
- PB10: parada, GPIO/EXTI10 se necessário.
- PA8: candidato PWM TIM1_CH1, AF1. Frequência, prescaler e ARR ainda indefinidos.
- PB12/PB13: candidatos a sinais de direção da ponte; PB14 reservado ao enable
  independente. Reavaliar totalmente se a ponte usar outra interface.
- PB6/PB7: I²C1 SCL/SDA, AF4, para o LCD.
- PB5: buzzer ativo com driver externo, se necessário.
- PC13: LED da base, polaridade/corrente a verificar.
- PA6/PA7: reserva possível para TIM3 encoder futuro; não habilitado.
- PA11/PA12: preservar USB DM/DP; PA13/PA14: preservar SWD.
- PH0/PH1: preservar HSE; reservar NRST, BOOT0 e condicionamento de boot.

PB6/PB7 também oferecem TIM4, mas nesta proposta são I²C. Não configurar PWM
TIM4: o timer já atende à HAL. PA8 não pode exercer simultaneamente PWM e MCO
ou USB SOF. PB12 não será TIM1_BKIN se usado como direção: uma futura entrada
de corte via BKIN exige rever esse sinal. EXTI compartilha linhas por número;
não atribuir dois ports à mesma linha. DMA ainda não foi atribuído: verificar
stream/canal e prioridade de ADC/I²C após a geração CubeMX. PA9 deve continuar
disponível se a implementação USB da placa exigir detecção de VBUS.

Alimentar o joystick de forma que o ADC não ultrapasse VDDA. Tolerância a 5 V
em determinados GPIOs não autoriza 5 V em entrada analógica. Verificar pull-ups
do backpack LCD e usar adaptação de nível quando exigida pelos datasheets.
Nenhuma ponte H, fonte, corrente limite ou frequência PWM foi fechada aqui.

## Referência mecânica

O modelo [Elevator Lift Arduino](https://www.printables.com/model/1353500-elevator-lift-arduino)
é descrito no resultado público do próprio Printables como “DYI Elevator with
Nema 17 motor and Arduino”. A página completa não pôde ser recuperada nesta
consulta em 27/10/2026; STLs, rolamentos, transmissão e acoplamento não foram
inspecionados. Não é possível afirmar adaptação direta a motor CC.

Antes de imprimir, inspecionar arquivos e medir: fixação e eixo do motor,
transmissão (fuso/correia/cabo), folgas, curso útil, espaço para sensores, torque
e retenção ao desligar. NEMA 17 é uma referência de formato de motor de passo;
a interface PWM/direção de motor CC desta base não é um driver STEP/DIR.
Se o grupo escolher motor de passo, preservar FSM e entradas e substituir a
interface de acionamento com requisitos e testes próprios. Não há segundo
acionamento implementado agora.

## Uso do material de MAQ

`MAQ/PWM/DriverDeMotor.pdsprj` foi aberto como contêiner; contém PROJECT.XML,
ROOT.CDB e ROOT.DSN. No conteúdo binário foi identificada referência ao modelo
“Simple DC Motor model”. Isso confirma apenas uma referência a motor CC no
projeto: não permite certificar topologia, componentes ou ligações do esquema.
É necessário abri-lo no Proteus para revisão elétrica antes de reaproveitar.
Não foi realizada simulação Proteus nesta entrega. Os materiais de instalações
prediais/transformadores não foram usados como fórmulas de acionamento CC.

## Critério de liberação do hardware

Definir primeiro motor + transmissão + retenção + massa. Depois dimensionar
ponte/fonte/proteções, fechar esquema e mapa CubeMX, implementar o port e executar
o roteiro de bancada. Não há controle de velocidade PI nem frenagem regenerativa
implementados. Uma partida gradual em PWM limita comando; não limita corrente
de forma medida e não substitui proteção elétrica.
