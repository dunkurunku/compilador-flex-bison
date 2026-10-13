#include "TS.h"

// Definición de las variables globales declaradas en TS.h
entradaTS TS[MAX_TS];
long int TOPE;
dtipo tipoActual;
unsigned int numParametros;

void TS_Inicializar() {
  TOPE = -1;
  numParametros = 0;
}

void TS_InsertaMARCA() {
  if (TOPE >= MAX_TS - 1) {
    fprintf(stderr, "Error: Desbordamiento de la Tabla de Símbolos\n");
    exit(1);
  }
  TOPE++;
  TS[TOPE].entrada = MARCA;
  TS[TOPE].nombre = "{";

  if (TOPE > 0) {
    TS_copiarParams();
  }
}

void TS_copiarParams() {
  int numParams = 0;
  int i = TOPE;
  i--;
  if (TS[i].entrada == PROCEDIMIENTO) {
    numParams = i - 1 - TS[i].parametros;
    i--;
  } else
    return;

  for (; i > numParams; i--) {
    if (TS[i].entrada == PARAMETRO_FORMAL) {
      TS_InsertaIDENT(TS[i].nombre, TS[i].tipoDato, VARIABLE);
    }
  }
}

void TS_VaciarENTRADAS() {
  // Desapilar hasta encontrar una MARCA
  while (TOPE >= 0 && TS[TOPE].entrada != MARCA) {
    // DEBUG // printf("--> TS: Eliminando %s\n", TS[TOPE].nombre);
    TOPE--;
  }
  // Eliminar también la marca
  if (TOPE >= 0 && TS[TOPE].entrada == MARCA) {
    TOPE--;
    // DEBUG // printf("--> TS: Cierre de bloque procesado. Tope actual: %ld\n",
    // TOPE);
  }
}

int TS_InsertaIDENT(char *nombre, dtipo tipo, tipoEntrada entrada) {
  entradaTS *simbolo = TS_BuscarIDENTLocal(nombre);
  if (simbolo != NULL) {
    fprintf(stderr,
            "Error Semántico (Línea %d): '%s' ya declarado en este "
            "contexto.\n",
            yylineno, nombre);
    num_errores_semanticos++;
    return -1;
  }

  if (TOPE >= MAX_TS - 1) {
    fprintf(stderr, "Error: Desbordamiento de la Tabla de Símbolos\n");
    exit(1);
  }

  TOPE++;

  TS[TOPE].entrada = entrada;
  TS[TOPE].nombre = strdup(nombre);
  TS[TOPE].tipoDato = tipo;

  if (entrada == PROCEDIMIENTO) {
    TS[TOPE].parametros = numParametros;
    numParametros = 0;
  } else if (entrada == PARAMETRO_FORMAL) {
    numParametros++;
  }

  return TOPE;
}

entradaTS *TS_BuscarIDENT(char *nombre) {
  // Búsqueda desde el tope hacia abajo (ámbito más local a más global)
  for (long int i = TOPE; i >= 0; i--) {
    if (TS[i].entrada != MARCA && TS[i].entrada != PARAMETRO_FORMAL &&
        strcmp(TS[i].nombre, nombre) == 0) {
      return &TS[i];
    }
  }
  return NULL;
}

// Específicamente para InsertarIDENT, así permite usar verificarAcceso(tipo de error más específico)
entradaTS *TS_BuscarIDENTLocal(char *nombre) {
  // Búsqueda desde el tope hacia abajo (ámbito más local a más global)
  for (long int i = TOPE; i >= 1; i--) {
    if (TS[i].entrada == MARCA && TS[i-1].entrada == PROCEDIMIENTO) {
      return NULL;  // Límite del procedimiento actual
    }
    if (TS[i].entrada != PARAMETRO_FORMAL && TS[i].entrada != MARCA &&
        strcmp(TS[i].nombre, nombre) == 0) {
      return &TS[i];
    }
  }
  return NULL;
}

