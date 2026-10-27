# Evidências e alcance da validação

Execução local em **27/10/2026**. Repositório base MIC2:
`278dc298ca86d6c2ff089cdee511aa2cb771fd05`. As fontes novas ficam em `elevador/`.
O histórico Git identifica a revisão entregue; futuros resultados devem indicar
explicitamente o commit e os componentes usados.

## Testes no computador

Comandos executados em Ubuntu/WSL, GCC 15.2.0, C11:

```sh
make test
make sanitize
```

Resultado: **24 grupos aprovados**, tanto na execução normal como com
AddressSanitizer + UndefinedBehaviorSanitizer. Compilação do código próprio com
`-Wall -Wextra -Werror -pedantic`; sanitizadores sem ocorrências nessa execução.
Os casos são determinísticos, usam relógio simulado e não contêm esperas reais.

Cobertura por grupo (identificadores em `tests/test_elevator.c`):

1. `test_boot`: inicia sem referência e recusa viagem antes de referência.
2. `test_home`: referência a partir de posição desconhecida.
3. `test_existing_ground`: referência quando sensor do térreo já está ativo.
4. `test_both_directions`: subida 0→2 e descida 2→0 com sensor intermediário.
5. `test_same_floor`: pedido do próprio andar sem acionamento.
6. `test_busy`: seleção, confirmação e pressão longa recusadas durante viagem.
7. `test_home_timeout`: referência não encontra térreo no prazo.
8. `test_travel_timeout`: destino não é confirmado no prazo.
9. `test_sensors`: sensores simultâneos, andar pulado e sensor de partida preso.
10. `test_limits`: limites inferior, superior e ambos ativos; reconhecimento bloqueado.
11. `test_stop_recovery`: parada em movimento, causa ativa, reconhecimento separado da referência.
12. `test_reset`: reset durante movimento perde referência e não retoma.
13. `test_position_loss`: perda do sensor enquanto parado.
14. `test_deadline`: excesso do intervalo máximo de controle.
15. `test_invalid_sample`: amostra inválida impede operação.
16. `test_wrap`: referência e timeout através do rollover de `uint32_t`.
17. `test_motor`: teto de PWM, rampa, troca de sentido apenas após zero/intervalo,
    corte por falha e bloqueio de reversão imediata após corte.
18. `test_joystick`: repouso, uma seleção sustentada e rearme apenas no centro físico.
19. `test_debounce`: rejeição de pulso de sensor e prioridade de parada bruta.
20. `test_button`: botão segurado no boot, clique e pressão longa única.
21. `test_display`: “entre andares”, último confirmado/destino e linhas de 16 caracteres.
22. `test_safety_sweep`: parada prioritária nos sete estados e cinco eventos (35 combinações).
23. `test_integrated_raw_trip`: entradas brutas → filtros → FSM → chegada,
    sem aceitar pressão iniciada em viagem como novo pedido após chegar.
24. `test_fault_requires_new_press`: pressão iniciada antes da falha não serve como reconhecimento.

As invariantes auxiliares verificam limite de comando e saída inativa nos
estados sem movimento. Não foi medida cobertura de linhas/branches; “24 grupos”
não significa cobertura exaustiva de falhas, ruídos ou estados possíveis.

## Compilação ARM

O script `scripts/build_f411.py` foi executado com GNU Tools for STM32
14.3.rel1 do STM32CubeIDE instalado, alvo Cortex-M4F/hard-float e `STM32F411xE`.
Compilação e ligação concluídas com HAL, FreeRTOS, USB, startup e linker reais
da base local. O mapa e o ELF são gerados em `build/f411`, ignorados pelo Git.

Build local final: 51 objetos ligados; `text=24732`, `data=332`, `bss=20264`
bytes conforme GNU `size`. Esses números descrevem o port inativo desta revisão,
não orçamento final de RAM/flash com ADC, I²C e driver de motor. O uso efetivo de
pilha/heap em execução não foi medido.

Há avisos de parâmetros não usados em fontes herdadas HAL/CMSIS/USB. Não foram
silenciados nem corrigidos no projeto original. As fontes novas usam `-Werror`.
O código de placa é `board_unconfigured.c`: nenhum GPIO/PWM do elevador é
acionado, e a amostra inválida leva à falha de entrada.

## Não executado nesta etapa

- Gravação e execução em STM32 físico; medição de deadlines ou jitter FreeRTOS.
- Importação/compilação pela interface gráfica do STM32CubeIDE.
- Ensaio de USB físico, endereço I²C/LCD, ADC e polaridade de sensores.
- Teste de corte físico, watchdog real, stack high-water mark e falhas de energia.
- Simulação Proteus do circuito MAQ ou avaliação de precisão de parada mecânica.
- Medição de corrente, velocidade, torque, carga ou temperatura.

O roteiro para essas verificações está em [BANCADA.md](BANCADA.md). Resultados
futuros devem ser acrescentados com instrumentos, configuração e evidências.
Não preencher valores estimados como se fossem medições.
