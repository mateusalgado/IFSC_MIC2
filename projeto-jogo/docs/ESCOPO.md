# Escopo — Labirinto de Precisão

Atualizado em 27/10/2026. O usuário escolheu um jogo com diferentes plantas de
labirinto, conduzindo um ponto pelo caminho sem tocar nas paredes. Também
confirmou que deseja uma interface bonita e interativa, não inteligência artificial.

## Requisitos da disciplina

- Projeto individual em ARM com RTOS e ideia aprovada pelo professor.
- Display matricial, joystick e sons.
- Tela inicial e final de game over, com reinício através do joystick.
- Pelo menos três níveis de dificuldade.

A avaliação considera funcionamento, originalidade, criatividade, jogabilidade,
imagens, montagem, qualidade e domínio do código/hardware e desenvolvimento em
sala. Fonte: [enunciado](../referencias/Projeto_uC_II_2026_02.pdf).

## Proposta de entrega

Campanha de nove mapas, três por dificuldade, três vidas e pontuação por mapa
concluído. Paredes mais próximas e percursos mais complexos aumentam a dificuldade;
o controle mantém resposta previsível. O nome Labirinto de Precisão é provisório.

As regras exatas, parâmetros iniciais, arquitetura e critérios de teste estão em
[PLANO_LABIRINTO.md](PLANO_LABIRINTO.md). Telas, inventário de arte e fontes estão
em [PLANO_VISUAL.md](PLANO_VISUAL.md). Esses documentos são a proposta v1; ainda
não representam jogo implementado ou aprovação do professor.

## Estado atual

- [x] Materiais do professor organizados e origem preservada.
- [x] Pinagem definida: display SDA A7, SCK A5, CS B12, RESET B13, DC B14;
  joystick VRY A0, VRX A1, SW C15; alimentação dos módulos em 3V3.
- [x] Diagnóstico compilado e publicado no GitHub.
- [x] Gravação e funcionamento do diagnóstico relatados pelo aluno.
- [x] Conceito do jogo escolhido e plano v1 de regras/RTOS/interface elaborado.
- [ ] Aprovação da ideia pelo professor.
- [ ] Valores medidos de calibração, identificação do MCU e ensaio prolongado.
- [x] Primeiro mapa, FSM, três tarefas e comunicação implementados; ensaio físico pendente.
- [x] Vidas, tempo, pontuação, pausa, derrota e reinício implementados/testados no computador para o mapa 01.
- [ ] Três dificuldades jogáveis e nove mapas validados.
- [ ] Buzzer escolhido, circuito dimensionado e áudio funcionando.
- [ ] Arte final, testes, medições e relatório concluídos.

O firmware padrão desta revisão é o mapa 01; o diagnóstico é uma opção de build.
Ver [PRIMEIRO_MAPA.md](PRIMEIRO_MAPA.md). O hardware
segue com tela, joystick e som, sem motor. A ideia do elevador permanece em MAQ.

## Próxima entrega concreta

Gravar M01 v0.1 e registrar o comportamento real: centro, orientação, parada,
colisões, pausa, vidas e saída. Ajustar a jogabilidade e depois expandir para
três dificuldades. A reconciliação/regeneração do `.ioc`, medições de RTOS,
áudio e demais mapas permanecem pendentes. A sequência M0–M6 está no plano.

Não marcar testes físicos, medições ou aprovação docente como realizados sem
evidência. O relato funcional do aluno e os resultados medidos têm registros separados.
