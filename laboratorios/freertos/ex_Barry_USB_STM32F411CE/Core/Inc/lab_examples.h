#ifndef LAB_EXAMPLES_H
#define LAB_EXAMPLES_H

/* Escolha um exemplo de 1 a 16 e recompile o projeto no STM32CubeIDE. */
#ifndef LAB_EXAMPLE
#define LAB_EXAMPLE 1
#endif

#if LAB_EXAMPLE < 1 || LAB_EXAMPLE > 16
#error "LAB_EXAMPLE deve estar entre 1 e 16"
#endif

void LabExamples_Start(void);

#endif
