# Inventário comparado com a base do professor

Gerado por `scripts/compare_professor.py`. Não editar manualmente.

Base: `referencias/Game_Start2_STM32F411CE.zip`.
SHA-256: `0130a2994b2002d8e184d69a591b900be8716b7981480139348a8c594157b3cb`.

A comparação de arquivos é byte a byte. Builds, preferências locais e configurações
de lançamento foram excluídos. O JSON inclui hashes, funções, assinaturas, typedefs,
includes e chamadas diretas do Core/Legacy, tanto da base como da versão atual.

O inventário sintático não executa o pré-processador: funções condicionais e exemplos
podem aparecer mesmo sem entrar no firmware. Macros de bibliotecas podem produzir
erros de análise; eles ficam identificados no JSON. Não é uma auditoria formal das bibliotecas.

## Arquivos

- `added`: 16.
- `modified`: 6.
- `moved-identical`: 6.
- `unchanged`: 138.

### Alterados, acrescentados ou movidos

- **modified**: `Core/Inc/FreeRTOSConfig.h`.
- **moved-identical**: `Legacy/Inc/PRNG_LFSR.h` (antes: `Core/Inc/PRNG_LFSR.h`).
- **moved-identical**: `Legacy/Inc/atraso.h` (antes: `Core/Inc/atraso.h`).
- **moved-identical**: `Legacy/Inc/defPrincipais.h` (antes: `Core/Inc/defPrincipais.h`).
- **modified**: `Core/Inc/st7735.h`.
- **moved-identical**: `Legacy/Src/PRNG_LFSR.c` (antes: `Core/Src/PRNG_LFSR.c`).
- **moved-identical**: `Legacy/Src/atraso.c` (antes: `Core/Src/atraso.c`).
- **moved-identical**: `Legacy/Src/figuras.c` (antes: `Core/Src/figuras.c`).
- **modified**: `Core/Src/freertos.c`.
- **modified**: `Core/Src/main.c`.
- **modified**: `Core/Src/st7735.c`.
- **modified**: `Game_Start2_STM32F411CE.ioc`.
- **added**: `Core/Inc/app_diagnostic.h`.
- **added**: `Core/Inc/app_game.h`.
- **added**: `Core/Inc/game_config.h`.
- **added**: `Core/Inc/game_renderer.h`.
- **added**: `Core/Inc/joy_input.h`.
- **added**: `Core/Inc/maze_game.h`.
- **added**: `Core/Inc/maze_map.h`.
- **added**: `Core/Src/app_diagnostic.c`.
- **added**: `Core/Src/app_game.c`.
- **added**: `Core/Src/game_renderer.c`.
- **added**: `Core/Src/joy_input.c`.
- **added**: `Core/Src/maze_game.c`.
- **added**: `Core/Src/maze_map.c`.
- **added**: `Core/Src/maze_mapdata.c`.
- **added**: `Legacy/README.md`.
- **added**: `README.md`.

## Bibliotecas e quantidade de definições

- CMSIS e dispositivo: 1007 definições sintáticas de função.
- Core: aplicacao, suporte e dados: 151 definições sintáticas de função.
- Exemplos preservados: 5 definições sintáticas de função.
- FreeRTOS e adaptador CMSIS-RTOS v2: 341 definições sintáticas de função.
- HAL STM32: 1250 definições sintáticas de função.

## Funções e tipos do Core e dos exemplos

Os números de linha apontam para a versão atual. O arquivo JSON também inventaria
todas as definições reconhecidas nas bibliotecas HAL, CMSIS e FreeRTOS.

### `Core/Inc/figuras.h`

- L7, tipo: `typedef struct { const uint8_t width; uint8_t height; const uint16_t *data; } FigDef;`.

### `Core/Inc/fonts.h`

- L7, tipo: `typedef struct { const uint8_t width; const uint8_t height; const uint16_t *data; } FontDef;`.

### `Core/Inc/joy_input.h`

- L6, tipo: `typedef struct { uint32_t sum_x; uint32_t sum_y; uint16_t count; uint16_t min_x; uint16_t min_y; uint16_t max_x; uint16_t max_y; uint16_t center_x; uint16_t center_y; int32_t filtered_x; int32_t filtered_y; uint8_t button_count; bool calibrated; bool stable_button; bool candidate_button; bool button_armed; } Joy;`.
- L25, tipo: `typedef struct { int16_t x; /* -1000..1000; zero dentro da zona morta. */ int16_t y; bool calibrated; bool click; /* Um evento por pressionamento, depois do debounce. */ } JoyResult;`.

### `Core/Inc/maze_game.h`

