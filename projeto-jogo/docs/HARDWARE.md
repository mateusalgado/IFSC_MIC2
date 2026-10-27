# Hardware do jogo

## Confirmado pelo usuário

Em 27/10/2026: jogo com **tela, joystick e sons, sem motor**.

- Display: a foto posterior confirma a inscrição **“1.8 TFT SPI 128*160 V1.1”**.
- Joystick: módulo com terminais **SW, VRY, VRX, +5V e GND**, confirmados na foto.
- Buzzer: ainda não adquirido.
- Placa STM32 física: modelo ainda não informado; o projeto do professor seleciona F411CEU6.

A foto confirma o layout vermelho com leitor microSD e conector VCC, GND, CS,
RESET, A0, SDA, SCK, LED. O fabricante e a marcação do controlador não estão
legíveis. O aluno relatou funcionamento do diagnóstico com o driver ST7735;
a identificação exata do controlador continua sem confirmação visual.
A placa STM32 tem marcações abreviadas A0/A1/B12/C15 e alimentação 3V3/G.
O modelo exato gravado no MCU não é legível na foto.

## Ligação definida para o teste — SPI1 por hardware

Usar os nomes impressos nos conectores, não a posição aparente dos fios na foto:
A5 significa PA5, B12 significa PB12 e C15 significa PC15. G significa GND.
Não confundir A0 do display (dados/comando) com A0 da placa STM32.

Decisão final: **SDA em A7 e SCK em A5**, mantendo a ligação original.
As alternativas com A8/A9 ou SPI2 não foram adotadas.

Display (funcionamento com o driver ST7735 relatado pelo aluno):

- Clock SCK, por vezes identificado SCL: **PA5 / SPI1_SCK**.
- Entrada de dados MOSI, por vezes identificada SDA: **PA7 / SPI1_MOSI**.
- Seleção CS: **PB12**.
- Reset RST/RES: **PB13**.
- Seleção dados/comando DC/A0: **PB14**.
- GND: **G/GND** do STM32.
- VCC: **3V3** do STM32, para o ensaio inicial.
- LED: **3V3**, para iluminação contínua; não usar PC13 nem outro GPIO como alimentação.

A opção de alimentação em 3,3 V e LED em 3,3 V corresponde à documentação do
[módulo de mesmo layout V1.1](https://shillehtek.com/blogs/shillehtek-product-manuals/shillehtek-1-8-inch-tft-lcd-display-module-dual-side-soldering-manual)
e do [MSP1803](https://www.lcdwiki.com/1.8inch_SPI_Module_ST7735S_SKU:MSP1803).
Isso não comprova fabricante ou revisão elétrica da unidade fotografada. Não
extrapolar a ligação para 5 V. Validar consumo, iluminação e imagem em bancada.

SCL/SDA nessa lista são possíveis nomes impressos para sinais SPI; não significam
que se deve configurar I²C. Não conectar terminal identificado apenas como LED
diretamente ao GPIO sem identificar seu circuito.

Joystick, caso seja o módulo passivo de dois potenciômetros e botão:

- VRY, eixo Y, primeiro canal da sequência ADC: **PA0 / ADC1_IN0**.
- VRX, eixo X, segundo canal da sequência ADC: **PA1 / ADC1_IN1**.
- Botão SW: **PC15**, com pull-up e fechamento para GND, como no exemplo.
- GND: **G/GND** do STM32.
- Terminal marcado **+5V** no joystick: conectar em **3V3**, não em 5 V.
  O módulo de potenciômetros será alimentado em 3,3 V para que suas saídas
  analógicas acompanhem a faixa de alimentação do ADC.

A ordem de conversão do professor foi mantida: primeiro Y, depois X. Conferir as inscrições
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

## Preparação da bancada

Fazer conexões sem alimentação. Separar a placa STM32 da chapa metálica do leitor
microSD vista na foto para evitar contato das soldas com a blindagem. Compartilhar
3V3 e GND por protoboard/distribuição, sem sobrepor conectores num mesmo pino.
Deixar SD_CS, SD_MOSI, SD_MISO e SD_SCK desconectados; não há uso de microSD no jogo.
Não é possível certificar as conexões atuais pela foto, pois as outras pontas
dos fios dos módulos não aparecem.

Faltam seleção e circuito do buzzer, identificação legível do MCU e medições
de calibração/estabilidade. O teste funcional do diagnóstico foi relatado como
bem-sucedido pelo aluno em 27/10/2026.
PA8/A8 permanece reservado, sem ligação de buzzer nesta etapa. O firmware padrão
é o [primeiro mapa jogável](PRIMEIRO_MAPA.md); o diagnóstico de
[TESTE_BANCADA.md](TESTE_BANCADA.md) continua opcional. A pinagem SPI1, ADC e botão
do professor foi mantida.
