#include <stdio.h>
#include <stdlib.h>
#include "SemanticoGest.h"
extern FILE *yyin;
extern int yylineno;

// Declaramos la función del generador para poder llamarla
extern void escribir_cuadruplos_archivo();

// Contador de errores global
int num_errores_semanticos = 0;
extern int yynerrs;
int num_errores_lexicos = 0;

int yyparse(void);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void yyerror(char *s) {
  num_errores_semanticos++; //¿?
  fprintf(stderr, "[Línea %2d ] *** %s \n ", yylineno, s);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

FILE *abrir_entrada(int argc, char *argv[]) {
  FILE *f = NULL;

  if (argc > 1) {
    f = fopen(argv[1], "r");

    if (f == NULL) {
      fprintf(stderr, "Error: Archivo ' %s ' no encontrado \n ", argv[1]);
      exit(1);
    } else
      printf("\nLeyendo fichero ' %s '\n", argv[1]);
  } else
    printf("\nLeyendo entrada standard .\n");

  return (f);
}

// En main.c

int main(int argc, char *argv[]) {
    // 1. Abrir archivo
    yyin = abrir_entrada(argc, argv);

    // 2. Ejecutar análisis
    int parse_status = yyparse(); 
    
    // 3. COMPROBACIÓN TOTAL DE ERRORES (Léxicos + Sintácticos + Semánticos)
    if (parse_status != 0 || yynerrs > 0 || num_errores_semanticos > 0 || num_errores_lexicos > 0) { // <--- AÑADIDO num_errores_lexicos
        fprintf(stderr, "\n--- PROCESO DETENIDO --- \n");
        fprintf(stderr, "Errores Léxicos: %d\n", num_errores_lexicos);
        fprintf(stderr, "Errores Sintácticos: %d\n", yynerrs);
        fprintf(stderr, "Errores Semánticos: %d\n", num_errores_semanticos);
        return 1;        
    }else{
      // 4. GENERACIÓN DE CÓDIGO (Solo si el código está limpio de todo error)
      escribir_cuadruplos_archivo(); 
    }
    return 0;
}