- L7, tipo: `typedef enum { GAME_CALIBRATING, GAME_TITLE, GAME_READY, GAME_PLAYING, GAME_PAUSED, GAME_HIT, GAME_RESULT, GAME_WON, GAME_OVER, GAME_FAULT, GAME_HELP } GameState;`.
- L21, tipo: `typedef enum { TUTORIAL_CENTER, TUTORIAL_MOVE, TUTORIAL_WALL, TUTORIAL_IMPACT, TUTORIAL_EXIT, TUTORIAL_PAUSE, TUTORIAL_RESUME, TUTORIAL_DONE } TutorialStep;`.
- L33, tipo: `typedef enum { GAME_NO_ERROR, GAME_WALL, GAME_TIMEOUT, GAME_ADC_ERROR, GAME_DISPLAY_ERROR, GAME_TIMING_ERROR, GAME_QUEUE_ERROR } GameReason;`.
- L44, tipo: `typedef struct { int16_t x; int16_t y; /* -1000..1000 */ bool calibrated; bool valid; bool click; bool render_ready; } GameInput;`.
- L53, tipo: `typedef struct { GameState state; GameReason reason; uint32_t scene; /* Numero da cena; confirmado pela tarefa do display. */ uint32_t state_ms; uint32_t neutral_ms; uint32_t countdown_ms; uint32_t remaining_ms; uint32_t active_ms; uint32_t score; uint32_t map_score; uint32_t tutorial_wait_ms; uint16_t collisions; uint16_t timeouts; int32_t x_milli; /* Posicao em milesimos de pixel, sem float. */ int32_t y_milli; uint8_t lives; uint8_t pause_choice; uint8_t map_index; uint8_t title_choice; bool menu_latched; TutorialStep tutorial_step; } Game;`.

### `Core/Inc/maze_map.h`

- L10, tipo: `typedef struct { const uint8_t *walls; const char *name; uint32_t limit_ms; uint8_t id; uint8_t difficulty; uint8_t corridor_width; uint8_t spawn_x; uint8_t spawn_y; uint8_t goal_x; uint8_t goal_y; } MazeMap;`.

### `Core/Inc/st7735.h`

- L257, tipo: `typedef enum { GAMMA_10 = 0x01, GAMMA_25 = 0x02, GAMMA_22 = 0x04, GAMMA_18 = 0x08 } GammaDef;`.

### `Core/Src/app_diagnostic.c`

- L17: `void Diagnostic_AdcFromISR(uint16_t x, uint16_t y, uint32_t timestamp)`.
- L24: `void Diagnostic_Init(void)`.
- L38: `void Diagnostic_Run(void)`.

### `Core/Src/app_game.c`

- L56: `static void RegisterFault(GameReason reason)`.
- L67: `void App_AdcFromISR(uint16_t x, uint16_t y, uint32_t timestamp)`.
- L79: `static void InputTask(void *argument)`.
- L112: `static GameInput ReadGameInput(const Game *game, uint32_t *now)`.
- L136: `static void CheckGameFaults(Game *game, const GameInput *input)`.
- L153: `static void GameTask(void *argument)`.
- L187: `static void RenderTask(void *argument)`.
- L214: `void App_Start(void)`.
- L24, tipo: `typedef struct { uint16_t x; uint16_t y; uint32_t timestamp; } AdcSample;`.
- L31, tipo: `typedef struct { int16_t x; int16_t y; bool valid; bool calibrated; uint32_t timestamp; } JoystickSample;`.

### `Core/Src/freertos.c`

- L62: `void vApplicationStackOverflowHook(TaskHandle_t task, char *name)`.
- L70: `void vApplicationMallocFailedHook(void)`.

### `Core/Src/game_renderer.c`

- L25: `static uint16_t DifficultyColor(const MazeMap *map)`.
- L37: `static void DrawCenteredTextOn(int y, const char *value, uint16_t color, uint16_t bg, FontDef font)`.
- L45: `static void DrawCenteredText(int y, const char *value, uint16_t color, FontDef font)`.
- L49: `static void DrawOutline(int x, int y, int w, int h, uint16_t color)`.
- L56: `static void DrawIcon(int x, int y, const uint8_t rows[7], uint16_t color)`.
- L69: `static void DrawHeart(int x, uint16_t color)`.
- L74: `static void DrawButton(int y, const char *value, bool selected, uint16_t color)`.
- L80: `static void DrawProgress(int y, unsigned complete, bool current)`.
- L99: `static void DrawPlayer(const Game *game, uint16_t color)`.
- L104: `static uint16_t BackgroundColor(const MazeMap *map, int x, int y)`.
- L125: `static void RestoreRegion(const MazeMap *map, int x, int y, int w, int h)`.
- L141: `static void DrawHud(const Game *game)`.
- L168: `static void DrawCalibration(bool scene_changed)`.
- L179: `static void DrawTitle(const Game *game, bool scene_changed)`.
- L203: `static const char *TutorialHint(const Game *game)`.
- L237: `static unsigned TutorialLesson(TutorialStep step)`.
- L254: `static const char *FaultMessage(GameReason reason)`.
- L269: `static void DrawTutorial(const Game *game, bool scene_changed)`.
- L318: `static void DrawCountdown(const Game *game, const MazeMap *map, bool scene_changed)`.
- L352: `static void DrawPlaying(const Game *game, const MazeMap *map, bool scene_changed)`.
- L370: `static void DrawPause(const Game *game, const MazeMap *map, bool scene_changed)`.
- L382: `static void DrawHit(const Game *game, bool scene_changed)`.
- L395: `static void DrawResult(const Game *game, const MazeMap *map, bool scene_changed)`.
- L427: `static void DrawEnding(const Game *game, const MazeMap *map, bool scene_changed)`.
- L461: `static void DrawFault(const Game *game, bool scene_changed)`.
- L472: `void Renderer_Init(void)`.
- L476: `void Renderer_Draw(const Game *game)`.