int TS_BuscarIDENTKey(char *nombre) {
  // Búsqueda desde el tope hacia abajo (ámbito más local a más global)
  for (long int i = TOPE; i >= 0; i--) {
    if (TS[i].entrada != MARCA && TS[i].entrada != PARAMETRO_FORMAL &&
        strcmp(TS[i].nombre, nombre) == 0) {
      return i;
    }
  }
  return -1;
}

const char *dtipo_to_string(dtipo tipo) {
  switch (tipo) {
  case ENTERO:
    return "int";
  case REAL:
    return "float";
  case CARACTER:
    return "char";
  case BOOLEANO:
    return "bool";
  case LISTA_ENTERO:
    return "list int";
  case LISTA_REAL:
    return "list float";
  case LISTA_CARACTER:
    return "list char";
  case LISTA_BOOLEANO:
    return "list bool";
  case STRING:
    return "char*";
  case DESCONOCIDO:
    return "desconocido";
  case NO_ASIGNADO:
    return "no asignado";
  default:
    return "desconocido";
  }
}

const char *dtipo_to_enum_string(dtipo tipo) {
  switch (tipo) {
  case ENTERO:
    return "ENTERO";
  case REAL:
    return "REAL";
  case CARACTER:
    return "CARACTER";
  case BOOLEANO:
    return "BOOLEANO";
  case LISTA_ENTERO:
    return "LISTA_ENTERO";
  case LISTA_REAL:
    return "LISTA_REAL";
  case LISTA_CARACTER:
    return "LISTA_CARACTER";
  case LISTA_BOOLEANO:
    return "LISTA_BOOLEANO";
  case STRING:
    return "STRING";
  case DESCONOCIDO:
    return "DESCONOCIDO";
  case NO_ASIGNADO:
    return "NO_ASIGNADO";
  default:
    return "DESCONOCIDO";
  }
}

void TS_Mostrar() {
  printf("\n--- CONTENIDO TABLA SIMBOLOS (Tope: %ld) ---\n", TOPE);
  for (long int i = TOPE; i >= 0; i--) {
    if (TS[i].entrada == MARCA) {
      printf("%ld: [ MARCA ]\n", i);
    } else if (TS[i].entrada == PROCEDIMIENTO) {
      printf("%ld: [ PROCEDIMIENTO | %s | Parametros: %u ]\n", i, TS[i].nombre,
             TS[i].parametros);
    } else if (TS[i].entrada == PARAMETRO_FORMAL) {
      printf("%ld: [ PARAMETRO_FORMAL | %s | Tipo: %s ]\n", i, TS[i].nombre,
             dtipo_to_string(TS[i].tipoDato));
    } else {
      printf("%ld: [ VARIABLE | %s | Tipo: %s ]\n", i, TS[i].nombre,
             dtipo_to_string(TS[i].tipoDato));
    }
  }
  printf("------------------------------------------\n\n");
}

void TS_InsertaMARCA2(char *etiq_entrada, char *etiq_salida, 
                            char *etiq_else, char *nombre_var_control) {
  /* Paso 1: Insertar la MARCA */
  //TS_InsertaMARCA();
  
  /* Paso 2: Obtener el descriptor actual */
  DescriptorDeInstrControl *desc = obtener_descriptor_actual();
  
  /* Paso 3: Asignar las etiquetas */
  desc->EtiquetaEntrada = etiq_entrada;
  desc->EtiquetaSalida = etiq_salida;
  desc->EtiquetaElse = etiq_else;
  desc->NombreVarControl = nombre_var_control;
}

/* Obtener el descriptor de la MARCA más reciente en la TS */
DescriptorDeInstrControl *obtener_descriptor_actual() {
  /* Buscar hacia atrás desde TOPE hasta encontrar una MARCA */
  for (int i = TOPE; i >= 0; i--) {
    if (TS[i].entrada == MARCA) {
      return &(TS[i].descCtrl);
    }
  }
  fprintf(stderr, "Error: No hay instrucción de control activa\n");
  return NULL;
}