# Mini elevador didático — Microcontroladores II

Firmware inicial em C para uma maquete de três paradas (térreo, 1º e 2º andar),
integrando máquina de estados, FreeRTOS e conceitos de Máquinas e Acionamentos.
Alvo inicial: **STM32F411CEU6**, ainda sujeito à confirmação da placa física.

**Situação:** núcleo executável e testável no computador; integração ARM com o
FreeRTOS/HAL/USB do laboratório. A montagem e os drivers da placa estão pendentes.
O port fornecido não aciona pinos: retorna entrada inválida e mantém a aplicação
em falha. Compilar não significa que a cabine esteja pronta para operar.

## Começar pelos testes

Linux, WSL ou ambiente com GCC e Make, dentro desta pasta:

```sh
make test
make sanitize
```

No PowerShell, a partir da raiz MIC2 local:

```powershell
wsl -d Ubuntu -- make -C /mnt/c/Users/mateu/OneDrive/Documentos/MIC2/elevador test
```

Os testes não dependem de HAL, placa, internet ou FreeRTOS. Simulam sensores,
comandos e relógio. `sanitize` acrescenta verificações de memória e comportamento
indefinido. Não usar `-DNDEBUG`, pois os testes empregam `assert`.

## Documentação

- [Relatório técnico e vínculo com a disciplina](docs/RELATORIO.md).
- [Arquitetura, estados e operação](docs/ARQUITETURA.md).
- [Compilação ARM e integração no STM32CubeIDE](docs/INTEGRACAO.md).
- [Componentes, pinagem provisória e decisões pendentes](docs/HARDWARE.md).
- [Montagem, calibração e roteiro de bancada](docs/BANCADA.md).
- [Evidências e limites da validação](docs/VALIDACAO.md).

## Organização

```text
include/              Contratos e parâmetros provisórios
src/                  FSM, entradas, motor e formatação LCD, sem HAL
tests/                Testes determinísticos no computador
ports/stm32f411/       Três tarefas FreeRTOS, USB, startup e port inativo
scripts/              Compilação ARM reproduzível
docs/                 Relatório, montagem, decisões e evidências
```

## Operação prevista

1. Após ligar, o motor permanece inativo. Soltar o botão antes de usá-lo.
2. Segurar o botão do joystick por pelo menos 1 s e soltar para iniciar referência.
3. A cabine desce com comando limitado até confirmar o sensor do térreo.
4. Com a cabine parada, mover o joystick uma vez para selecionar o andar;
   voltar ao centro antes de outra seleção. Clique curto confirma.
5. Pedidos durante referência, viagem ou chegada são recusados e descartados.
6. Em falha, corrigir a causa, liberar a parada/limites e segurar/soltar para
   reconhecer. Isso retorna à inicialização sem movimento. Uma nova ação longa
   solicita outra referência. O destino anterior não é retomado.

O LCD alterna informação de último andar confirmado/destino com “entre andares”.
PWM é percentual de comando, não velocidade medida. A primeira versão para no
sensor, em movimento lento; não estima distância contínua nem desaceleração precisa.

## Dependências e preservação

O build ARM reutiliza, sem modificar, `ex_Barry_USB_STM32F411CE` deste repositório
(no workspace local ele fica em `labRTOS/`). O script reconhece ambas as estruturas.
HAL STM32F4, CMSIS, FreeRTOS 10.3.1, USB CDC, startup e linker permanecem na base
original. A configuração FreeRTOS local foi derivada dela, conservando a licença.
`washmachine` foi consultado como referência de FSM, sem importar HAL, pinagem,
startup ou configuração STM32F103. Os slides foram consultados localmente; não são
duplicados nesta entrega.

O estilo segue os READMEs por exemplo e os roteiros de simulação do
[repositório MIC1](https://github.com/mateusalgado/IFSC_MICROCONTROLADORESI).
