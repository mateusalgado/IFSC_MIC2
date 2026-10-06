# Laboratório FreeRTOS no STM32F411CE

O projeto `ex_Barry_USB_STM32F411CE` é a base para executar os 16 exemplos do
arquivo `exemplos_FreeRTOS.zip` no STM32F411CEU6. O clock, o GPIO e a USB CDC
continuam configurados pelo projeto Barry. A lógica dos exercícios está em
`Core/Src/lab_examples.c`; somente um exemplo é compilado por vez.

## Como escolher e testar

1. Abra `ex_Barry_USB_STM32F411CE` como projeto existente no STM32CubeIDE.
2. Em `Core/Inc/lab_examples.h`, altere `#define LAB_EXAMPLE 1` para um número
   entre 1 e 16.
3. Compile a configuração **Debug** e grave pelo ST-LINK em modo **SWD**.
4. Mantenha a USB de dados da placa conectada ao computador e abra sua porta
   serial virtual para ver as mensagens. Após cada gravação, reinicie a placa.

No Linux, baixe o repositório com:

```bash
git clone https://github.com/mateusalgado/IFSC_MIC2.git
cd IFSC_MIC2
```

Depois, importe a pasta
`ex_Barry_USB_STM32F411CE` no STM32CubeIDE e grave o firmware pelo ST-LINK.
Para encontrar e ler a porta USB CDC da placa:

```bash
ls -l /dev/serial/by-id/ 2>/dev/null
ls /dev/ttyACM* 2>/dev/null
cat /dev/ttyACM0
```

Se a placa aparecer com outro número, substitua `/dev/ttyACM0` pelo dispositivo
encontrado. Abra o `cat` e pressione **RESET** na placa; as mensagens são
periódicas e devem aparecer continuamente. Se houver erro de permissão,
adicione seu usuário ao grupo `dialout` e entre novamente na sessão:

```bash
sudo usermod -aG dialout "$USER"
```

O projeto espera uma placa com **STM32F411CEU6**, cristal externo de **25 MHz**
e USB de dados ligada ao microcontrolador. Se a placa conectada ao Linux for
uma STM32F103, este projeto precisa de outro alvo de compilação.

O ST-LINK é usado para gravar e depurar; as mensagens `printf` saem pela USB
CDC da própria placa. Os exemplos 12 a 14 disparam a EXTI0 por software e não
precisam de botão externo. O exemplo 7 usa o *idle hook* e o 16 usa o *tick
hook*. O exemplo 3 pode deixar a tarefa de menor prioridade sem tempo de CPU:
esse é o efeito de uma tarefa prioritária com espera ocupada.

## Adaptação feita

- Os códigos originais foram escritos para STM32F103C8T6; este projeto compila
  para STM32F411CEU6 e usa a inicialização de hardware do projeto Barry.
- `main.c` inicia a USB dentro da tarefa padrão, cria as tarefas do exemplo
  escolhido e encerra a tarefa padrão.
- `FreeRTOSConfig.h` habilita `vTaskDelayUntil`, semáforos de contagem e os hooks
  de idle e tick usados pelos exercícios.
- O `main1_1_4_ex1.c` que já existia no projeto Barry foi mantido como
  referência, mas excluído da compilação para haver apenas um `main`.
- O envio USB aguarda a conclusão de cada transmissão antes de reutilizar o
  buffer; se a porta não estiver conectada, descarta a mensagem sem travar a
  tarefa.

Se regenerar o projeto a partir do arquivo `.ioc`, confira novamente as opções
do FreeRTOS, a exclusão do `main1_1_4_ex1.c` e os blocos `USER CODE` de
`main.c`.

Até aqui, os 16 caminhos foram verificados pelo compilador e os exemplos 1,
13, 14 e 16 foram ligados com sucesso. A execução física e a saída USB ainda
precisam ser verificadas na placa.
