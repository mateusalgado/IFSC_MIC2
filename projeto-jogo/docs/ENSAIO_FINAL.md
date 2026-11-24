# Ensaio final — TUTORIAL v0.4 sem buzzer

Preparado em 24/11/2026. Nenhum resultado físico abaixo foi preenchido.
Na consulta desta data, no computador de preparação, o STM32CubeProgrammer
v2.23.0 não detectou ST-LINK. Nesse computador, as ferramentas de gravação,
GDB e servidor ST-LINK estão instaladas. O aluno fará o teste em outro computador.

## Teste em outro computador

No computador com a placa, instalar ou usar o STM32CubeIDE e o driver do ST-LINK.
Para obter o projeto pela primeira vez:

```sh
git clone https://github.com/mateusalgado/IFSC_MIC2.git
cd IFSC_MIC2
```

Se o clone já existe, abrir um terminal na raiz dele e executar `git pull --ff-only`.
Se houver alterações locais, preservá-las antes de resolver qualquer conflito;
não usar reset para descartar trabalho.

1. No STM32CubeIDE, importar como projeto existente a pasta
   `projeto-jogo/Game_Start2_STM32F411CE` (contém `.project` e `.cproject`).
2. Manter `APP_DIAGNOSTIC=0` em `Core/Inc/game_config.h`.
3. Executar Refresh, Clean e Build na configuração Debug. Não é necessário
   regenerar o `.ioc` nem instalar Python para compilar pelo CubeIDE.
4. Conectar o ST-LINK à placa e usar Run/Debug com o ELF recém-compilado.
5. Deixar o joystick centralizado ao reiniciar. A tela deve mostrar **TUTORIAL v0.4**.
6. Seguir a sequência de validação abaixo e anotar o resultado de `git rev-parse HEAD`.

