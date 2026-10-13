/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *** Fichero: SemanticoGest.c
 *** Funcion: Implementación de funciones para la gestión del A. Semántico
 *** y de Generación de Código Intermedio.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

#include "SemanticoGest.h"
#include <sys/stat.h>

// Importamos la variable del main
extern int num_errores_semanticos;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 ** Funciones para el Análisis Semántico
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

char *id_init;

dtipo argumentos[20];
int cont = 0;

int numCuadruplos = 0;
Cuadruplo cuadruplos[MAX_CUADRUPLOS];


int contador_etiqueta = 0;
int contador_temp = 0;

char *expresiones[20];
int expresionesCont = 0;

int anidamiento = 0;

int esLista(dtipo tipo)
{
  return (tipo == LISTA_ENTERO || tipo == LISTA_REAL ||
          tipo == LISTA_CARACTER || tipo == LISTA_BOOLEANO);
}

dtipo obtenerTipoBase(dtipo tipoLista)
{
  switch (tipoLista)
  {
  case LISTA_ENTERO:
    return ENTERO;
  case LISTA_REAL:
    return REAL;
  case LISTA_CARACTER:
    return CARACTER;
  case LISTA_BOOLEANO:
    return BOOLEANO;
  default:
    return DESCONOCIDO;
  }
}

dtipo obtenerTipoLista(dtipo tipoBase)
{
  switch (tipoBase)
  {
  case ENTERO:
    return LISTA_ENTERO;
  case REAL:
    return LISTA_REAL;
  case STRING:
  case CARACTER:
    return LISTA_CARACTER;
  case BOOLEANO:
    return LISTA_BOOLEANO;
  default:
    return DESCONOCIDO;
  }
}

