# Entrega — Labirinto de Precisão

Revisão de 03/11/2026, **TUTORIAL v0.4**. Este é o índice para apresentação e
conferência do trabalho. A campanha foi concluída na placa pelo aluno, sem
problemas relatados até o momento. O tutorial novo ainda precisa desse reteste.

## Material organizado no repositório

- [README do projeto](../README.md): apresentação, imagem, conexões, código e uso,
  seguindo a organização dos exemplos do repositório de MIC1.
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

- [x] Nove mapas, três dificuldades, pontuação, vidas, pausa, derrota e reinício.
- [x] Campanha concluída na placa, conforme relato do aluno.
- [x] Tutorial interativo implementado e testado no computador.
- [x] `.ioc` alinhado, relatório técnico e instruções de reprodução preparados.
- [ ] Retestar TUTORIAL v0.4 na placa e registrar commit, resultado e duração.
- [ ] Escolher/montar buzzer e implementar/verificar sons exigidos no enunciado.
- [ ] Registrar aprovação da ideia pelo professor e identificação acadêmica completa.
- [ ] Acrescentar fotografia legível da montagem/MCU e vídeo da demonstração.
- [ ] Preencher medições de calibração, duração das tarefas, margens de pilha/heap
  e ensaio contínuo descritos no relatório.

O requisito de sons continua aberto. Portanto, esta revisão deixa a documentação
organizada e o software disponível para teste, mas não declara o atendimento
integral do enunciado antes de concluir as pendências acima.

## Roteiro sugerido de apresentação

1. Mostrar placa, alimentação, pinagem, periféricos e versão no título.
2. Executar COMO JOGAR: alvo, colisão, desvio e pausa/retomada.
3. Iniciar campanha; demonstrar movimento proporcional e perda de uma vida.
4. Mostrar pausa, avanço de fase e mudanças fácil → médio → difícil.
5. Mostrar vitória após M09, derrota e reinício desde M01.
6. Explicar as três tarefas, filas, callbacks curtos e dono único do display.
7. Mostrar testes, resultado da compilação e medições reais registradas.
8. Demonstrar o áudio quando implementado; até lá, declarar a pendência.

## Registro do ensaio final

Preencher somente após o teste real:

- Data, responsável, versão e commit: pendente.
- Placa/MCU, alimentação e buzzer: pendente.
- Tutorial: completar, repetir após erro e sair para o menu — pendente.
- Campanha desta revisão: nove mapas, mudanças de dificuldade, pausa e reinício — pendente.
- Duração, problemas encontrados, correções e novo ensaio — pendente.
- Valores de pilha/heap/atrasos e calibração — pendente.
- Links para fotos/vídeo — pendente.

Atualizar também a seção de resultados do [relatório](RELATORIO.md), mantendo
o relato funcional separado das medidas e evidências capturadas.
