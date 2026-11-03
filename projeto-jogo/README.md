<img src="docs/assets/ifsc-logo.png" alt="Instituto Federal de Santa Catarina - Câmpus Florianópolis" width="30%" style="padding: 10px">

# Labirinto de Precisão

## Jogo de labirinto em C para STM32F411 com FreeRTOS

> Repositório destinado ao desenvolvimento do projeto da disciplina de Microcontroladores II.
>
> Aluno: Mateus Salgado Barboza Costa<br>
> Instituição: Instituto Federal de Santa Catarina<br>
> Professor: identificação a confirmar para a entrega.
>
> Firmware: **TUTORIAL v0.4** · Documentação atualizada em **03/11/2026**.

---

### Descrição

O jogador conduz um ponto por nove labirintos sem tocar nas paredes, com três
dificuldades, três vidas compartilhadas, tempo, pontuação acumulada, pausa e
vitória final. O menu **COMO JOGAR** oferece um tutorial interativo de movimento,
colisão, desvio e pausa, sem penalizar a campanha.

**Tecnologias utilizadas:**

- Linguagem: C, HAL e CMSIS.
- Microcontrolador: STM32F411CEU6, HSI/PLL a 96 MHz.
- Sistema operacional: FreeRTOS, com tarefas de entrada, jogo e desenho.
- Interface: TFT ST7735 SPI 160×128 e joystick analógico com botão.
- Ferramentas: STM32CubeIDE/CubeMX, GNU Arm Embedded e testes de host com sanitizadores.

<p align="center">
  <img src="docs/assets/interface-renderizada.png" alt="Telas do Labirinto de Precisão produzidas pelo renderizador C com display simulado" width="90%">
  <br>
  <em>Prévia da interface produzida no computador; não é fotografia da placa.</em>
</p>

---

<p align="center"><strong>SUMÁRIO</strong></p>

[**1. ANÁLISE DO SISTEMA**](analise.md)<br>
[**2. PROJETO DO SISTEMA**](projeto.md)<br>
[**3. IMPLEMENTAÇÃO (C / FREERTOS)**](implementacao.md)<br>
[**4. TESTES**](testes.md)<br>

As quatro páginas possuem navegação **Retroceder · Sumário · Avançar**.
A estrutura segue o [projeto de PROGOO](https://github.com/mateusalgado/IFSC_PROGOO_PROJECT),
adaptada ao sistema embarcado em C e à arquitetura FreeRTOS.

---

### Acesso rápido

- [Compilar, gravar e testar a versão atual](docs/CAMPANHA.md).
- [Conexões de hardware](docs/HARDWARE.md).
- [Relatório técnico consolidado](docs/RELATORIO.md).
- [Roteiro de entrega e apresentação](docs/ENTREGA.md).
- [Configuração e regeneração CubeMX](docs/REGENERACAO.md).
- [Enunciado da disciplina](referencias/Projeto_uC_II_2026_02.pdf).
- [Materiais, créditos e identidade visual](referencias/ORIGEM.md).

### Situação da entrega

O aluno confirmou a conclusão dos nove mapas da versão anterior ao tutorial,
sem problemas até o momento. O tutorial v0.4 passou nos testes do computador e
aguarda teste físico. Áudio, identificação docente e registros de bancada
continuam pendentes, conforme o [roteiro de entrega](docs/ENTREGA.md).

---

<div align="center">

[Iniciar leitura: 1. Análise do sistema →](analise.md)

</div>