### `Core/Src/joy_input.c`

- L16: `static void ResetCalibration(Joy *joy)`.
- L27: `void Joy_Init(Joy *joy)`.
- L33: `static bool UpdateButton(Joy *joy, bool pressed)`.
- L62: `static void UpdateCalibration(Joy *joy, uint16_t x, uint16_t y)`.
- L107: `static int16_t NormalizeAxis(uint16_t raw, uint16_t center, int32_t *filtered)`.
- L140: `JoyResult Joy_Update(Joy *joy, uint16_t x, uint16_t y, bool button, bool valid)`.

### `Core/Src/main.c`

- L80: `static void PublishAdcPair(unsigned offset)`.
- L98: `void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)`.
- L106: `void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)`.
- L119: `int main(void)`.
- L204: `void SystemClock_Config(void)`.
- L251: `static void MX_ADC1_Init(void)`.
- L312: `static void MX_SPI1_Init(void)`.
- L348: `static void MX_DMA_Init(void)`.
- L366: `static void MX_GPIO_Init(void)`.
- L419: `void StartDefaultTask(void *argument)`.
- L447: `void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)`.
- L465: `void Error_Handler(void)`.
- L483: `void assert_failed(uint8_t *file, uint32_t line)`.

### `Core/Src/maze_game.c`

- L8: `static int ToPixel(int32_t milli)`.
- L12: `int Game_X(const Game *game)`.
- L16: `int Game_Y(const Game *game)`.
- L22: `static void EnterState(Game *game, GameState next)`.
- L30: `void Game_Init(Game *game)`.
- L37: `static void PlaceAtStart(Game *game)`.
- L44: `static void PrepareMap(Game *game)`.
- L59: `static void StartCampaign(Game *game)`.
- L68: `void Game_Fail(Game *game, GameReason reason)`.
- L77: `uint32_t Game_Score(const MazeMap *map, uint32_t active_ms, unsigned collisions, unsigned timeouts)`.
- L90: `static void ApplyPenalty(Game *game, GameReason reason)`.
- L109: `static unsigned IntegerSquareRoot(unsigned value)`.
- L133: `static void CalculateDestination(const Game *game, const GameInput *input, uint32_t elapsed_ms, int32_t *next_x, int32_t *next_y)`.
- L148: `static void EnterTutorialStep(Game *game, TutorialStep step)`.
- L153: `static void PlaceInTutorial(Game *game)`.
- L158: `static void UpdateTutorial(Game *game, const GameInput *input, uint32_t elapsed_ms)`.
- L256: `static void UpdateTitle(Game *game, const GameInput *input)`.
- L286: `static void UpdateResult(Game *game, const GameInput *input)`.
- L303: `static void UpdateCountdown(Game *game, const GameInput *input, uint32_t elapsed_ms)`.
- L330: `static void UpdatePlaying(Game *game, const GameInput *input, uint32_t elapsed_ms)`.
- L371: `static void UpdatePause(Game *game, const GameInput *input)`.
- L398: `static void UpdateHit(Game *game)`.
- L413: `void Game_Tick(Game *game, const GameInput *input, uint32_t elapsed_ms)`.

### `Core/Src/maze_map.c`

- L4: `const MazeMap *Maze_Get(unsigned index)`.
- L9: `const char *Maze_Difficulty(const MazeMap *map)`.
- L22: `bool Maze_Wall(const MazeMap *map, int x, int y)`.
- L33: `bool Maze_Free(const MazeMap *map, int x, int y)`.
- L48: `bool Maze_SweepFree(const MazeMap *map, int x0, int y0, int x1, int y1)`.
- L91: `bool Maze_Exit(const MazeMap *map, int x, int y)`.

### `Core/Src/st7735.c`

