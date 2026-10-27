# Diagnóstico opcional de bancada

O build padrão agora é o [primeiro mapa jogável](PRIMEIRO_MAPA.md). Para esta
tela de diagnóstico, usar `--diagnostic` no script ou `APP_DIAGNOSTIC=1` em
`game_config.h` no CubeIDE, seguido de Clean/Build.

Este firmware opcional é um diagnóstico baseado no material do professor.
Não contém ainda as regras do jogo ou áudio. A pinagem definitiva deste ensaio
é a original com **SPI1 por hardware: SDA A7 e SCK A5**.

## Conferir antes de alimentar

Display, na ordem dos nomes impressos no módulo:

1. VCC → 3V3.
2. GND → G.
3. CS → B12.
4. RESET → B13.
5. A0 → B14 (dados/comando, não o A0 da placa).
6. SDA → A7.
7. SCK → A5.
8. LED → 3V3.

Joystick: VRY → A0, VRX → A1, SW → C15, GND → G e terminal +5V → **3V3**.
O nome +5V impresso no módulo não muda a alimentação de 3,3 V definida aqui.
Deixar microSD e buzzer desconectados. Separar o STM32 da chapa metálica do
leitor microSD. Detalhes e referências: [HARDWARE.md](HARDWARE.md).

## Compilar e gravar

No STM32CubeIDE, importar `Game_Start2_STM32F411CE` como projeto existente,
compilar Debug e gravar com o ST-Link. Usar o projeto de `projeto-jogo`, não
o projeto separado dos exemplos do laboratório. Não regenerar pelo CubeMX
nesta etapa: há uma divergência de clock herdada do exemplo, descrita em
[BASE_PROFESSOR.md](BASE_PROFESSOR.md).

Também é possível usar `scripts/build_base.py`, conforme o README. O script
com `--diagnostic` gera `build/diagnostico/diagnostico-spi1.elf`, `.hex` e `.bin`. Pelo STM32CubeProgrammer,
conectar ao ST-Link, abrir o `.hex`, programar com verificação e reiniciar.
O `.hex` já contém os endereços; se optar pelo `.bin`, usar **0x08000000**.
Não é necessário alterar BOOT0 para gravar por SWD/ST-Link.

O GitHub Actions disponibiliza os mesmos três formatos no artefato
`mic2-diagnostico-spi1`, associado a cada execução bem-sucedida. Usar a execução
do commit desejado, não um binário antigo extraído do ZIP do professor.
Não combinar duas fontes de alimentação de 3,3 V: definir uma única fonte
para a montagem e manter GND comum ao programador.

## Resultado esperado

- A tela inicia automaticamente com **MIC2 TESTE SPI1**; não espera pressionar SW.
- Três barras aparecem, da esquerda para a direita: vermelho, verde e azul.
- `X A1` e `Y A0` mostram valores brutos do ADC de 12 bits, entre 0 e 4095.
  O centro deve ficar aproximadamente na metade da faixa; medir o valor real.
- `SW: SOLTO` muda para `SW: PRESSIONADO` enquanto o joystick estiver pressionado.
  É uma leitura bruta para diagnóstico, ainda sem debounce de comandos de jogo.
- `ADC: OK` indica callbacks recentes. `ADC: SEM DADOS` indica ausência de
  amostra inicial ou mais de 250 ms sem atualização observada.
- Um quadrado verde se desloca no retângulo inferior conforme os dois eixos.
  Nesta etapa o cursor acompanha os valores brutos; a orientação final será
  ajustada após verificar como o joystick será montado.
- O LED da placa alterna aproximadamente a cada 500 ms. A tarefa atualiza a
  apresentação com período nominal de 50 ms; os tempos reais ainda serão medidos.

Há uma única tarefa acessando o display. As interrupções ADC/DMA publicam os
dados; não fazem desenho. O ADC usa duas metades de buffer alternadas (média de 16 pares por metade), e a tarefa
lê X/Y juntos em uma palavra de 32 bits. O teste usa HSI/PLL em 96 MHz e SPI1
com prescaler 8 (12 MHz nominais).

## Registrar o ensaio

Status atualizado em 27/10/2026: **o aluno informou que gravou o firmware de
teste e que tudo funcionou**. Trata-se de confirmação funcional relatada pelo
usuário, não de medição independente. Ainda faltam valores de calibração,
identificação legível do MCU, duração do ensaio e evidências fotográficas.
Preencher os campos abaixo com medições reais:

- Data, commit gravado e identificação legível do MCU: ______.
- Imagem completa, orientação e sequência das três cores: ______.
- X: centro ______; mínimo ______; máximo ______; sentido observado ______.
- Y: centro ______; mínimo ______; máximo ______; sentido observado ______.
- Botão: solto ______; pressionado ______; resultado de dez acionamentos ______.
- Cursor percorre toda a área delimitada: ______.
- Cinco minutos movimentando os eixos: falhas de imagem/reinícios/ADC parado ______.
- Foto ou vídeo do resultado e observações: ______.

Se só houver iluminação, conferir CS, RESET, A0/DC, SDA e SCK, além do firmware
gravado. Cores trocadas, deslocamento ou corte da imagem podem exigir ajustar
ordem RGB/BGR, orientação ou offsets do ST7735 após identificar a variante.
Se um eixo não variar, conferir alimentação em 3V3, GND e VRX/VRY. Não concluir
que um módulo funciona apenas porque a compilação passou.

## Verificação de software realizada

Na versão original `4382347`, em 27/10/2026, o diagnóstico compilou e ligou 42 objetos com GNU Tools for STM32
14.3.rel1 (GCC 14.3.1), sem avisos de `-Wall`. Resultado: text 31864 bytes,
data 104 bytes e bss 21176 bytes. Foram gerados ELF, HEX e BIN.
Essa execução confirmou somente a compilação. Posteriormente, o aluno relatou
gravação e funcionamento do diagnóstico; ver o registro de status acima.

Próximas entregas: [ESCOPO.md](ESCOPO.md).
