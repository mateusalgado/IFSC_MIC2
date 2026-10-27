# Microcontroladores II — IFSC

Exemplos e projetos da disciplina, com código, instruções de uso e documentação
próximos de cada aplicação, seguindo a organização dos trabalhos de
[Microcontroladores I](https://github.com/mateusalgado/IFSC_MICROCONTROLADORESI).

## Projetos

- [Laboratório FreeRTOS no STM32F411](README-labRTOS.md): 16 exemplos de tarefas,
  prioridades, comunicação, sincronização e interrupções, com USB CDC.
- [Mini elevador didático](elevador/README.md): base modular em C, máquina de
  estados, três tarefas FreeRTOS, testes no computador e preparação da montagem.
  [Relatório técnico](elevador/docs/RELATORIO.md) ·
  [Validação](elevador/docs/VALIDACAO.md) ·
  [Montagem e ensaios](elevador/docs/BANCADA.md).

## Usar o repositório

```sh
git clone https://github.com/mateusalgado/IFSC_MIC2.git
cd IFSC_MIC2/elevador
make test
make sanitize
```

Os testes da lógica usam GCC e Make em Linux/WSL. Para compilar o firmware ARM,
seguir [a integração STM32CubeIDE](elevador/docs/INTEGRACAO.md).
Cada exemplo deve ter um único `main` selecionado na compilação.

O mini elevador está na etapa de software: componentes, estrutura, drivers
físicos e validação em bancada ainda precisam ser concluídos. O port entregue
é inativo e não deve ser interpretado como firmware pronto para movimentar uma cabine.
