#ifndef _TS_H_
#define _TS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Tamaño máximo de la pila
#define MAX_TS 500

// Declaración de las variables globales
extern int num_errores_semanticos;

// Tipos de datos posibles (Apéndice F.1)
typedef enum {
  ENTERO,
  REAL,
  CARACTER,
  BOOLEANO,
  LISTA_ENTERO,
  LISTA_REAL,
  LISTA_CARACTER,
  LISTA_BOOLEANO,
  STRING,
  DESCONOCIDO,
  NO_ASIGNADO
} dtipo;

// Tipo de entrada en la tabla (Variable, Función, Marca de bloque...)
typedef enum { MARCA, VARIABLE, PARAMETRO_FORMAL, PROCEDIMIENTO } tipoEntrada;

typedef struct {
  char *EtiquetaEntrada;
  char *EtiquetaSalida;
  char *EtiquetaElse;
  char *NombreVarControl;
} DescriptorDeInstrControl;

// Estructura de una entrada en la Tabla de Símbolos
typedef struct {
  tipoEntrada entrada;
  char *nombre;
  dtipo tipoDato;
  unsigned int parametros;  // Para funciones/procedimientos
  unsigned int dimensiones; // Para arrays (si los hubiera)
  DescriptorDeInstrControl descCtrl;
} entradaTS;

typedef struct {
  int atrib;    // Atributo del símbolo ( si tiene )
  char *lexema; // Nombre del lexema
  dtipo tipo;   // Tipo del símbolo
  char *str;
  int entero;
  float flotante;
  int booleano;
} atributos;

#define YYSTYPE atributos

// Variables Globales
extern entradaTS TS[MAX_TS];
extern long int TOPE;
extern int yylineno;
extern dtipo tipoActual;
extern unsigned int numParametros;

// Funciones de gestión de la TS
void TS_Inicializar();
void TS_InsertaMARCA();
void TS_VaciarENTRADAS();
int TS_InsertaIDENT(char *nombre, dtipo tipo, tipoEntrada entrada);
entradaTS *TS_BuscarIDENT(char *nombre);
entradaTS *TS_BuscarIDENTLocal(char *nombre);
int TS_BuscarIDENTKey(char *nombre);
void TS_Mostrar(); // Para depuración
void TS_copiarParams();

// Funciones auxiliares para tipos
const char *dtipo_to_string(dtipo tipo);
const char *dtipo_to_enum_string(dtipo tipo);

void TS_InsertaMARCA2(char *etiq_entrada, char *etiq_salida, 
                            char *etiq_else, char *nombre_var_control);

// Función para obtener el descriptor de la instrucción de control actual
DescriptorDeInstrControl *obtener_descriptor_actual();

#endif
