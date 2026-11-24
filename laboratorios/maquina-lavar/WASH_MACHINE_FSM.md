# Máquina de Lavar - FSM (Finite State Machine)

## Visão Geral

Implementação de uma máquina de estados para controlar uma máquina de lavar horizontal com abertura de tampa, utilizando ponteiros para função no STM32F1xx.

## Arquitetura

### Estados da Máquina

| Estado | Descrição | Saídas |
|--------|-----------|--------|
| **IDLE** | Parada, aguardando início. Tampa aberta | Nenhuma |
| **DOOR_CLOSED** | Tampa fechada e bloqueada | Travas ativas |
| **FILLING** | Enchendo com água | Válvula de entrada aberta |
| **WASHING** | Ciclo de lavagem. Tambor rodando | Motor ligado |
| **DRAINING** | Drenando a água | Válvula de drenagem aberta |
| **SPINNING** | Centrifugando/secando | Motor em alta velocidade |
| **READY** | Ciclo completo, pronto para abrir | Travas desativadas |
| **ERROR** | Estado de erro (ex: tampa aberta) | Parada de emergência |

### Fluxo Principal

```
IDLE 
  ↓ (START button)
DOOR_CLOSED 
  ↓ (Initialization)
FILLING 
  ↓ (Level reached)
WASHING 
  ↓ (Timer finished)
DRAINING 
  ↓ (Water empty)
SPINNING 
  ↓ (Cycle done)
READY 
  ↓ (Door opened)
IDLE
```

### Proteções de Segurança

- **Tampa aberta durante ciclo** → Transição para ERROR
- **Reset em ERROR** → Volta para IDLE
- **Parada de emergência em qualquer estado**

## Implementação

### Estrutura de Dados

```c
typedef enum {
    STATE_IDLE,
    STATE_DOOR_CLOSED,
    STATE_FILLING,
    STATE_WASHING,
    STATE_DRAINING,
    STATE_SPINNING,
    STATE_READY,
    STATE_ERROR
} WashMachineState;

typedef struct {
    WashMachineState current_state;
    uint32_t timer_ms;
    void (*state_handler)(void);
} WashMachine;
```

### Ponteiros para Função

Cada estado tem um handler dedicado que implementa a lógica:

- `handle_idle()` - Aguarda botão START
- `handle_door_closed()` - Bloqueia e inicializa
- `handle_filling()` - Controla enchimento
- `handle_washing()` - Executa lavagem
- `handle_draining()` - Drena água
- `handle_spinning()` - Centrifuga
- `handle_ready()` - Aguarda abertura
- `handle_error()` - Gerencia erro

### Transições

A máquina realiza transições através da função:
```c
void transition_to(WashMachineState next_state);
```

## Sensores e Entradas (Simulados)

Para o Proteus:

| Entrada | Pino | Descrição |
|---------|------|-----------|
| **BTN_START** | PA0 | Botão para iniciar ciclo |
| **SENSOR_DOOR** | PA1 | Sensor de porta (1=fechada) |
| **SENSOR_LEVEL** | PA2 | Sensor de nível de água |
| **SENSOR_EMPTY** | PA3 | Sensor de tanque vazio |

## Saídas (Simuladas com LEDs)

| Saída | Pino | Descrição |
|-------|------|-----------|
| **LED_STATUS** | PA7 | Status geral (intermitente = ciclo) |
| **MOTOR** | PA6 | Motor do tambor |
| **VALVE_IN** | PA5 | Válvula de entrada |
| **VALVE_OUT** | PA4 | Válvula de saída |

## Ciclos de Tempo (para Proteus)

- Cada estado executa por ~2 segundos (para visualização rápida)
- Timer de 2000ms por etapa
- Facilita testes e validação no simulador

## Testes no Proteus

1. **Teste 1**: Sequência completa (START → ciclo completo)
2. **Teste 2**: Erro de porta (porta aberta durante ciclo)
3. **Teste 3**: Reset de erro
4. **Teste 4**: LED de status piscando durante ciclo

## Arquivos

- `wash_machine_fsm.h` - Header da FSM
- `wash_machine_fsm.c` - Implementação da FSM
- `main.c` - Integração e loop principal

## Próximos Passos

1. ✅ Implementação básica da FSM
2. 🔄 Integração de sensores reais
3. 🔧 Calibração de tempos
4. 📊 Testes em hardware real
