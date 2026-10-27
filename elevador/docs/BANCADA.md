# Montagem, calibração e ensaios

## Antes de montar

1. Confirmar placa, MCU e cristal. Revisar esquema e pinagem candidata.
2. Escolher a estrutura e transmissão; conferir motor/torque, curso, massa e
   retenção mecânica. O mecanismo não pode depender de PWM para impedir queda.
3. Selecionar ponte e fonte pela corrente de partida/bloqueio e pela dissipação,
   não só pela corrente em vazio. Prever proteção e corte físico acessível.
4. Posicionar sensores 0/1/2 sem zonas de detecção sobrepostas. Posicionar fins
   de curso além das paradas normais, com margem antes de colisão. A referência
   deve encontrar sensor 0 antes do limite inferior.
5. Separar caminho de potência e sinais; planejar desacoplamento, retorno de
   corrente e referência comum quando exigida pela interface. Verificar tensão
   em cada entrada STM32 e no barramento I²C antes de conectar.

## Etapa A — sem motor conectado

- Implementar o port após fechar componentes. Medir boot/reset: PWM e enable
  devem ficar inativos desde a energização, inclusive antes de HAL/RTOS.
- Acionar cada sensor individualmente e verificar bit e andar. Acionar dois:
  falha deve prevalecer sobre clique/joystick. Confirmar comportamento de cabo
  desconectado; o software atual não identifica sozinho todo circuito aberto.
- Calibrar centro/faixa do joystick, sentido do eixo, ruído e zona morta.
  Manter deslocado: uma seleção; voltar ao centro: rearmar. Botão segurado
  no boot não pode iniciar movimento; clique e pressão longa não se confundem.
- Verificar LCD 16×2, dois textos completos e endereço real do módulo.
- Testar USB desconectada, ocupada e reconectada; medir período/jitter do controle
  e comparar com o prazo de 50 ms. Medir high-water mark das três pilhas e heap.
- Verificar corte por parada, fins de curso e watchdog independentemente de LCD/USB.

## Etapa B — motor desacoplado

- Usar fonte com limitação adequada, motor preso e corte acessível. Medir saída
  da ponte antes de acoplar transmissão. Confirmar sentido e tabela brake/coast.
- Aumentar comando por ensaios controlados para identificar limiar de partida,
  corrente, aquecimento e rampa. Ajustar teto, referência e viagem em
  `elevator_config.h`; os valores de simulação não autorizam esses limites.
- Selecionar frequência PWM conforme motor e ponte; calcular PSC/ARR usando
  clock real do timer e medir com osciloscópio. Não confundir duty com velocidade.
- Confirmar PWM zero antes da troca de direção e intervalo neutro. O motor pode
  continuar girando após corte; 300 ms é apenas hipótese inicial.

## Etapa C — mecânica montada, baixa carga

1. Instalar retenção e sustentar a cabine durante ajuste. Não usar passageiros.
2. Ligar: nenhum movimento. Solicitar referência explicitamente; descer devagar
   até sensor 0. Se atingir fim inferior primeiro, falha e correção mecânica,
   sem tentativa automática de sair do limite.
3. Viajar 0→1, 1→2, 2→1, 1→0, 0→2 e 2→0. Medir ultrapassagem após sensor e
   permanência no andar. Ajustar velocidade/comando e largura de detecção.
4. Testar pedido do andar atual e pedidos durante viagem. Confirmar recusa,
   ausência de fila e indicação “entre andares” sem posição inventada.
5. Interromper cada direção com parada, sensor incoerente e limite. Desligar e
   religar durante movimento em ensaio contido. Não deve retomar o destino.
6. Simular sensor ausente e preso, perda de referência e travamento de execução
   com motor desconectado primeiro. Verificar timeouts e reação do watchdog.
7. Recuperar: remover causa, reconhecer, permanecer inativo; solicitar referência
   separadamente. Quando um limite estiver ativo, reposicionamento deve ocorrer
   manualmente e com potência isolada; não há modo de escape automático.

## Comparação de Máquinas e Acionamentos

Repetir percursos de subida e descida com cabine vazia e cargas conhecidas,
dentro de capacidade previamente definida. Para cada configuração repetir pelo
menos três vezes; guardar resultados brutos, média e dispersão, sem preencher
medições que ainda não foram realizadas.

Registrar data, commit, operador, motor/redução, carga, tensão da fonte, frequência
PWM, duty, rampa, sentido, percurso, tempo, corrente de partida e regime medida
por instrumento externo, temperatura e ultrapassagem no andar. Conservar logs
USB e, quando disponíveis, capturas dos sinais PWM/direção/enable.

Comparar efeito da carga e da gravidade sobre tempo e corrente, estabilidade ao
parar e capacidade de retenção. Relacionar torque, redução e massa; não transferir
equações de motores trifásicos diretamente para motor CC. Dimensionamento numérico
depende dos dados reais, atrito, eficiência e geometria da transmissão.

## Critérios para encerrar esta etapa física

Todos os percursos devem parar e permanecer no sensor, sem atingir limites;
falhas devem ser reconhecidas sem retomada; reset deve inibir potência; retenção
deve funcionar sem energia. Definir tolerância de parada e limites térmicos e
elétricos antes do ensaio, conforme componentes. Anexar evidências ao relatório.

## Próxima etapa

Adicionar medição de corrente calibrada e proteção apropriada; instalar encoder
ou sensor de aproximação para medir velocidade/posição relativa. Só então
projetar desaceleração e eventual PI, com unidades, taxa de amostragem, saturação
e ensaios próprios. Fila de chamadas, portas e comunicação de comandos ficam fora
da primeira entrega.
