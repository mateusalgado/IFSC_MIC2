<img src="docs/assets/ifsc-logo.png" alt="IFSC - Câmpus Florianópolis" width="30%" style="padding: 10px">

# 4. Testes

> Estratégia, casos verificados, resultados obtidos e pendências de bancada.
>
> Página **4 de 4** · [Sumário](README.md)

---

## Estratégia de testes

1. **Testes de host:** regras, entrada, geometria, renderização e SPI simulados.
2. **Compilação ARM:** jogo e diagnóstico, incluindo cópia regenerada pelo CubeMX.
3. **Ensaio físico:** execução pelo aluno na placa e registro de comportamento.
4. **Medições:** calibração, latência, pilhas, heap e duração, ainda a registrar.

Testes simulados não medem os tempos reais do escalonador nem verificam as
ligações elétricas. Uma partida concluída não substitui todos os casos de bancada.

## Resultados automatizados

Firmware de referência: **TUTORIAL v0.4**, commit
[`8283d69`](https://github.com/mateusalgado/IFSC_MIC2/commit/8283d694e7bbd37f82338290b85a7e4accaa9edf),
03/11/2026. A [execução do GitHub Actions](https://github.com/mateusalgado/IFSC_MIC2/actions/runs/41644819836)
terminou com sucesso nos jobs de regras/interface e compilação ARM.

- **8.348 verificações** de lógica, entrada e geometria aprovadas. Esse número
  representa as verificações executadas, não 8.348 casos independentes.
- Testes do renderizador aprovados, incluindo limites de texto e equivalência
  entre atualização parcial e quadro completo.
- Driver ST7735 aprovado com SPI simulado, incluindo clipping e timeout.
- Sanitizadores AddressSanitizer/UndefinedBehaviorSanitizer sem falhas observadas.
- Contrato `.ioc`/fontes aprovado; geometrias geradas coerentes com os JSONs.

### Casos de teste cobertos

- **TC01 — Geometria:** busca independente de caminho para o ponto 5×5 nos nove
  mapas, limites e percursos; aprovado no computador.
- **TC02 — Campanha:** percorrer M01→M09 pela lógica real, preservar vidas/pontos,
  confirmar resultados, vitória e reinício; aprovado no computador.
- **TC03 — Penalidades:** colisões, timeout 60/50/40 s, derrota em fase posterior
  e reinício; aprovado no computador.
- **TC04 — Controles:** calibração, zona morta, debounce, menus e pausa; aprovado
  no computador.
- **TC05 — Tutorial:** completar alvo/colisão/desvio/pausa, repetir após erro e
  preservar vidas/pontos; aprovado no computador.
- **TC06 — Desenho:** estados visuais, restauração dos mapas, HUD e tutorial;
  aprovado no display simulado.
- **TC07 — Driver:** bordas, regiões vazias, ordem de bytes e falhas SPI com
  espera limitada; aprovado com HAL simulado.
- **TC08 — Configuração:** HSI/PLL, ranks ADC, DMA, prioridades e hooks;
  aprovado pelo verificador e pelos builds da cópia regenerada.

Fontes: [test_game.c](tests/test_game.c), [test_renderer.c](tests/test_renderer.c),
[test_driver.c](tests/test_driver.c) e [check_config.py](scripts/check_config.py).

## Compilação e regeneração

Compilação local com GNU Arm GCC 14.3.1, 48 objetos, `-O0 -Wall`, sem avisos:

- Jogo salvo: text 76360, data 120, bss 24272 bytes.
- Diagnóstico salvo: text 32292, data 104, bss 21616 bytes.
- Jogo regenerado: text 76328, data 120, bss 24272 bytes.
- Diagnóstico regenerado: text 32276, data 104, bss 21616 bytes.

A cópia foi gerada com CubeMX 6.18/F4 v1.28.3, seguida da correção documentada do
protótipo de hook. Os módulos próprios e as regiões USER CODE foram preservados.
[Procedimento e registro](docs/REGENERACAO.md). BSS não representa o uso máximo
de RAM durante uma partida.

---

## Testes funcionais na placa

**Relato do aluno em 03/11/2026:** concluiu os nove mapas da versão anterior ao
tutorial, sem problemas até o momento. O commit gravado e a duração não foram
informados. Também há confirmação anterior de funcionamento do diagnóstico.

**Ainda pendente nesta revisão:**

- Completar/sair/reentrar no tutorial TUTORIAL v0.4 na placa.
- Retestar a campanha após gravar esta versão e a cópia regenerada, se utilizada.
- Registrar duração, centro/extremos do joystick, identificação da placa e alimentação.
- Medir `game_input_stack_free`, `game_logic_stack_free`,
  `game_render_stack_free`, `game_min_free_heap` e `game_late_ticks`.
- Registrar fotos/vídeo da montagem final e ensaio contínuo.
- Implementar e testar áudio após escolher o buzzer e o circuito.

## Relação com os conteúdos da disciplina

- **Máquina de estados:** transições, derrota/vitória, pausa e tutorial.
- **Interrupções e DMA:** publicação de amostras coerentes sem desenho na ISR.
- **FreeRTOS:** tarefas periódicas, prioridades, filas e proprietário único do SPI.
- **Recursos limitados:** máscaras na Flash e buffer gráfico parcial.
- **Verificação:** regras independentes da HAL, testes de host e diagnóstico físico.

## Registro para entrega

O [relatório consolidado](docs/RELATORIO.md) reúne fundamentação e evidências.
O [roteiro de entrega](docs/ENTREGA.md) concentra pendências, demonstração e
campos para as medições reais. Áudio e aprovação docente continuam abertos;
a documentação não declara esses requisitos concluídos.

---

<div align="center">

[Retroceder](implementacao.md) | [Sumário](README.md) | **4 / 4** | [Início](README.md)

</div>