Os arquivos `build/`, `Debug/` e `Release/` locais não são enviados ao Git.
Cada computador gera seus próprios binários. A alternativa por Python/GNU Arm
e gravação de HEX está em [implementação](../implementacao.md#como-compilar-e-gravar).
O ZIP em `referencias/` é a base original do professor, não o jogo atualizado.

Após o teste, informar commit, resultado do tutorial/campanha, duração e qualquer
erro observado. Fotos/vídeo e medições podem ser acrescentados depois; a execução
em outro computador não será tratada como observada diretamente pelo assistente.

### Aviso "Project has no explicit encoding set"

Esse aviso indica que o projeto herda a codificação do workspace. A preferência
`Game_Start2_STM32F411CE/.settings/org.eclipse.core.resources.prefs` agora é
versionada e define UTF-8 explicitamente. As demais preferências locais do IDE
continuam ignoradas pelo Git.

Depois de atualizar o clone, execute Refresh no projeto. Se o aviso persistir,
clique com o botão direito no projeto citado e abra **Properties → Resource →
Text file encoding → Other: UTF-8 → Apply and Close**. Esse procedimento está
descrito na [documentação do Eclipse CDT](https://help.eclipse.org/latest/topic/org.eclipse.cdt.doc.user/reference/cdt_u_prop_resource.htm).

Se o aviso citar `projeto-jogo`, confira **Properties → Resource → Location**:
o projeto de firmware a importar é a subpasta `projeto-jogo/Game_Start2_STM32F411CE`,
que contém `.project`, `.cproject` e `Core`. A pasta superior reúne também mapas,
documentação e testes. Um nome diferente pode ser apenas uma renomeação no IDE;
confira o caminho antes de importar outro projeto.

Se o projeto foi importado com **Copy projects into workspace**, ele é uma
cópia separada do clone: `git pull` no clone não atualiza essa cópia. Nesse caso,
aplique a preferência nela também e confira se está compilando o código atualizado.

## O que precisa estar conectado

- Placa STM32 com display e joystick montados conforme [HARDWARE.md](HARDWARE.md).
- ST-LINK conectado ao computador por USB e à placa por SWD: SWDIO/PA13,
  SWCLK/PA14 e GND comum. Identificar o modelo e a pinagem do programador antes
  de ligar alimentação/Vref; não presumir que todos os ST-LINK têm o mesmo conector.
- Placa e módulos alimentados conforme a montagem validada, com uma única
  fonte de 3,3 V. A USB da placa, sozinha, não substitui o ST-LINK para depuração SWD.
- Buzzer e microSD desconectados. Não mudar fios com a montagem alimentada.

## Participação durante a sessão

O assistente pode identificar o programador/MCU, gravar e verificar o firmware,
ler variáveis de diagnóstico e organizar resultados quando houver acesso SWD.
O aluno precisa observar a tela, movimentar o joystick, apertar o botão e
relatar o comportamento. ST-LINK não fornece uma imagem da tela física.
Fotos e vídeo da montagem devem ser capturados pelo aluno.

Antes de gravar, registrar o alvo detectado e confirmar que corresponde ao
STM32F411 do projeto. Registrar também commit, alterações locais e SHA-256 do
HEX usado, pois o commit sozinho não identifica uma árvore com mudanças locais.
Firmware preparado: `../build/campanha/labirinto-campanha.hex`.

## Sequência de validação

1. Identificar a marcação do MCU, o modelo de ST-LINK e a fonte de alimentação.
2. Conectar por SWD, conferir o alvo, gravar o HEX com verificação e reiniciar.
3. Deixar o joystick centralizado na calibração. Conferir direção dos eixos,
   ausência de movimento sozinho, seleção do menu e clique único do botão.
4. Completar COMO JOGAR: alvo, colisão, desvio, pausa/retomada. Sair e reentrar.
5. Jogar M01–M09, conferir três dificuldades, progressão, vidas e pontuação.
6. Conferir pausa sem perda de posição/tempo, timeout, derrota e reinício em M01.
7. Manter uma sessão contínua de 30 minutos, registrando reinícios, travamentos,
   imagem corrompida ou falhas de entrada, mesmo se não ocorrer nenhum problema.
8. Ao fim, ler margens de pilha, heap mínimo e contador de atrasos por depuração.
   Coletas que pausarem a CPU devem ficar fora do intervalo do ensaio contínuo;
   pausas do debugger podem distorcer contagens de tempo e comunicação.
9. Registrar foto da montagem e vídeo do tutorial e dos estados principais.

Calibração bruta pode ser registrada com o diagnóstico opcional. Se ele for
gravado, restaurar a campanha e reiniciar antes do ensaio final.

## Registro a preencher com valores reais

- Data, aluno, MCU, ST-LINK, alimentação: pendentes.
- Commit, alterações locais, arquivo e SHA-256 gravado: pendentes.
- Joystick X/Y: centro, mínimo, máximo e sentido observado: pendentes.
- Tutorial, campanha, pausa, timeout, derrota e reinício: pendentes.
- Início/fim do ensaio contínuo e ocorrências: pendentes.
- `game_input_stack_free`: pendente, em palavras de 32 bits (×4 para bytes).
- `game_logic_stack_free`: pendente, em palavras de 32 bits.
- `game_render_stack_free`: pendente, em palavras de 32 bits.
- `game_min_free_heap`: pendente, em bytes.
- `game_late_ticks`: pendente; registrar se houve pausas do debugger.
- Duração das tarefas/ISR e latência: pendentes; as variáveis acima não medem
  esses tempos. Definir instrumentação separada antes de apresentar números.
- Fotos/vídeo: pendentes.
- Nome do professor, aprovação da ideia e posição sobre entrega sem som: pendentes.

Após o ensaio, preencher [ENTREGA.md](ENTREGA.md) e [RELATORIO.md](RELATORIO.md).
Não substituir medições por períodos configurados nem por resultados dos testes de host.
