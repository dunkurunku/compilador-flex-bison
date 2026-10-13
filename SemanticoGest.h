/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Fichero: SemanticoGest.h
 * Función: Declaración de datos y funciones para la gestión del Árbol Semántico
 *          y de Generación de Código Intermedio.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __SEMANTICOGEST__
#define __SEMANTICOGEST__

#define MAX_CUADRUPLOS 1000

#include "TS.h"
#include <stdio.h>
#include <string.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 ** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *** Funciones para el Análisis Semántico
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 ** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

extern dtipo argumentos[20];
extern int cont;
extern int num_errores_semanticos;
extern int num_errores_lexicos;

int esTipoLista(dtipo t);
void asignacionTipo(char *id, dtipo tipoExpresion);
void verificarCondicion(dtipo tipoExpresion, const char *contexto);
dtipo comprobarOperacionBinaria(dtipo tipo1, dtipo tipoOp, dtipo tipo2);
dtipo comprobarEntrada(char *id);
void comprobarSalida(dtipo tipoExpresion);
void comprobarLlamada(char *id);
dtipo comprobarOperacionUnaria(int atribOp, dtipo tipoOperando);
dtipo comprobarSignoUnario(dtipo tipoOperando);
void contarArgumentos(dtipo tipo);

// --- Funciones Auxiliares para Listas ---
int esLista(dtipo tipo);
dtipo obtenerTipoBase(dtipo tipoLista);
dtipo obtenerTipoLista(dtipo tipoBase);

// Operaciones de Lista
dtipo comprobarAccesoLista(dtipo tipoLista, dtipo tipoIndice);
dtipo comprobarInsercionLista(dtipo tipoLista, dtipo tipoIndice,
                              dtipo tipoValor);
dtipo comprobarListaBinaria(dtipo tipo1, int atribOp, dtipo tipo2);
dtipo comprobarAritmeticaLista(dtipo tipo1, int atribOp, dtipo tipo2);
void comprobarDesplaza(char* lexema);
void comprobarInitCursor(char* lexema);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*** Funciones Generación de Código Intermedio
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef enum {
  SUMA,
  RESTA,
  MULTIPLICACION,
  DIVISION,
  MODULO,
  IGUAL,
  DIFERENTE,
  MAYOR,
  MENOR,
  MAYOR_IGUAL,
  MENOR_IGUAL,
  AND,
  OR,
  NOT,
  ASIGNACION,
  LLAVE_IZQ,
  LLAVE_DER,
  INICIO_PROC,
  FIN_PROC,
  DECLARACION,
  POTENCIA,
  POTENCIA2,
  MENOSMENOS,
  AMP,
  BARR,
  IFGOTO,
  ETIQUETA,
  GOTO,
  PARAMETRO,
  ENTRADA,
  SALIDA,
  LLAMADA_PROC,
  MASUNARIO,
  MENOSUNARIO,
  INIT_CURSOR,
  AVANZAR_CURSOR,
  RETROCEDER_CURSOR,
  NUMERAL,
  INTERR,
  LISTA_SUMA,
  LISTA_RESTA,
  LISTA_MUL,
  LISTA_DIV,
  LLENAR_INTBOOL,
  LLENAR_REAL,
  LLENAR_STRING,
  LLENAR_LISTA
} Op;

static const char *Op_str[] = {
    "SUMA",      "RESTA",       "MULTIPLICACION", "DIVISION",
    "MODULO",    "IGUAL",       "DIFERENTE",      "MAYOR",
    "MENOR",     "MAYOR_IGUAL", "MENOR_IGUAL",    "AND",
    "OR",        "NOT",         "ASIGNACION",     "LLAVE_IZQ",
    "LLAVE_DER", "INICIO_PROC", "FIN_PROC",       "DECLARACION"};

typedef enum {
  REF_VARIABLE,   // El campo 'indice_ts' del union es válido.
  REF_CONST_INT,  // El campo 'valor_entero' del union es válido.
  REF_CONST_REAL, // (Si manejas floats/doubles)
  REF_LABEL,      // El campo 'etiqueta_id' del union es válido.
  REF_TIPO,
  REF_NULA // No hay referencia (operando vacío)
} TipoReferencia;

typedef struct {
  TipoReferencia tipo_dir; // <-- Aquí iría REF_CONST_INT
  union {
    int indice_ts;     // Usado si tipo_dir == REF_VARIABLE
    int valor_entero;  // Usado si tipo_dir == REF_CONST_INT
    float valor_real;  // Usado si tipo_dir == REF_CONST_REAL
    char *etiqueta_id; // Usado si tipo_dir == REF_LABEL
    dtipo tipo;
    // Añadir: float valor_real; para números flotantes
  } valor;
} Direccion;

typedef struct {
  Op op;
  Direccion arg1;
  Direccion arg2;
  Direccion resultado;
} Cuadruplo;

extern Cuadruplo cuadruplos[MAX_CUADRUPLOS];
extern int numCuadruplos;
extern int contador_etiqueta;
extern char *id_init;
extern int contador_temp;
extern char *expresiones[20];
extern int expresionesCont;
extern int anidamiento;

int crear_cuadruplo(Op op, Direccion arg1, Direccion arg2, Direccion resultado);
char *generar_nombre_temporal();
char *generar_nombre_etiqueta();
Direccion crear_direccion_constante_int(int valor);
Direccion crear_direccion_constante_float(float valor);
Direccion crear_direccion_etiqueta(char *nombre_etiqueta);
Direccion crear_direccion_nula();
void imprimir_direccion(Direccion d);
void imprimir_cuadruplos();
void escribir_cuadruplos_archivo();
void generar_cabecera_funcion(int TSKey);
void INTER_Llaveizq();
void INTER_Llaveder();
void INTER_decl_var(char *lexema, dtipo tipo);
void INTER_unop(int clase, int unop, dtipo tipo);
void INTER_binop(int binop, dtipo tipo1, dtipo tipo2);
void INTER_asignacion(char *nombre);
void INTER_salto_condicional(char *etiqueta);
void INTER_etiqueta(char *etiq);
void INTER_goto(char *etiq);
void INTER_finproc();
void INTER_entrada(char *id, dtipo tipo);
void INTER_salida(dtipo tipoExpresion);
void INTER_llamada_proc(char *id);
void INTER_initCursor(char *id);
void INTER_avanzarCursor(char *id);
void INTER_retrocederCursor(char *id);
void INTER_sumrest_lista(int op, dtipo tipo1);
void INTER_multdiv_lista(int op, dtipo tipo1);
void INTER_LlaveIzqAntesExpr();
void INTER_declLista(dtipo tipo);
void verificarAcceso(char *nombre);

#endif