int esTipoLista(dtipo t)
{
  return (t == LISTA_ENTERO || t == LISTA_REAL || t == LISTA_CARACTER || t == LISTA_BOOLEANO);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 ** Funciones para el Análisis Semántico (General)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

void asignacionTipo(char *id, dtipo tipoExpresion)
{
  /* Comprobación de Tipos en Asignación */
  entradaTS *var = TS_BuscarIDENT(id);
  if (var == NULL)
  {
    num_errores_semanticos++;
    fprintf(stderr, "Error Semántico (Línea %d): Variable '%s' no declarada.\n",
            yylineno, id);
  }
  else
  {
    if (var->tipoDato != tipoExpresion && tipoExpresion != DESCONOCIDO)
    {
      fprintf(stderr,
              "Error Semántico (Línea %d): Asignación incompatible. Variable "
              "'%s' es %s. %s\n",
              yylineno, id, dtipo_to_string(var->tipoDato),
              dtipo_to_string(tipoExpresion));
      num_errores_semanticos++;
    }
  }
}

void verificarCondicion(dtipo tipoExpresion, const char *contexto)
{
  if (tipoExpresion != BOOLEANO && tipoExpresion != DESCONOCIDO)
  {
    num_errores_semanticos++;
    fprintf(stderr,
            "Error Semántico (Línea %d): La condición del %s debe ser bool.\n",
            yylineno, contexto);
  }
}

/* Comprobación para operaciones estándar (no listas) */
dtipo comprobarOperacionBinaria(dtipo tipo1, dtipo tipoOp, dtipo tipo2)
{
  if (tipoOp == BOOLEANO)
  {
    return BOOLEANO;
  }
  else if (tipo1 == tipo2)
  {
    return tipo1;
  }
  else
  {
    num_errores_semanticos++;
    fprintf(stderr,
            "Error Semántico (Línea %d): Los tipos de las variables no "
            "coinciden.\n",
            yylineno);
    return DESCONOCIDO;
  }
}

dtipo comprobarEntrada(char *id)
{
  entradaTS *var = TS_BuscarIDENT(id);
  if (var == NULL)
  {
    num_errores_semanticos++;
    fprintf(stderr, "Error Semántico (Línea %d): Variable '%s' no declarada.\n",
            yylineno, id);
  }
  else if (var->tipoDato != STRING && var->tipoDato != ENTERO && var->tipoDato != REAL)
  {
    num_errores_semanticos++;
    fprintf(stderr,
            "Error Semántico (Línea %d): La variable de entrada '%s' debe ser "
            "de tipo string, entero o real.\n",
            yylineno, id);
  }
  return var->tipoDato;
}

void comprobarSalida(dtipo tipoExpresion)
{
  if (tipoExpresion != STRING && tipoExpresion != ENTERO && tipoExpresion != REAL)
  {
    num_errores_semanticos++;
    fprintf(stderr,
            "Error Semántico (Línea %d): La expresión de salida debe ser de "
            "tipo string, entero o real.\n",
            yylineno);
  }
}

void comprobarLlamada(char *id)
{
  int key = TS_BuscarIDENTKey(id);
  entradaTS *simbolo;
  if (key == -1)
  {
    num_errores_semanticos++;
    fprintf(stderr,
            "Error Semántico (Línea %d): Subprograma '%s' no declarado.\n",
            yylineno, id);
  }
  else
  {
    simbolo = &TS[key];
  }
  if (simbolo->entrada != PROCEDIMIENTO)
  {
    num_errores_semanticos++;
    fprintf(stderr, "Error Semántico (Línea %d): '%s' no es un subprograma.\n",
            yylineno, id);
  }

  if (simbolo->parametros != cont)
  {
    num_errores_semanticos++;
    fprintf(stderr,
            "Error Semántico (Línea %d): Número de parámetros no coincide.\n",
            yylineno);
  }

  for (int i = key - cont; i < key; i++)
  {
    if (TS[i].tipoDato != argumentos[i - (key - cont)])
    {
      num_errores_semanticos++;
      fprintf(stderr,
              "Error Semántico (Línea %d): Parámetro %d no coincide el tipo. "
              "Parametro: %s, Argumento: %s \n",
              yylineno, i - (key - cont) + 1, dtipo_to_string(TS[i].tipoDato),
              dtipo_to_string(argumentos[i - (key - cont)]));
    }
  }
  cont = 0;
}

/* Modificado para soportar operadores de lista (#, ?) además de '!' */
dtipo comprobarOperacionUnaria(int atribOp, dtipo tipoOperando)
{
  if (tipoOperando == DESCONOCIDO)
    return DESCONOCIDO;

  // Operador '!' (NOT) - atrib 0
  if (atribOp == 0)
  {
    if (tipoOperando != BOOLEANO && tipoOperando != LISTA_BOOLEANO)
    {
      num_errores_semanticos++;
      fprintf(stderr,
              "Error Semántico (Línea %d): Se esperaba bool para operador "
              "'!'.\n",
              yylineno);
      return DESCONOCIDO;
    }
    return tipoOperando; // Mantiene el tipo (bool o lista bool)
  }

  // Operador '#' (Longitud) - atrib 1 -> Devuelve ENTERO
  if (atribOp == 1)
  {
    if (!esLista(tipoOperando))
    {
      num_errores_semanticos++;
      fprintf(stderr,
              "Error Semántico (Línea %d): El operador '#' (longitud) espera "
              "una lista.\n",
              yylineno);
      return DESCONOCIDO;
    }
    return ENTERO;
  }

  // Operador '?' (Elemento actual/Cursor) - atrib 2 -> Devuelve Tipo Base
  if (atribOp == 2)
  {
    if (!esLista(tipoOperando))
    {
      num_errores_semanticos++;
      fprintf(stderr,
              "Error Semántico (Línea %d): El operador '?' (cursor) espera una "
              "lista.\n",
              yylineno);
      return DESCONOCIDO;
    }
    return obtenerTipoBase(tipoOperando);
  }

  return tipoOperando;
}

dtipo comprobarSignoUnario(dtipo tipoOperando)
{
  // Operadores unarios + o - esperan tipos numéricos
  if (tipoOperando != ENTERO && tipoOperando != REAL)
  {
    num_errores_semanticos++;
    fprintf(stderr,
            "Error Semántico (Línea %d): Se esperaba tipo numérico para signo "
            "+/-.\n",
            yylineno);
    return DESCONOCIDO;
  }
  else
  {
    return tipoOperando;
  }
}

void contarArgumentos(dtipo tipo)
{
  argumentos[cont] = tipo;
  cont++;
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 ** Funciones Específicas para Listas
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

/* Operador Binario '@' (acceso posicional) */
dtipo comprobarAccesoLista(dtipo tipoLista, dtipo tipoIndice)
{

  if (tipoLista == DESCONOCIDO || tipoIndice == DESCONOCIDO)
    return DESCONOCIDO;

  if (!esLista(tipoLista))
  {
    num_errores_semanticos++;
    fprintf(stderr,
            "Error Semántico (Línea %d): El operador '@' requiere una lista a "
            "la izquierda.\n",
            yylineno);
    return DESCONOCIDO;
  }

  if (tipoIndice != ENTERO)
  {
    num_errores_semanticos++;
    fprintf(
        stderr,
        "Error Semántico (Línea %d): El índice de la lista debe ser entero.\n",
        yylineno);
    return DESCONOCIDO;
  }

  return obtenerTipoBase(tipoLista);
}

/* Operador Ternario '@...++...' (inserción) */
dtipo comprobarInsercionLista(dtipo tipoLista, dtipo tipoIndice,
                              dtipo tipoValor)
{

  if (tipoLista == DESCONOCIDO || tipoIndice == DESCONOCIDO ||
      tipoValor == DESCONOCIDO)
    return DESCONOCIDO;

  if (!esLista(tipoLista))
  {
    num_errores_semanticos++;
    fprintf(stderr,
            "Error Semántico (Línea %d): La inserción requiere una lista.\n",
            yylineno);
    return DESCONOCIDO;
  }

  if (tipoIndice != ENTERO)
  {
    num_errores_semanticos++;
    fprintf(stderr,
            "Error Semántico (Línea %d): La posición de inserción debe ser un "
            "entero.\n",
            yylineno);
    return DESCONOCIDO;
  }

  dtipo base = obtenerTipoBase(tipoLista);

  if (base != tipoValor)
  {
    num_errores_semanticos++;
    fprintf(stderr,
            "Error Semántico (Línea %d): Tipo incompatible en inserción. La "
            "lista espera %s pero se inserta %s.\n",
            yylineno, dtipo_to_string(base), dtipo_to_string(tipoValor));
    return DESCONOCIDO;
  }

  return tipoLista;
}

/* Operadores binarios de listas: ** (concat), -- (borrar elem), % (borrar
 * resto) */
dtipo comprobarListaBinaria(dtipo tipo1, int atribOp, dtipo tipo2)
{

  if (tipo1 == DESCONOCIDO || tipo2 == DESCONOCIDO)
    return DESCONOCIDO;

  // Concatenación (**): atrib 3
  if (atribOp == 3)
  {
    if (esLista(tipo1) && esLista(tipo2))
    {
      if (tipo1 == tipo2)
      {
        return tipo1;
      }
      else
      {
        num_errores_semanticos++;
        fprintf(stderr,
                "Error Semántico (Línea %d): Las listas a concatenar deben ser "
                "del mismo tipo base.\n",
                yylineno);
        return DESCONOCIDO;
      }
    }
    else
    {
      num_errores_semanticos++;
      fprintf(
          stderr,
          "Error Semántico (Línea %d): El operador '**' requiere dos listas.\n",
          yylineno);
      return DESCONOCIDO;
    }
  }

  // Borrado (-- y %): atrib 14 y 2
  if (atribOp == 14 || atribOp == 2)
  {
    if (!esLista(tipo1))
    {
      num_errores_semanticos++;
      fprintf(stderr,
              "Error Semántico (Línea %d): Se esperaba una lista a la "
              "izquierda del operador.\n",
              yylineno);
      return DESCONOCIDO;
    }
    if (tipo2 != ENTERO)
    {
      num_errores_semanticos++;
      fprintf(stderr,
              "Error Semántico (Línea %d): La posición para borrar debe ser un "
              "entero.\n",
              yylineno);
      return DESCONOCIDO;
    }
    return tipo1;
  }

  return DESCONOCIDO;
}

/* Aritmética de Listas (+, -, *, /) */
dtipo comprobarAritmeticaLista(dtipo tipo1, int atribOp, dtipo tipo2)
{

  if (!esLista(tipo1) && !esLista(tipo2))
    return DESCONOCIDO;

  // Caso 1: Lista op Valor
  if (esLista(tipo1))
  {
    dtipo base = obtenerTipoBase(tipo1);
    if (base == tipo2)
      return tipo1;

    // Manejo especial: Lista Float op Entero (si tu lenguaje permite promoción
    // implícita)
    if (base == REAL && tipo2 == ENTERO)
      return tipo1;
    
    num_errores_semanticos++;
    fprintf(stderr,
            "Error Semántico (Línea %d): Operación lista-valor incompatible. "
            "Lista de %s con valor %s.\n",
            yylineno, dtipo_to_string(base), dtipo_to_string(tipo2));
    return DESCONOCIDO;
  }

  // Caso 2: Valor op Lista
  if (esLista(tipo2))
  {
    dtipo base = obtenerTipoBase(tipo2);
    if (base == tipo1)
      return tipo2;

    if (base == REAL && tipo1 == ENTERO)
      return tipo2;

    num_errores_semanticos++;
    fprintf(stderr,
            "Error Semántico (Línea %d): Operación valor-lista incompatible. "
            "Valor %s con lista de %s.\n",
            yylineno, dtipo_to_string(tipo1), dtipo_to_string(base));
    return DESCONOCIDO;
  }

  return DESCONOCIDO;
}

void comprobarDesplaza(char *lexema)
{
  entradaTS *ent = TS_BuscarIDENT(lexema);
  if (ent == NULL)
  {
    num_errores_semanticos++;
    fprintf(stderr, "Error Semántico (Línea %d): Variable '%s' no declarada.\n",
            yylineno, lexema);
  }
  else
  {
    if (!esLista(ent->tipoDato))
    {
      num_errores_semanticos++;
      fprintf(stderr,
              "Error Semántico (Línea %d): Los operadores '>>' y '<<' requieren una lista.\n",
              yylineno);
    }
  }
}

void comprobarInitCursor(char *lexema)
{
  entradaTS *ent = TS_BuscarIDENT(lexema);
  if (ent == NULL)
  {
    num_errores_semanticos++;
    fprintf(stderr, "Error Semántico (Línea %d): Variable '%s' no declarada.\n",
            yylineno, lexema);
  }
  else
  {
    if (!esLista(ent->tipoDato))
    {
      num_errores_semanticos++;
      fprintf(stderr,
              "Error Semántico (Línea %d): El operador $ requieren una lista.\n",
              yylineno);
    }
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
** Funciones Generación de Código Intermedio
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int crear_cuadruplo(Op op, Direccion arg1, Direccion arg2,
                    Direccion resultado)
{
  if (numCuadruplos >= MAX_CUADRUPLOS)
  {
    fprintf(stderr, "Error: Límite de cuádruplos alcanzado (%d).\n",
            MAX_CUADRUPLOS);
    exit(1);
  }

  cuadruplos[numCuadruplos].op = op;
  cuadruplos[numCuadruplos].arg1 = arg1;
  cuadruplos[numCuadruplos].arg2 = arg2;
  cuadruplos[numCuadruplos].resultado = resultado;

  return numCuadruplos++;
}

char *generar_nombre_temporal()
{
  char cadena[20];
  sprintf(cadena, "temp%d", contador_temp++);
  return strdup(cadena);
}

char *generar_nombre_etiqueta()
{
  char cadena[20];
  sprintf(cadena, "etiqueta%d", contador_etiqueta++);
  return strdup(cadena);
}

Direccion crear_direccion_constante_int(int valor)
{
  Direccion d;
  d.tipo_dir = REF_CONST_INT;
  d.valor.valor_entero = valor;
  return d;
}

Direccion crear_direccion_constante_float(float valor)
{
  Direccion d;
  d.tipo_dir = REF_CONST_REAL;
  d.valor.valor_real = valor;
  return d;
}

Direccion crear_direccion_etiqueta(char *nombre_etiqueta)
{
  Direccion d;
  d.tipo_dir = REF_LABEL;
  d.valor.etiqueta_id = strdup(nombre_etiqueta);
  return d;
}

Direccion crear_direccion_nula()
{
  Direccion d;
  d.tipo_dir = REF_NULA;
  return d;
}

void imprimir_direccion(Direccion d)
{
  switch (d.tipo_dir)
  {
  case REF_VARIABLE:
    printf("VAR(%d)", d.valor.indice_ts);
    break;
  case REF_CONST_INT:
    printf("CONST_INT(%d)", d.valor.valor_entero);
    break;
  case REF_LABEL:
    printf("LABEL(%s)", d.valor.etiqueta_id);
    break;
  case REF_CONST_REAL:
    printf("CONST_REAL(%.2f)", d.valor.valor_real);
    break;
  case REF_NULA:
    printf("-");
    break;
  }
}

void imprimir_cuadruplos()
{
  /* printf("\n--- CODIGO INTERMEDIO (Cuádruplos) ---\n");
  for (int i = 0; i < numCuadruplos; i++) {
    if (cuadruplos[i].op == INICIO_PROC) {
      int TSkey = cuadruplos[i].arg1.valor.valor_entero;
      entradaTS *ent = &TS[TSkey];
      if (strcmp(ent->nombre, id_init) == 0) {
        int params = ent->parametros;
        for (int i = TSkey - params; i < TSkey; i++) {
          printf("%s %s;\n", dtipo_to_string(TS[i].tipoDato), TS[i].nombre);
        }
        printf("int main()\n");
      } else {
        printf("void %s(", ent->nombre);
        int params = ent->parametros;
        for (int i = TSkey - params; i < TSkey; i++) {
          printf("%s %s\n", dtipo_to_string(TS[i].tipoDato), TS[i].nombre);
          if (i + 1 < TSkey)
            printf(",");
        }
        printf(")\n");
      }
    } else if (cuadruplos[i].op == LLAVE_IZQ) {
      printf("{\n");
    } else if (cuadruplos[i].op == LLAVE_DER) {
      printf("}\n");
    } else if (cuadruplos[i].op == DECLARACION) {
      int TSkey = cuadruplos[i].arg1.valor.valor_entero;
      entradaTS *ent = &TS[TSkey];
      printf("%s %s;\n", dtipo_to_string(ent->tipoDato), ent->nombre);
    } else if (cuadruplos[i].op == SUMA) {
      int TSkey = cuadruplos[i].resultado.valor.valor_entero;
      entradaTS *ent = &TS[TSkey];
      printf("%s = %s + %s;\n", ent->nombre,
             cuadruplos[i].arg1.valor.etiqueta_id,
             cuadruplos[i].arg2.valor.etiqueta_id);
    } else if (cuadruplos[i].op == RESTA) {
      int TSkey = cuadruplos[i].resultado.valor.valor_entero;
      entradaTS *ent = &TS[TSkey];
      printf("%s = %s - %s;\n", ent->nombre,
             cuadruplos[i].arg1.valor.etiqueta_id,
             cuadruplos[i].arg2.valor.etiqueta_id);
    } else if (cuadruplos[i].op == MULTIPLICACION) {
      int TSkey = cuadruplos[i].resultado.valor.valor_entero;
      entradaTS *ent = &TS[TSkey];
      printf("%s = %s * %s;\n", ent->nombre,
             cuadruplos[i].arg1.valor.etiqueta_id,
             cuadruplos[i].arg2.valor.etiqueta_id);
    } else if (cuadruplos[i].op == DIVISION) {
      int TSkey = cuadruplos[i].resultado.valor.valor_entero;
      entradaTS *ent = &TS[TSkey];
      printf("%s = %s / %s;\n", ent->nombre,
             cuadruplos[i].arg1.valor.etiqueta_id,
             cuadruplos[i].arg2.valor.etiqueta_id);
    } else if (cuadruplos[i].op == MODULO) {
      int TSkey = cuadruplos[i].resultado.valor.valor_entero;
      entradaTS *ent = &TS[TSkey];
      printf("%s = %s %% %s;\n", ent->nombre,
             cuadruplos[i].arg1.valor.etiqueta_id,
             cuadruplos[i].arg2.valor.etiqueta_id);
    } else if (cuadruplos[i].op == IGUAL) {
      int TSkey = cuadruplos[i].resultado.valor.valor_entero;
      entradaTS *ent = &TS[TSkey];
      printf("%s = %s;\n", ent->nombre, cuadruplos[i].arg1.valor.etiqueta_id);
    } else if (cuadruplos[i].op == DIFERENTE) {
      int TSkey = cuadruplos[i].resultado.valor.valor_entero;
      entradaTS *ent = &TS[TSkey];
      printf("%s = %s != %s;\n", ent->nombre,
             cuadruplos[i].arg1.valor.etiqueta_id,
             cuadruplos[i].arg2.valor.etiqueta_id);
    } else if (cuadruplos[i].op == MAYOR) {
      int TSkey = cuadruplos[i].resultado.valor.valor_entero;
      entradaTS *ent = &TS[TSkey];
      printf("%s = %s > %s;\n", ent->nombre,
             cuadruplos[i].arg1.valor.etiqueta_id,
             cuadruplos[i].arg2.valor.etiqueta_id);
    } else if (cuadruplos[i].op == MENOR) {
      int TSkey = cuadruplos[i].resultado.valor.valor_entero;
      entradaTS *ent = &TS[TSkey];
      printf("%s = %s < %s;\n", ent->nombre,
             cuadruplos[i].arg1.valor.etiqueta_id,
             cuadruplos[i].arg2.valor.etiqueta_id);
    } else if (cuadruplos[i].op == MAYOR_IGUAL) {
      int TSkey = cuadruplos[i].resultado.valor.valor_entero;
      entradaTS *ent = &TS[TSkey];
      printf("%s = %s >= %s;\n", ent->nombre,
             cuadruplos[i].arg1.valor.etiqueta_id,
             cuadruplos[i].arg2.valor.etiqueta_id);
    } else if (cuadruplos[i].op == MENOR_IGUAL) {
      int TSkey = cuadruplos[i].resultado.valor.valor_entero;
      entradaTS *ent = &TS[TSkey];
      printf("%s = %s <= %s;\n", ent->nombre,
             cuadruplos[i].arg1.valor.etiqueta_id,
             cuadruplos[i].arg2.valor.etiqueta_id);
    } else if (cuadruplos[i].op == AND) {
      int TSkey = cuadruplos[i].resultado.valor.valor_entero;
      entradaTS *ent = &TS[TSkey];
      printf("%s = %s && %s;\n", ent->nombre,
             cuadruplos[i].arg1.valor.etiqueta_id,
             cuadruplos[i].arg2.valor.etiqueta_id);
    } else if (cuadruplos[i].op == OR) {
      int TSkey = cuadruplos[i].resultado.valor.valor_entero;
      entradaTS *ent = &TS[TSkey];
      printf("%s = %s || %s;\n", ent->nombre,
             cuadruplos[i].arg1.valor.etiqueta_id,
             cuadruplos[i].arg2.valor.etiqueta_id);
    } else if (cuadruplos[i].op == NOT) {
      int TSkey = cuadruplos[i].resultado.valor.valor_entero;
      entradaTS *ent = &TS[TSkey];
      printf("%s = !%s;\n", ent->nombre, cuadruplos[i].arg1.valor.etiqueta_id);
    } else if (cuadruplos[i].op == ASIGNACION) {
      int TSkey = cuadruplos[i].resultado.valor.valor_entero;
      entradaTS *ent = &TS[TSkey];
      printf("%s = %s;\n", ent->nombre, cuadruplos[i].arg1.valor.etiqueta_id);
    } else if (cuadruplos[i].op == IFGOTO) {
      printf("if (!%s) goto %s;\n", cuadruplos[i].arg1.valor.etiqueta_id,
             cuadruplos[i].arg2.valor.etiqueta_id);
    } else if (cuadruplos[i].op == ETIQUETA) {
      printf("%s:\n", cuadruplos[i].arg1.valor.etiqueta_id);
    } else if (cuadruplos[i].op == GOTO) {
      printf("goto %s;\n", cuadruplos[i].arg1.valor.etiqueta_id);
    } else if (cuadruplos[i].op == ENTRADA) {
      int TSkey = cuadruplos[i].resultado.valor.valor_entero;
      entradaTS *ent = &TS[TSkey];
      printf("scanf(\"%%s\", &%s);\n", ent->nombre);
    }else if (cuadruplos[i].op == SALIDA) {
      Direccion arg1 = cuadruplos[i].arg1;
      char *nombre = arg1.valor.etiqueta_id;
      entradaTS *ent = TS_BuscarIDENT(nombre);
      printf("printf(\"%%s\\n\", %s);\n", nombre);
    } else if (cuadruplos[i].op == LLAMADA_PROC) {
        int TSkey = cuadruplos[i].arg1.valor.valor_entero;
        entradaTS *ent = &TS[TSkey];

        // Imprimir nombre de la función
        printf("%s(", ent->nombre);

        // Imprimir los argumentos que guardamos en arg2 (si existen)
        if (cuadruplos[i].arg2.tipo_dir == REF_LABEL) {
            printf("%s", cuadruplos[i].arg2.valor.etiqueta_id);
        }

        printf(");\n");
    }

  } */
}

void escribir_cuadruplos_archivo()
{
  // Create the 'salida' directory if it doesn't exist
  mkdir("./salida", 0777);

  FILE *fmain = fopen("./salida/main.c", "w");
  if (fmain == NULL)
  {
    fprintf(stderr, "Error al abrir el archivo main.c para escritura.\n");
    return;
  }

  FILE *fFunc = fopen("./salida/dec_fun.h", "w");
  if (fFunc == NULL)
  {
    fprintf(stderr, "Error al abrir el archivo dec_fun.h para escritura.\n");
    return;
  }

  fprintf(fmain, "#include <stdio.h>\n");
  fprintf(fmain, "#include <stdbool.h>\n");
  fprintf(fmain, "#include \"dec_fun.h\"\n");
  fprintf(fmain, "#include \"dec_dat.h\"\n\n");

  fprintf(fFunc, "#include <stdio.h>\n");
  fprintf(fFunc, "#include <stdbool.h>\n");

  FILE *f = fmain;

  for (int i = 0; i < numCuadruplos; i++)
  {
    if (cuadruplos[i].op == INICIO_PROC)
    {
      char *nombre = cuadruplos[i].arg1.valor.etiqueta_id;
      int params = cuadruplos[i].arg2.valor.valor_entero;
      if (strcmp(nombre, id_init) == 0)
      {
        for (int j = i - params; j < i; j++)
        {
          fprintf(f, "%s %s;\n", dtipo_to_string(cuadruplos[j].arg1.valor.tipo),
                  cuadruplos[j].arg2.valor.etiqueta_id);
        }
        fprintf(f, "int main(int argc, char **argv)\n");
      }
      else
      {
        f = fFunc;
        anidamiento++;
        char *nombre = cuadruplos[i].arg1.valor.etiqueta_id;
        int params = cuadruplos[i].arg2.valor.valor_entero;
        fprintf(f, "void %s(", nombre);
        for (int j = i - params; j < i; j++)
        {
          const char *t;
          if (cuadruplos[j].arg1.valor.tipo == STRING)
            t = "const char*";
          else
            t = dtipo_to_string(cuadruplos[j].arg1.valor.tipo);
          fprintf(f, "%s %s", t, cuadruplos[j].arg2.valor.etiqueta_id);
          if (j + 1 < i)
            fprintf(f, ", ");
        }
        fprintf(f, ")\n");
      }
    }
    else if (cuadruplos[i].op == LLAVE_IZQ)
    {
      fprintf(f, "{\n");
    }
    else if (cuadruplos[i].op == LLAVE_DER)
    {
      fprintf(f, "}\n");
    }
    else if (cuadruplos[i].op == DECLARACION)
    {
      dtipo t = cuadruplos[i].arg1.valor.tipo;
      if (esLista(t))
      {
        fprintf(f, "Lista *%s = crearLista(%s);\n", cuadruplos[i].arg2.valor.etiqueta_id, dtipo_to_enum_string(obtenerTipoBase(t)));
      }
      else
        fprintf(f, "%s %s;\n", dtipo_to_string(cuadruplos[i].arg1.valor.tipo),
                cuadruplos[i].arg2.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == SUMA)
    {
      fprintf(f, "%s = %s + %s;\n", cuadruplos[i].resultado.valor.etiqueta_id,
              cuadruplos[i].arg1.valor.etiqueta_id,
              cuadruplos[i].arg2.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == RESTA)
    {
      fprintf(f, "%s = %s - %s;\n", cuadruplos[i].resultado.valor.etiqueta_id,
              cuadruplos[i].arg1.valor.etiqueta_id,
              cuadruplos[i].arg2.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == MULTIPLICACION)
    {
      fprintf(f, "%s = %s * %s;\n", cuadruplos[i].resultado.valor.etiqueta_id,
              cuadruplos[i].arg1.valor.etiqueta_id,
              cuadruplos[i].arg2.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == DIVISION)
    {
      fprintf(f, "%s = %s / %s;\n", cuadruplos[i].resultado.valor.etiqueta_id,
              cuadruplos[i].arg1.valor.etiqueta_id,
              cuadruplos[i].arg2.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == MODULO)
    {
      fprintf(f, "%s = %s %% %s;\n", cuadruplos[i].resultado.valor.etiqueta_id,
              cuadruplos[i].arg1.valor.etiqueta_id,
              cuadruplos[i].arg2.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == IGUAL)
    {
      fprintf(f, "%s = %s == %s;\n", cuadruplos[i].resultado.valor.etiqueta_id,
              cuadruplos[i].arg1.valor.etiqueta_id,
              cuadruplos[i].arg2.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == DIFERENTE)
    {
      fprintf(f, "%s = %s != %s;\n", cuadruplos[i].resultado.valor.etiqueta_id,
              cuadruplos[i].arg1.valor.etiqueta_id,
              cuadruplos[i].arg2.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == MAYOR)
    {
      fprintf(f, "%s = %s > %s;\n", cuadruplos[i].resultado.valor.etiqueta_id,
              cuadruplos[i].arg1.valor.etiqueta_id,
              cuadruplos[i].arg2.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == MENOR)
    {
      fprintf(f, "%s = %s < %s;\n", cuadruplos[i].resultado.valor.etiqueta_id,
              cuadruplos[i].arg1.valor.etiqueta_id,
              cuadruplos[i].arg2.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == MAYOR_IGUAL)
    {
      fprintf(f, "%s = %s >= %s;\n", cuadruplos[i].resultado.valor.etiqueta_id,
              cuadruplos[i].arg1.valor.etiqueta_id,
              cuadruplos[i].arg2.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == MENOR_IGUAL)
    {
      fprintf(f, "%s = %s <= %s;\n", cuadruplos[i].resultado.valor.etiqueta_id,
              cuadruplos[i].arg1.valor.etiqueta_id,
              cuadruplos[i].arg2.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == AND)
    {
      fprintf(f, "%s = %s && %s;\n", cuadruplos[i].resultado.valor.etiqueta_id,
              cuadruplos[i].arg1.valor.etiqueta_id,
              cuadruplos[i].arg2.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == OR)
    {
      fprintf(f, "%s = %s || %s;\n", cuadruplos[i].resultado.valor.etiqueta_id,
              cuadruplos[i].arg1.valor.etiqueta_id,
              cuadruplos[i].arg2.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == NOT)
    {
      fprintf(f, "%s = !%s;\n", cuadruplos[i].resultado.valor.etiqueta_id,
              cuadruplos[i].arg1.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == MASUNARIO)
    {
      fprintf(f, "%s = +%s;\n", cuadruplos[i].resultado.valor.etiqueta_id,
              cuadruplos[i].arg1.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == MENOSUNARIO)
    {
      fprintf(f, "%s = -%s;\n", cuadruplos[i].resultado.valor.etiqueta_id,
              cuadruplos[i].arg1.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == ASIGNACION)
    {
      fprintf(f, "%s = %s;\n", cuadruplos[i].resultado.valor.etiqueta_id,
              cuadruplos[i].arg1.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == IFGOTO)
    {
      fprintf(f, "if (!%s) goto %s;\n", cuadruplos[i].arg1.valor.etiqueta_id,
              cuadruplos[i].arg2.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == ETIQUETA)
    {
      fprintf(f, "%s: ;\n", cuadruplos[i].arg1.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == GOTO)
    {
      fprintf(f, "goto %s;\n", cuadruplos[i].arg1.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == FIN_PROC)
    {
      anidamiento--;
      if (anidamiento == 0)
        f = fmain;
    }
    else if (cuadruplos[i].op == ENTRADA)
    {
      char *formato;
      if (cuadruplos[i].arg1.valor.tipo == ENTERO)
        formato = "%d";
      else if (cuadruplos[i].arg1.valor.tipo == REAL)
        formato = "%f";
      else
        formato = "%s";
      fprintf(f, "scanf(\"%s\", &%s);\n",
              formato,
              cuadruplos[i].resultado.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == SALIDA)
    {
      char *formato;
      if (cuadruplos[i].arg2.valor.tipo == ENTERO)
        formato = "%d";
      else if (cuadruplos[i].arg2.valor.tipo == REAL)
        formato = "%f";
      else
        formato = "%s";
      fprintf(f, "printf(\"%s\", %s);\n",
              formato,
              cuadruplos[i].arg1.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == LLAMADA_PROC)
    {
      fprintf(f, "%s(%s);\n", cuadruplos[i].arg1.valor.etiqueta_id,
              cuadruplos[i].arg2.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == INIT_CURSOR)
    {
      fprintf(f, "cursorInicio(%s);\n", cuadruplos[i].arg1.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == AVANZAR_CURSOR)
    {
      fprintf(f, "cursorAdelante(%s);\n", cuadruplos[i].arg1.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == RETROCEDER_CURSOR)
    {
      fprintf(f, "cursorAtras(%s);\n", cuadruplos[i].arg1.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == LISTA_SUMA)
    {
      fprintf(f, "SumaValor(%s, %s);\n", cuadruplos[i].arg1.valor.etiqueta_id, cuadruplos[i].arg2.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == LISTA_RESTA)
    {
      fprintf(f, "SumaValor(%s, -%s);\n", cuadruplos[i].arg1.valor.etiqueta_id, cuadruplos[i].arg2.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == LISTA_MUL)
    {
      fprintf(f, "MultiplicaValor(%s, %s);\n", cuadruplos[i].arg1.valor.etiqueta_id, cuadruplos[i].arg2.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == LISTA_DIV)
    {
      fprintf(f, "MultiplicaValor(%s, 1/%s);\n", cuadruplos[i].arg1.valor.etiqueta_id, cuadruplos[i].arg2.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == LLENAR_INTBOOL)
    {
      fprintf(f, "LlenarLista(%s, (int[]){%s}, %s);\n", cuadruplos[i].resultado.valor.etiqueta_id, cuadruplos[i].arg1.valor.etiqueta_id, cuadruplos[i].arg2.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == LLENAR_REAL)
    {
      fprintf(f, "LlenarLista(%s, (float[]){%s}, %s);\n", cuadruplos[i].resultado.valor.etiqueta_id, cuadruplos[i].arg1.valor.etiqueta_id, cuadruplos[i].arg2.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == LLENAR_STRING)
    {
      fprintf(f, "LlenarLista(%s, (char*[]){%s}, %s);\n", cuadruplos[i].resultado.valor.etiqueta_id, cuadruplos[i].arg1.valor.etiqueta_id, cuadruplos[i].arg2.valor.etiqueta_id);
    }
    else if (cuadruplos[i].op == LLENAR_LISTA)
    {
      fprintf(f, "%s;\n", cuadruplos[i].arg1.valor.etiqueta_id);
    }
  }
  fclose(fmain);
  fclose(fFunc);

  // Compilar el código generado
  printf("Compilando el código generado...\n");
  int result = system("gcc -o ./salida/programa ./salida/main.c -Wall");
  if (result == 0)
  {
    printf("Compilación exitosa. Ejecutable generado en ./salida/programa\n");
  }
  else
  {
    fprintf(stderr, "Error durante la compilación.\n");
  }
}

// Generacion de codigo intermedio - reglas

void generar_cabecera_funcion(int TSKey)
{
  entradaTS *ent = &TS[TSKey];

  for (int i = TSKey - ent->parametros; i < TSKey; i++)
  {
    Direccion tipo;
    tipo.tipo_dir = REF_TIPO;
    tipo.valor.tipo = TS[i].tipoDato;
    Direccion nombre;
    nombre.tipo_dir = REF_LABEL;
    nombre.valor.etiqueta_id = strdup(TS[i].nombre);
    crear_cuadruplo(PARAMETRO, tipo, nombre, crear_direccion_nula());
  }

  Direccion nombre;
  nombre.tipo_dir = REF_LABEL;
  nombre.valor.etiqueta_id = strdup(ent->nombre);
  Direccion parametros;
  parametros.tipo_dir = REF_CONST_INT;
  parametros.valor.valor_entero = ent->parametros;
  crear_cuadruplo(INICIO_PROC, nombre, parametros, crear_direccion_nula());
}

void INTER_Llaveizq()
{
  contador_temp = 0;
  expresionesCont = 0;
  crear_cuadruplo(LLAVE_IZQ, crear_direccion_nula(), crear_direccion_nula(),
                  crear_direccion_nula());
}

void INTER_Llaveder()
{
  crear_cuadruplo(LLAVE_DER, crear_direccion_nula(), crear_direccion_nula(),
                  crear_direccion_nula());
}

void INTER_decl_var(char *nombre, dtipo tipo)
{
  Direccion t;
  t.tipo_dir = REF_TIPO;
  t.valor.tipo = tipo;
  Direccion n;
  n.tipo_dir = REF_LABEL;
  n.valor.etiqueta_id = nombre;
  crear_cuadruplo(DECLARACION, t, n, crear_direccion_nula());
}

void INTER_unop(int clase, int unop, dtipo tipo)
{
  char *nombre = generar_nombre_temporal();
  INTER_decl_var(nombre, tipo);
  Direccion result;
  result.tipo_dir = REF_LABEL;
  result.valor.etiqueta_id = nombre;

  Direccion expr;
  expr.tipo_dir = REF_LABEL;
  expr.valor.etiqueta_id = expresiones[expresionesCont - 1];
  Op op;
  if (clase == 0)
  {
    switch (unop)
    {
    case 0:
      op = NOT;
      break;
    case 1:
      op = NUMERAL;
      break;
    case 2:
      op = INTERR;
      break;
    }
  }
  else
  {
    switch (unop)
    {
    case 0:
      op = MASUNARIO;
      break;
    case 1:
      op = MENOSUNARIO;
      break;
    }
  }
  crear_cuadruplo(op, expr, crear_direccion_nula(), result);
  expresiones[expresionesCont - 1] = nombre;
}

void INTER_binop(int binop, dtipo tipo1, dtipo tipo2)
{
  char *nombre = generar_nombre_temporal();
  INTER_decl_var(nombre, tipo1);
  Direccion result;
  result.tipo_dir = REF_LABEL;
  result.valor.etiqueta_id = nombre;

  Direccion expr1;
  expr1.tipo_dir = REF_LABEL;
  expr1.valor.etiqueta_id = expresiones[expresionesCont - 2];
  Direccion expr2;
  expr2.tipo_dir = REF_LABEL;
  expr2.valor.etiqueta_id = expresiones[expresionesCont - 1];
  Op op;
  switch (binop)
  {
  case 0:
    op = MULTIPLICACION;
    break;
  case 1:
    op = DIVISION;
    break;
  case 2:
    op = MODULO;
    break;
  case 3:
    op = POTENCIA;
    break;
  case 4:
    op = AND;
    break;
  case 5:
    op = OR;
    break;
  case 6:
    op = POTENCIA2;
    break;
  case 7:
    op = MENOR;
    break;
  case 8:
    op = MENOR_IGUAL;
    break;
  case 9:
    op = MAYOR;
    break;
  case 10:
    op = MAYOR_IGUAL;
    break;
  case 11:
    op = IGUAL;
    break;
  case 12:
    op = AMP;
    break;
  case 13:
    op = BARR;
    break;
  case 14:
    op = MENOSMENOS;
    break;
  case 15:
    op = DIFERENTE;
    break;
  case 16:
    op = SUMA;
    break;
  case 17:
    op = RESTA;
    break;
  }
  crear_cuadruplo(op, expr1, expr2, result);
  expresiones[expresionesCont - 2] = nombre;
  expresionesCont--;
}

void INTER_asignacion(char *nombre)
{
  Direccion expr1;
  expr1.tipo_dir = REF_LABEL;
  Direccion resultado;
  Op o;
  if (strcmp(expresiones[expresionesCont - 1], ".") == 0) {
    expresionesCont--;
    o = LLENAR_LISTA;
    resultado = crear_direccion_nula();
  } else {
    resultado.tipo_dir = REF_LABEL;
    resultado.valor.etiqueta_id = nombre;
    o = ASIGNACION;
  }

  expr1.valor.etiqueta_id = expresiones[expresionesCont - 1];

  expresionesCont--;
  
  crear_cuadruplo(o, expr1, crear_direccion_nula(), resultado);

}

void INTER_salto_condicional(char *etiqueta)
{
  Direccion etiq;
  etiq.tipo_dir = REF_LABEL;
  etiq.valor.etiqueta_id = etiqueta;

  // Extraer la última expresión guardada
  char *id_cond = expresiones[expresionesCont - 1];
  expresionesCont--;

  Direccion cond;
  cond.tipo_dir = REF_LABEL;
  cond.valor.etiqueta_id = id_cond;

  /* La operación NOT seguida de salto condicional */
  crear_cuadruplo(IFGOTO, cond, etiq, crear_direccion_nula());
}

void INTER_etiqueta(char *etiq)
{
  Direccion d;
  d.tipo_dir = REF_LABEL;
  d.valor.etiqueta_id = etiq;
  crear_cuadruplo(ETIQUETA, d, crear_direccion_nula(), crear_direccion_nula());
}

void INTER_goto(char *etiq)
{
  Direccion d;
  d.tipo_dir = REF_LABEL;
  d.valor.etiqueta_id = etiq;
  crear_cuadruplo(GOTO, d, crear_direccion_nula(), crear_direccion_nula());
}

void INTER_finproc()
{
  crear_cuadruplo(FIN_PROC, crear_direccion_nula(), crear_direccion_nula(),
                  crear_direccion_nula());
}

void INTER_entrada(char *id, dtipo tipo)
{
  Direccion res;
  res.tipo_dir = REF_LABEL;
  res.valor.etiqueta_id = id;
  Direccion t;
  t.tipo_dir = REF_TIPO;
  t.valor.tipo = tipo;
  crear_cuadruplo(ENTRADA, t, crear_direccion_nula(), res);
}

void INTER_salida(dtipo tipoExpresion)
{
  char *nombre_expresion = expresiones[expresionesCont - 1];
  Direccion expr_dir;
  expr_dir.tipo_dir = REF_LABEL;
  expr_dir.valor.etiqueta_id = strdup(nombre_expresion);
  Direccion tipo;
  tipo.tipo_dir = REF_TIPO;
  tipo.valor.tipo = tipoExpresion;
  crear_cuadruplo(SALIDA, expr_dir, tipo,
                  crear_direccion_nula());
  expresionesCont--;
}

void INTER_llamada_proc(char *id)
{
  int key = TS_BuscarIDENTKey(id);

  if (key != -1)
  {
    entradaTS *ent = &TS[key];
    int num_params = ent->parametros;
    char buffer[1024] = "";

    for (int i = 0; i < num_params; i++)
    {
      if (i > 0)
        strcat(buffer, ", ");

      char *arg_real = expresiones[expresionesCont - 1 - i];
      strcat(buffer, arg_real);
    }
    expresionesCont -= num_params;
    Direccion proc_dir;
    proc_dir.tipo_dir = REF_LABEL;
    proc_dir.valor.etiqueta_id = id;
    Direccion args_dir;
    args_dir.tipo_dir = REF_LABEL;
    args_dir.valor.etiqueta_id = strdup(buffer);

    crear_cuadruplo(LLAMADA_PROC, proc_dir, args_dir, crear_direccion_nula());
  }
}

void verificarAcceso(char *nombre) {
    int key = TS_BuscarIDENTKey(nombre);
    if (key == -1) return;

    int inicioProcActual = -1;
    for (int i = TOPE; i >= 1; i--) {
        if (TS[i].entrada == MARCA && TS[i-1].entrada == PROCEDIMIENTO) {
            inicioProcActual = i;
            break;
        }
    }

    if (inicioProcActual == -1) return;

    if (key < inicioProcActual) {
        if (TS[key].entrada != PROCEDIMIENTO) {
            fprintf(stderr,
            "Error Semántico (Línea %d): '%s' no está declarado en el ámbito actual.\n",
            yylineno, nombre);
            num_errores_semanticos++;
        }
    }
}

void INTER_initCursor(char *id)
{
  Direccion var;
  var.tipo_dir = REF_LABEL;
  var.valor.etiqueta_id = id;
  crear_cuadruplo(INIT_CURSOR, var, crear_direccion_nula(),
                  crear_direccion_nula());
}

void INTER_avanzarCursor(char *id)
{
  Direccion var;
  var.tipo_dir = REF_LABEL;
  var.valor.etiqueta_id = id;
  crear_cuadruplo(AVANZAR_CURSOR, var, crear_direccion_nula(),
                  crear_direccion_nula());
}

void INTER_retrocederCursor(char *id)
{
  Direccion var;
  var.tipo_dir = REF_LABEL;
  var.valor.etiqueta_id = id;
  crear_cuadruplo(RETROCEDER_CURSOR, var, crear_direccion_nula(),
                  crear_direccion_nula());
}

void INTER_sumrest_lista(int op, dtipo tipo1)
{
  Direccion expr1; expr1.tipo_dir = REF_LABEL;
  Direccion expr2; expr2.tipo_dir = REF_LABEL;

  if (esLista(tipo1)) {
      // Caso: Lista + Valor (La lista está en top-2)
      expr1.valor.etiqueta_id = expresiones[expresionesCont - 2];
      expr2.valor.etiqueta_id = expresiones[expresionesCont - 1];
  } else {
      // Caso: Valor + Lista (La lista está en top-1)
      expr1.valor.etiqueta_id = expresiones[expresionesCont - 1]; 
      expr2.valor.etiqueta_id = expresiones[expresionesCont - 2]; 
  }

  Op o = (op == 0) ? LISTA_SUMA : LISTA_RESTA;
  expresionesCont -= 2;
  crear_cuadruplo(o, expr1, expr2, crear_direccion_nula());
}

void INTER_multdiv_lista(int op, dtipo tipo1)
{
  Direccion expr1; expr1.tipo_dir = REF_LABEL;
  Direccion expr2; expr2.tipo_dir = REF_LABEL;

  if (esLista(tipo1)) {
      expr1.valor.etiqueta_id = expresiones[expresionesCont - 2]; // Lista
      expr2.valor.etiqueta_id = expresiones[expresionesCont - 1]; // Valor
  } else {
      expr1.valor.etiqueta_id = expresiones[expresionesCont - 1]; // Lista
      expr2.valor.etiqueta_id = expresiones[expresionesCont - 2]; // Valor
  }

  Op o = (op == 0) ? LISTA_MUL : LISTA_DIV;
  expresionesCont -= 2;
  crear_cuadruplo(o, expr1, expr2, crear_direccion_nula());
}

void INTER_LlaveIzqAntesExpr() {
  int pos = -1;
  for (int i = numCuadruplos-1; i >= 0; i--) {
    if (cuadruplos[i].op == DECLARACION && strcmp(cuadruplos[i].arg2.valor.etiqueta_id, "temp0") == 0) {
      pos = i;
      break;
    } else if (cuadruplos[i].op == LLAVE_DER) {
      break;
    }
  }

  if (pos == -1) {
    return;
  }

  for (int i = numCuadruplos - 1; i >= pos; i--) {
    cuadruplos[i + 1] = cuadruplos[i];
  }

  numCuadruplos++;

  cuadruplos[pos].op = LLAVE_IZQ;
  cuadruplos[pos].arg1 = crear_direccion_nula();
  cuadruplos[pos].arg2 = crear_direccion_nula();
  cuadruplos[pos].resultado = crear_direccion_nula();

}

void INTER_declLista(dtipo tipo) {
  Direccion result;
  result.tipo_dir = REF_LABEL;

  // CASO A: Literal [1,2,3] (Marcador ".")
  if (expresionesCont > 0 && expresiones[expresionesCont-1] != NULL && strcmp(expresiones[expresionesCont-1], ".") == 0) {
      
      Direccion expr1; expr1.tipo_dir = REF_LABEL;
      Direccion cantd; cantd.tipo_dir = REF_LABEL;
      
      expresionesCont--; // Fuera "."
      cantd.valor.etiqueta_id = expresiones[expresionesCont-1]; // Cantidad
      expresionesCont--;
      expr1.valor.etiqueta_id = expresiones[expresionesCont-1]; // String "1, 2"
      
      // Reutilizamos el hueco para el nombre del temporal
      char *nombre = generar_nombre_temporal();
      INTER_decl_var(nombre, tipo);
      
      result.valor.etiqueta_id = nombre;
      expresiones[expresionesCont-1] = nombre; // Pila actualizada

      Op o;
      if (tipo == LISTA_ENTERO || tipo == LISTA_BOOLEANO) o = LLENAR_INTBOOL;
      else if (tipo == LISTA_REAL) o = LLENAR_REAL;
      else if (tipo == LISTA_CARACTER) o = LLENAR_STRING;
      else o = LLENAR_INTBOOL;

      crear_cuadruplo(o, expr1, cantd, result);

  } 
  // CASO B: Variable temporal normal
  else {
      char *nombre = generar_nombre_temporal();
      INTER_decl_var(nombre, tipo);
      
      // Empujar a la pila
      expresiones[expresionesCont] = nombre;
      expresionesCont++;
  }
}
