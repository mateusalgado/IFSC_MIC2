# Hardware do jogo

## Confirmado pelo usuário

Em 27/10/2026: jogo com **tela, joystick e sons, sem motor**.

- Display informado: **“1.8 TFT SPI 128*60 V1.1”**.
- Joystick informado: **“MH preto”**.
- Buzzer: ainda não adquirido.
- Placa STM32 física: modelo ainda não informado; o projeto do professor seleciona F411CEU6.

“MH preto” não identifica sozinho fabricante, pinout ou componentes do joystick.
O texto do display também não confirma controlador ou requisitos elétricos.
Existem telas de 1,8 polegada com resolução 128×160 e ST7735S, como o
[módulo da Waveshare](https://www.waveshare.com/wiki/1.8inch_LCD_Module), mas essa
referência não identifica a placa do usuário. É possível que “128*60” tenha sido
uma transcrição de “128*160”; isso ainda precisa ser confirmado pela inscrição.

## Sinais propostos a partir da base do professor

Este mapa descreve funções, não a posição física dos conectores. Confirmar
as inscrições do módulo e da placa antes da montagem.

Display, caso seja compatível com o ST7735/SPI da base:

- Clock SCK, por vezes identificado SCL: **PA5 / SPI1_SCK**.
- Entrada de dados MOSI, por vezes identificada SDA: **PA7 / SPI1_MOSI**.
- Seleção CS: **PB12**.
- Reset RST/RES: **PB13**.
- Seleção dados/comando DC/A0: **PB14**.
- GND: referência comum com o STM32.
- VCC e LED/BL: **pendentes de identificar o módulo**, regulador e circuito de backlight.

SCL/SDA nessa lista são possíveis nomes impressos para sinais SPI; não significam
que se deve configurar I²C. Não conectar terminal identificado apenas como LED
diretamente ao GPIO sem identificar seu circuito.

Joystick, caso seja o módulo passivo de dois potenciômetros e botão:

- VRY, eixo Y, correspondente a `valor_ADC[0]`: **PA0 / ADC1_IN0**.
- VRX, eixo X, correspondente a `valor_ADC[1]`: **PA1 / ADC1_IN1**.
- Botão SW: **PC15**, com pull-up e fechamento para GND, como no exemplo.
- GND: referência comum; alimentação proposta dos potenciômetros: **3,3 V**,
  condicionada à confirmação do módulo e da alimentação analógica da placa.

O código do professor usa ADC[1] como X e ADC[0] como Y. Conferir as inscrições
VRX/VRY e eventual inversão de sentido ao testar o joystick físico. PC15 também tem função de
oscilador de baixa frequência; verificar se está disponível na placa escolhida.
As entradas ADC devem respeitar a faixa analógica do MCU; não presumir que a
tolerância de determinados GPIOs a 5 V se aplique ao ADC. Referência:
[datasheet STM32F411](https://www.st.com/resource/en/datasheet/stm32f411ce.pdf).

## Som a selecionar

Preferência de projeto: **transdutor piezoelétrico de excitação externa**, vendido
frequentemente como buzzer passivo, para gerar notas e efeitos pelo timer. Modelo,
tensão, frequência de operação, capacitância/corrente e driver ainda serão escolhidos.
Não há lista de compra final ou autorização de ligação direta ao GPIO.

Reservar provisoriamente **PA8 / TIM1_CH1** para o sinal de áudio. Isso não conflita
com ADC/SPI/SWD da base; TIM4 continua como timebase HAL. A configuração de áudio
ainda não foi adicionada ao firmware.

Um buzzer com oscilador interno serve para bipes, mas oferece menor liberdade
para produzir diferentes notas. A escolha do circuito depende do componente:
consultar os [exemplos de acionamento da Murata](https://www.murata.com/products/sound/library/basic/circuit)
e o datasheet do modelo escolhido antes de dimensionar transistor e resistores.

## Informação necessária para fechar o esquema

Conferir fotos legíveis da frente/verso do display, inscrições dos terminais do
joystick e modelo da placa STM32. Com isso, fechar resolução/controlador,
alimentação e backlight, posição de cada fio e seleção do buzzer/driver.
As fontes do professor permanecem inalteradas até a escolha da implementação.
