# Leitura técnica de Game_Start2_STM32F411CE

Inspeção dos arquivos fornecidos em 27/10/2026. As primeiras seções descrevem
o ZIP original; as alterações atuais estão na seção final de adaptação.

## O que existia no ZIP original

- STM32F411CEU6, configuração de CPU para 96 MHz e HAL com timebase TIM4.
- FreeRTOS com CMSIS-RTOS v2 e chamadas nativas FreeRTOS no exemplo.
- Display ST7735 configurado para 160×128 pixels, SPI1, fontes e figuras.
- Joystick em PA0/PA1, ADC1 de 12 bits, dois canais e DMA circular
  DMA2_Stream0/canal 0. Botão em PC15 com pull-up.
- SPI1 SCK em PA5 e MOSI em PA7. Display: CS PB12, RST PB13 e DC PB14.
- Tela “Press Start!”, espera pelo botão, movimento de personagem pelo joystick,
  tarefas para desenhar figuras/exibir ADC e LED periódico em PC13.
- Não há integração USB CDC no ZIP. Não há implementação completa de áudio,
  três níveis de dificuldade, regras de derrota ou reinício da partida.

## Pontos para resolver ao desenvolver o jogo

1. **Clock divergente:** o `.ioc` reserva PH0/PH1 para HSE e registra HSE de
   25 MHz; `SystemClock_Config` em `main.c` utiliza HSI com PLLM=16, PLLN=192,
   PLLP=2. Não regenerar presumindo que código e configuração estejam alinhados.
2. **Display compartilhado:** três tarefas chamam o ST7735 diretamente; o driver
   não protege a transação completa. Intercalação de CS/DC/comandos/pixels pode
   corromper a apresentação. Preferir uma tarefa dona da renderização e mensagens.
3. **SPI bloqueante:** o driver usa `HAL_SPI_Transmit` com `HAL_MAX_DELAY` e
   não trata seu resultado. Definir timeout e comportamento de falha no jogo;
   não colocar renderização dentro de uma tarefa que exija resposta rápida.
4. **Dados ADC:** callback de conversão atualiza dois elementos globais que as
   tarefas leem sem protocolo de publicação de uma amostra coerente. Planejar
   comunicação ISR→tarefa, idade/validade da amostra e filtragem/calibração.
5. **Inicialização:** a espera do botão ocorre em laço ocupado antes do scheduler.
   Na aplicação final, representar a tela inicial como estado com espera RTOS,
   permitindo áudio, entrada e apresentação desde o início.
6. **Joystick:** limites 1800/3800 estão fixos no exemplo. Medir centro, extremos,
   polaridade, ruído e debounce do botão real antes de definir comandos.
7. **APIs/prioridades:** a base usa `configMAX_PRIORITIES=56` e prioridades CMSIS
   em chamadas `xTaskCreate`. Não transportar esses números para a configuração
   do laboratório anterior, que tem sete prioridades. Definir uma convenção clara.
8. **Memória e periodicidade:** verificar retorno da criação de tarefas, dimensionar
   pilhas, habilitar diagnóstico apropriado e usar `pdMS_TO_TICKS`/periodicidade
   explícita. Os tempos do exemplo são expressos diretamente em ticks.

Esta base serve para aprender e iniciar o desenvolvimento. Ela não deve ser
descrita como um jogo concluído apenas por compilar ou mover uma figura.

## Verificação inicial

Em 27/10/2026, `scripts/build_base.py` compilou e ligou as fontes originais com
GNU Tools for STM32 14.3.rel1, GCC 14.3.1. Foram ligados 42 objetos;
GNU `size`: `text=32260`, `data=120`, `bss=21168` bytes, configuração sem
otimização (`-O0`). Não houve avisos nessa execução com `-Wall`.

As 154 fontes/configurações extraídas foram comparadas byte a byte com as
entradas correspondentes do ZIP. Não foram executados testes em placa, testes
de jogabilidade nem importação pela interface gráfica do CubeIDE. A base inclui
os pontos de revisão acima mesmo tendo compilado.

## Diagnóstico anterior (commit 4382347)

- SPI1 em PA5/PA7 e sinais CS/RST/DC em PB12/PB13/PB14 preservados.
- Uma tarefa CMSIS-RTOS v2, com 2048 bytes de pilha, é dona de todo o display.
  Não há transações concorrentes entre as três tarefas gráficas do exemplo.
- A tela inicia sem aguardar o botão; mostra RGB, X/Y, SW e um cursor.
- ADC/DMA circular com quatro meias-palavras: cada callback lê a metade recém
  concluída, enquanto a outra recebe a próxima conversão. Publica X/Y em uma
  palavra de 32 bits; a tarefa faz uma única leitura para obter o par.
- Callbacks apenas publicam amostra e instante; não desenham nem esperam.
- Atualização nominal de 50 ms com `vTaskDelayUntil`, LED a cada 500 ms,
  verificação da criação da tarefa e do início do ADC.
- O teste exibe valores brutos e o estado bruto do botão. Calibração, debounce,
  limites de latência e medição da margem de pilha ficam para a aplicação final.
- O SPI bloqueante e a divergência de clock no `.ioc` continuam como pontos de
  revisão. Não regenerar o projeto antes de reconciliar o clock. A compilação
  usa o `main.c` existente com HSI/PLL em 96 MHz.

Ver [TESTE_BANCADA.md](TESTE_BANCADA.md) para gravação e aceitação física.

## Primeiro mapa (M01 v0.1)

O código padrão atual usa três tarefas e um único dono do display, com filas de
amostra, entrada, cliques, snapshots e confirmação de cena. ADC/DMA passou para
64 meias-palavras com média de 16 pares por metade. O ST7735 agora limita a
espera SPI, expõe erro, valida bordas e preenche retângulos com buffer fixo,
sem malloc por desenho. A lógica e a interface possuem testes no computador.
O clock real HSI/PLL de 96 MHz foi mantido; a reconciliação/regeneração do `.ioc`
continua pendente. O plano completo de nove mapas ainda está em execução.
Veja [PRIMEIRO_MAPA.md](PRIMEIRO_MAPA.md).