- L9: `bool ST7735_HasError(void)`.
- L10: `static void ST7735_Transmit(const uint8_t *data, size_t length)`.
- L102: `static void ST7735_Select()`.
- L106: `void ST7735_Unselect()`.
- L110: `static void ST7735_Reset()`.
- L116: `static void ST7735_WriteCommand(uint8_t cmd)`.
- L121: `static void ST7735_WriteData(uint8_t* buff, size_t buff_size)`.
- L126: `static void ST7735_ExecuteCommandList(const uint8_t *addr)`.
- L152: `static void ST7735_SetAddressWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)`.
- L168: `void ST7735_Init()`.
- L178: `void ST7735_DrawPixel(uint16_t x, uint16_t y, uint16_t color)`.
- L191: `void ST7735_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor)`.
- L216: `void ST7735_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor)`.
- L230: `void ST7735_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)`.
- L244: `void ST7735_FillRectangleFast(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)`.
- L248: `void ST7735_FillScreen(uint16_t color)`.
- L252: `void ST7735_FillScreenFast(uint16_t color)`.
- L256: `void ST7735_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data)`.
- L268: `void ST7735_InvertColors(bool invert)`.
- L274: `void ST7735_SetGamma(GammaDef gamma)`.
- L285: `void ST7735_draw_horizontal_line(uint32_t x_init, uint32_t x_end, uint32_t y, uint16_t color)`.
- L292: `void ST7735_draw_vertical_line(uint32_t y_init, uint32_t y_end, uint32_t x, uint16_t color)`.
- L301: `void ST7735_draw_figure(uint32_t x, uint32_t y, FigDef fig, uint16_t color)`.
- L335: `void ST7735_write_nr(uint32_t x, uint32_t y, int nr, FontDef font, uint16_t color, uint16_t bgcolor)`.
- Atenção: há construções não resolvidas pelo parser; consultar o JSON e o fonte.

### `Core/Src/stm32f4xx_hal_msp.c`

- L63: `void HAL_MspInit(void)`.
- L88: `void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)`.
- L142: `void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)`.
- L173: `void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)`.
- L210: `void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)`.

### `Core/Src/stm32f4xx_hal_timebase_tim.c`

- L41: `HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)`.
- L121: `void HAL_SuspendTick(void)`.
- L133: `void HAL_ResumeTick(void)`.

### `Core/Src/stm32f4xx_it.c`

- L71: `void NMI_Handler(void)`.
- L86: `void HardFault_Handler(void)`.
- L101: `void MemManage_Handler(void)`.
- L116: `void BusFault_Handler(void)`.
- L131: `void UsageFault_Handler(void)`.
- L146: `void DebugMon_Handler(void)`.
- L166: `void TIM4_IRQHandler(void)`.
- L180: `void DMA2_Stream0_IRQHandler(void)`.

### `Core/Src/syscalls.c`

- L44: `void initialise_monitor_handles()`.
- L48: `int _getpid(void)`.
- L53: `int _kill(int pid, int sig)`.
- L61: `void _exit (int status)`.
- L67: `__attribute__((weak)) int _read(int file, char *ptr, int len)`.
- L80: `__attribute__((weak)) int _write(int file, char *ptr, int len)`.
- L92: `int _close(int file)`.
- L99: `int _fstat(int file, struct stat *st)`.
- L106: `int _isatty(int file)`.
- L112: `int _lseek(int file, int ptr, int dir)`.
- L120: `int _open(char *path, int flags, ...)`.
- L128: `int _wait(int *status)`.
- L135: `int _unlink(char *name)`.
- L142: `int _times(struct tms *buf)`.
- L148: `int _stat(char *file, struct stat *st)`.
- L155: `int _link(char *old, char *new)`.
- L163: `int _fork(void)`.
- L169: `int _execve(char *name, char **argv, char **env)`.

### `Core/Src/sysmem.c`

- L53: `void *_sbrk(ptrdiff_t incr)`.

### `Core/Src/system_stm32f4xx.c`

- L167: `void SystemInit(void)`.
- L220: `void SystemCoreClockUpdate(void)`.
- L279: `void SystemInit_ExtMemCtl(void)`.
- L445: `void SystemInit_ExtMemCtl(void)`.
- Atenção: há construções não resolvidas pelo parser; consultar o JSON e o fonte.

### `Legacy/Src/PRNG_LFSR.c`

- L9: `void init_LFSR(uint32_t valor)`.
- L18: `uint32_t prng_LFSR() // Galois LFSRs, Liner-Feedback Shift Register, (PRNG)`.

### `Legacy/Src/atraso.c`

- L19: `void atraso_us(uint32_t valor) //argumento passado pelo registrador r0`.
- L29: `void atraso_ms(uint32_t valor) //argumento passado pelo registrador r0`.
- L40: `void atraso_loops(uint32_t valor) //argumento passado pelo registrador r0`.
