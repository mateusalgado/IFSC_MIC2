<img src="assets/ifsc-logo.png" alt="IFSC - Câmpus Florianópolis" width="30%" style="padding: 10px">

# Entrega — Labirinto de Precisão

[Sumário do projeto](../README.md) · [Relatório consolidado](RELATORIO.md)

Revisão documental de 24/11/2026, firmware **TUTORIAL v0.4**. Este é o índice para apresentação e
conferência do trabalho. A campanha foi concluída na placa pelo aluno, sem
problemas relatados até o momento. O tutorial novo ainda precisa desse reteste.

## Material organizado no repositório

- [README do projeto](../README.md): capa com identidade IFSC, identificação,
  descrição, imagem e sumário, seguindo o projeto de PROGOO.
- Páginas numeradas com Retroceder/Sumário/Avançar:
  [1. Análise](../analise.md), [2. Projeto](../projeto.md),
  [3. Implementação](../implementacao.md) e [4. Testes](../testes.md).
- [Relatório técnico](RELATORIO.md): requisitos, hardware, RTOS, estados, regras,
  memória, implementação, resultados e referências.
- [Hardware](HARDWARE.md): pinagem e alimentação da montagem.
- [Campanha](CAMPANHA.md): nove mapas, controles, pontuação e roteiro de bancada.
- [Regeneração](REGENERACAO.md): contrato `.ioc`/fontes e procedimento CubeMX.
- [Código completo](../Game_Start2_STM32F411CE), [testes](../tests) e [scripts](../scripts).
- [Enunciado original](../referencias/Projeto_uC_II_2026_02.pdf) e
  [origem dos materiais](../referencias/ORIGEM.md).
- [Verificações e binários no GitHub Actions](https://github.com/mateusalgado/IFSC_MIC2/actions/workflows/projeto-jogo.yml).

## Evidências disponíveis

- Campanha e tutorial com testes automatizados de lógica, desenho e driver SPI.
- Compilações ARM do jogo e do diagnóstico; ensaio de regeneração documentado.
- Relato do aluno em 03/11/2026: concluiu os nove mapas da versão anterior ao
  tutorial, sem problemas até o momento. Duração e commit gravado não informados.
- Imagens da interface produzidas no computador, identificadas como simulação.
- [Foto dos componentes](assets/componentes-aluno.png) fornecida pelo aluno;
  ainda falta registrar a montagem final com o jogo em execução.

## Fechamento da entrega

### Conferência após a organização de 24/11/2026

O projeto foi separado em `MIC2/projeto-jogo`, sem alterações no firmware.
No novo caminho foram executados com sucesso:

- `check_config.py`: contrato entre CubeMX e fontes.
- `generate_map.py --check`: geometria e catálogo dos nove mapas.
- `test_host.py --sanitize`, com GCC no Ubuntu/WSL: 8.348 verificações de lógica,
  além das suítes de renderização e do driver ST7735; sem falhas reportadas.
- `build_base.py` e `build_base.py --diagnostic`, com GNU Arm do STM32CubeIDE:
  48 objetos ligados em cada variante, gerando ELF, HEX e BIN.

O firmware para a campanha está em `build/campanha/labirinto-campanha.hex`.
Essa conferência foi feita no computador. Não houve gravação, novo ensaio na
placa ou medição física nesta data. As pendências abaixo continuam abertas;
o aluno decidiu retirar áudio do escopo em 24/11/2026. O enunciado original
prevê sons, e a concordância docente com essa exceção ainda precisa ser registrada.

### Lista de conclusão

- [x] Nove mapas, três dificuldades, pontuação, vidas, pausa, derrota e reinício.
- [x] Campanha concluída na placa, conforme relato do aluno.
- [x] Tutorial interativo implementado e testado no computador.
- [x] `.ioc` alinhado, relatório técnico e instruções de reprodução preparados.
- [ ] Retestar TUTORIAL v0.4 na placa e registrar commit, resultado e duração.
- [x] Escopo da entrega definido pelo aluno: sem buzzer e sem áudio (24/11/2026).
- [ ] Registrar concordância do professor com a exceção ao requisito de sons.
- [ ] Registrar aprovação da ideia pelo professor e identificação acadêmica completa.
- [ ] Acrescentar fotografia legível da montagem/MCU e vídeo da demonstração.
- [ ] Preencher medições de calibração, duração das tarefas, margens de pilha/heap
  e ensaio contínuo descritos no relatório.

Áudio não é uma tarefa de implementação desta entrega. A documentação registra
a diferença em relação ao enunciado, sem presumir aprovação docente.
O [ensaio final](ENSAIO_FINAL.md) detalha o que conectar e o que observar na placa.

## Roteiro sugerido de apresentação

1. Mostrar placa, alimentação, pinagem, periféricos e versão no título.
2. Executar COMO JOGAR: alvo, colisão, desvio e pausa/retomada.
3. Iniciar campanha; demonstrar movimento proporcional e perda de uma vida.
4. Mostrar pausa, avanço de fase e mudanças fácil → médio → difícil.
5. Mostrar vitória após M09, derrota e reinício desde M01.
6. Explicar as três tarefas, filas, callbacks curtos e dono único do display.
7. Mostrar testes, resultado da compilação e medições reais registradas.
8. Informar que a entrega não inclui áudio e registrar a posição do professor.

## Registro do ensaio final

Preencher somente após o teste real:

- Data, responsável, versão e commit: pendente.
- Placa/MCU e alimentação: pendente. Buzzer: fora do escopo.
- Tutorial: completar, repetir após erro e sair para o menu — pendente.
- Campanha desta revisão: nove mapas, mudanças de dificuldade, pausa e reinício — pendente.
- Duração, problemas encontrados, correções e novo ensaio — pendente.
- Valores de pilha/heap/atrasos e calibração — pendente.
- Links para fotos/vídeo — pendente.

Atualizar também a seção de resultados do [relatório](RELATORIO.md), mantendo
o relato funcional separado das medidas e evidências capturadas.
