%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "TS.h"
#include "SemanticoGest.h"

extern int yylex(void);
void yyerror(const char *s);
int yydebug = 1;
%}

%define parse.error verbose

%token ID 
%token UNOP BINOP OPSUMRESTA 
%token TIPO LITERAL
%token COND CICLO REPEAT UNTIL ELSE IN OUT
%token ASIGN OPARR OPMASMAS
%token SHIFT_DER SHIFT_IZQ DOLLAR
%token PARIZQ PARDER CORCHIZQ CORCHDER LLAVIZQ LLAVDER
%token COMA PYC
%token VARIZQ VARDER INIT FINPROC LIST
%token PROC

%left OPSUMRESTA
%left BINOP
%left OPARR
%left OPMASMAS
%right UNOP      
%left PARIZQ

%start Programa

%%

/* ================= REGLAS GRAMATICALES ================= */

Programa
    : Cabecera_programa Bloque_principal
    ;

Cabecera_programa
    : INIT ID {
        id_init = $2.lexema;
    }
    ;

Bloque_principal
    : Declar_de_subprogs
    ;

Declar_de_subprogs
    : Declar_de_subprogs Declar_subprog
    | Declar_subprog
    ;

Declar_subprog
    : Cabecera_subprograma Bloque {INTER_finproc();}
    ;

Cabecera_subprograma
    : PROC ID PARIZQ Parametros PARDER {
    	int TSKey = TS_InsertaIDENT($2.lexema, DESCONOCIDO, PROCEDIMIENTO);
        generar_cabecera_funcion(TSKey);
    }
    ;

Parametros
    : Parametro COMA Parametros
    | Parametro
    | /* vacío */
    ;

Parametro
    : TIPO ID { 
        TS_InsertaIDENT($2.lexema, $1.tipo, PARAMETRO_FORMAL);
    }
    | error ID
    ;

/* GESTIÓN DE BLOQUES */
Bloque
    : LLAVIZQ { TS_InsertaMARCA(); INTER_Llaveizq();} 
      Declar_de_variables_locales Sentencias LLAVDER { 
        //TS_Mostrar();       
        TS_VaciarENTRADAS();
        //TS_Mostrar();
        INTER_Llaveder();
      }
      
    ;

Declar_de_variables_locales
    : VARIZQ Variables_locales VARDER
    | VARIZQ VARDER
    | /* vacío */
    ;

Variables_locales
    : Cuerpo_declar_variables Variables_locales
    | Cuerpo_declar_variables
    ;

Cuerpo_declar_variables
    : TIPO { tipoActual = $1.tipo; } Idents PYC
    | LIST TIPO { 
        /* Convertimos el tipo base al tipo lista correspondiente */
        switch($2.tipo) {
            case ENTERO: tipoActual = LISTA_ENTERO; break;
            case REAL: tipoActual = LISTA_REAL; break;
            case CARACTER: tipoActual = LISTA_CARACTER; break;
            case BOOLEANO: tipoActual = LISTA_BOOLEANO; break;
            default: tipoActual = DESCONOCIDO; break;
        }
      } Idents PYC
    | error
    ;

Idents
    : ID {TS_InsertaIDENT(strdup($1.lexema), tipoActual, VARIABLE);
        INTER_decl_var($1.lexema, tipoActual);
     } COMA Idents
    | ID error Idents
    | ID {TS_InsertaIDENT(strdup($1.lexema), tipoActual, VARIABLE);
        INTER_decl_var($1.lexema, tipoActual);}
    ;

Sentencias
    : Sentencias Sentencia_pre
    | Sentencia_pre
    ;

Sentencia_pre
    : Sentencia {contador_temp = 0; expresionesCont = 0;}

Sentencia
    : Bloque 
    | Sentencia_asignacion
    | Sentencia_if
    | Sentencia_while
    | Sentencia_repeat
    | Sentencia_entrada
    | Sentencia_salida
    | Llamada_proced PYC
    | Sentencia_avanzar
    | Sentencia_retroceder
    | Sentencia_init_cursor
    | Declar_subprog
    | Sentencia_sumrest_lista
    | Sentencia_multdiv_lista
    ;

Sentencia_asignacion
    : ID ASIGN {INTER_Llaveizq();} Expresion PYC {
        verificarAcceso($1.lexema);
        asignacionTipo($1.lexema, $4.tipo);
        INTER_asignacion($1.lexema);
        INTER_Llaveder();
    }
    ;

Sentencia_if
    : Cabecera_if {
        INTER_Llaveder();
        DescriptorDeInstrControl *desc = obtener_descriptor_actual();
        INTER_etiqueta(desc->EtiquetaSalida);
    }
    | Cabecera_if ELSE {
            DescriptorDeInstrControl *desc = obtener_descriptor_actual();
            INTER_goto(desc->EtiquetaElse);
            INTER_etiqueta(desc->EtiquetaSalida);
        }
        Bloque ELSE FINPROC {
            INTER_Llaveder();
            DescriptorDeInstrControl *desc = obtener_descriptor_actual();
            INTER_etiqueta(desc->EtiquetaElse);}
    ;
Cabecera_if
    : COND {INTER_Llaveizq();} Expresion {
            verificarCondicion($3.tipo, "IF");
            char *etiq_salida = generar_nombre_etiqueta();
            char *etiq_else = generar_nombre_etiqueta();
            TS_InsertaMARCA2(NULL, etiq_salida, etiq_else, NULL);
            INTER_salto_condicional(etiq_salida);
            } Bloque COND FINPROC
    ;

Sentencia_while
    : CICLO { 
        /* Paso 1: Generar etiquetas */
        char *etiq_entrada = generar_nombre_etiqueta();
        char *etiq_salida = generar_nombre_etiqueta();

        /* Paso 2: Insertar en la TS */
        TS_InsertaMARCA2(etiq_entrada, etiq_salida, NULL, NULL);
        
        /* Paso 3: Situar etiqueta de entrada en código objeto */
        INTER_etiqueta(etiq_entrada);

        // Inicio de bucle (después de etiqueta de entrada)
        INTER_Llaveizq();
        
      } Expresion { 
        // Semántico
        verificarCondicion($3.tipo, "WHILE");

        /* Paso 4: La expresión se procesa en Expresion {} */

        /* Paso 5: Emitir salto condicional a etiqueta de salida */
        DescriptorDeInstrControl *desc = obtener_descriptor_actual();
        INTER_salto_condicional(desc->EtiquetaSalida);
      } Bloque { 
        /* Paso 6: Cuerpo se proceso en Bloque {} */
        DescriptorDeInstrControl *desc = obtener_descriptor_actual();
        
        /* Paso 7: Emitir salto incondicional a etiqueta de entrada */
        INTER_goto(desc->EtiquetaEntrada);
      } CICLO FINPROC {
        // Fin de bucle (antes de etiqueta de salida)
        INTER_Llaveder();

        /* Paso 8: Emitir etiqueta de salida */
        DescriptorDeInstrControl *desc = obtener_descriptor_actual();
        INTER_etiqueta(desc->EtiquetaSalida);
        
        /* Paso 9: Eliminar entradas en TS hasta descriptor de control */
        //TS_VaciarENTRADAS(); tambien saque le insercion en TS_Insertar
      }
    ;

Sentencia_repeat
    : REPEAT {
        char *etiq_entrada = generar_nombre_etiqueta();
        TS_InsertaMARCA2(etiq_entrada, NULL, NULL, NULL);
        INTER_etiqueta(etiq_entrada);
    } Bloque REPEAT FINPROC UNTIL {INTER_Llaveizq();} Expresion PYC
    {
        verificarCondicion($8.tipo, "REPEAT");
        DescriptorDeInstrControl *desc = obtener_descriptor_actual();
        INTER_salto_condicional(desc->EtiquetaEntrada);
        INTER_Llaveder();
    }
    ;

Sentencia_entrada
    : IN SHIFT_DER ID PYC {
        verificarAcceso($3.lexema);
        dtipo t = comprobarEntrada($3.lexema);
        INTER_entrada($3.lexema, t);
    }
    ;

Sentencia_salida
    : OUT SHIFT_IZQ Cuerpo_salida PYC
    ;

Cuerpo_salida
    : Expresion { comprobarSalida($1.tipo); INTER_salida($1.tipo);}
    | Expresion { comprobarSalida($1.tipo); INTER_salida($1.tipo);} SHIFT_IZQ Cuerpo_salida
    ;

Llamada_proced
    : ID PARIZQ { INTER_Llaveizq(); } Argumentos PARDER {
        comprobarLlamada($1.lexema);   
        INTER_llamada_proc($1.lexema); 
        INTER_Llaveder();              
    }
    | ID PARIZQ PARDER {
        /* Aquí no hacen falta llaves extra porque no hay argumentos que generen temporales */
        comprobarLlamada($1.lexema);
        INTER_llamada_proc($1.lexema);
    }
    ;

/* Modificado para propagar tipos y verificar homogeneidad en listas */
Argumentos
    : Expresion {
        $$.tipo = $1.tipo;
        contarArgumentos($1.tipo);
    }
    | Expresion COMA Argumentos {
        if ($1.tipo == $3.tipo) {
            $$.tipo = $1.tipo;
            contarArgumentos($1.tipo);
        } else {
            /* Si son tipos distintos, marcamos como error, pero propagamos 
               DESCONOCIDO para evitar cascada de errores */
            fprintf(stderr, "Error Semántico (Línea %d): Los elementos de la lista deben ser del mismo tipo.\n", yylineno);
            $$.tipo = DESCONOCIDO;
        }
    }
    ;

Sentencia_avanzar
    : ID SHIFT_DER PYC {comprobarDesplaza($1.lexema); INTER_avanzarCursor($2.lexema);}
    ;

Sentencia_retroceder
    : ID SHIFT_IZQ PYC {comprobarDesplaza($1.lexema); INTER_retrocederCursor($2.lexema);}
    ;

Sentencia_init_cursor
    : DOLLAR ID PYC {comprobarInitCursor($2.lexema); INTER_initCursor($2.lexema);}
    ;

Sentencia_sumrest_lista
    : Expresion OPSUMRESTA Expresion PYC {
        $$.tipo = comprobarAritmeticaLista($1.tipo, $2.atrib, $3.tipo);

        /* Identificar cuál es el escalar (el número) */
        char *valStr;
        int idx; // Índice en la pila
        dtipo tipoBase;

        if (esLista($1.tipo)) { // Caso: Lista + Valor
             idx = expresionesCont - 1; 
             tipoBase = obtenerTipoBase($1.tipo);
        } else { // Caso: Valor + Lista
             idx = expresionesCont - 2;
             tipoBase = obtenerTipoBase($3.tipo);
        }

        valStr = expresiones[idx];
        char valModificado[100];

        /* Si es RESTA, invertimos el valor nosotros y usamos SUMA */
        if ($2.atrib == 1) { // 1 suele ser RESTA
             sprintf(valModificado, "-(%s)", valStr);
        } else {
             strcpy(valModificado, valStr);
        }

        /* Envolver en puntero: (int[]){ valor } */
        char buffer[256];
        if (tipoBase == ENTERO) sprintf(buffer, "(int[]){%s}", valModificado);
        else if (tipoBase == REAL) sprintf(buffer, "(float[]){%s}", valModificado);

        /* Actualizamos la pila */
        expresiones[idx] = strdup(buffer);

        /* 4. Llamamos siempre con 0 (SUMA) porque ya manejamos el signo arriba */
        INTER_sumrest_lista(0, $1.tipo);
    }
    ;

Sentencia_multdiv_lista
    : Expresion BINOP Expresion PYC {
        $$.tipo = comprobarAritmeticaLista($1.tipo, $2.atrib, $3.tipo);
        
        char *valStr;
        int idx;
        dtipo tipoBase;
        
        if (esLista($1.tipo)) { 
             idx = expresionesCont - 1; 
             tipoBase = obtenerTipoBase($1.tipo);
        } else { 
             idx = expresionesCont - 2;
             tipoBase = obtenerTipoBase($3.tipo);
        }

        valStr = expresiones[idx];
        char valModificado[100];

        /* Si es DIVISION, convertimos a multiplicacion por inverso: 1.0/(valor) */
        if ($2.atrib == 1) { 
             if (tipoBase == REAL) sprintf(valModificado, "1.0/(%s)", valStr);
             else sprintf(valModificado, "1/(%s)", valStr);
        } else {
             strcpy(valModificado, valStr);
        }

        /* Envolver en puntero temporal */
        char buffer[256];
        if (tipoBase == ENTERO) sprintf(buffer, "(int[]){%s}", valModificado);
        else if (tipoBase == REAL) sprintf(buffer, "(float[]){%s}", valModificado);
        
        expresiones[idx] = strdup(buffer);

        /* Llamamos a multiplicacion (op 0) siempre */
        INTER_multdiv_lista(0, $1.tipo);
    }
    ;



Expresion
    : PARIZQ Expresion PARDER { $$.tipo = $2.tipo; }
    
    | UNOP Expresion {
        $$.tipo = comprobarOperacionUnaria($1.atrib, $2.tipo);
        
        if (!esLista($2.tipo)) {
            INTER_unop(0, $1.atrib, $2.tipo); // Solo para enteros/bools normales
        } 
        else {
            // Para listas NO declaramos variable, solo string
            char buffer[200];
            char *cast = "";
            dtipo base = obtenerTipoBase($2.tipo);
            
            if (base == ENTERO || base == BOOLEANO) cast = "*(int*)";
            else if (base == REAL) cast = "*(float*)";
            else if (base == CARACTER) cast = "(char*)";

            if ($1.atrib == 1) // # Longitud
                snprintf(buffer, sizeof(buffer), "obtenerLongitud(%s)", expresiones[expresionesCont-1]);
            else if ($1.atrib == 2) // ? Cursor
                snprintf(buffer, sizeof(buffer), "%s obtenerDatoCursor(%s)", cast, expresiones[expresionesCont-1]);
            
            expresiones[expresionesCont-1] = strdup(buffer);
        }
    }
    
    | OPSUMRESTA Expresion {
        /* Comprobamos si es aritmética de listas o normal */
        if (esTipoLista($2.tipo)) {
             $$.tipo = comprobarAritmeticaLista($2.tipo, 1, $2.tipo); 
        } 
        $$.tipo = comprobarSignoUnario($2.tipo);
        INTER_unop(1, $1.atrib, $2.tipo);
    }

    | Expresion BINOP Expresion {
        /* Operaciones binarias generales */
        if (esTipoLista($1.tipo) || esTipoLista($3.tipo)) {
            $$.tipo = comprobarListaBinaria($1.tipo, $2.atrib, $3.tipo);
            
            // Guardar operandos
            char *op2Str = expresiones[expresionesCont-1];
            char *op1Str = expresiones[expresionesCont-2];

            // Declarar temporal
            // INTER_declLista($$.tipo);

            // Generar código con variables guardadas
            char buffer[100];
            if ($2.atrib == 14) // --
                snprintf(buffer, sizeof(buffer), "DupBorrPos(%s, %s)", op1Str, op2Str);
            else if ($2.atrib == 2) // %
                snprintf(buffer, sizeof(buffer), "DupBorrResto(%s, %s)", op1Str, op2Str);
            else if ($2.atrib == 3) // **
                snprintf(buffer, sizeof(buffer), "DupConcat(%s, %s)", op1Str, op2Str);
            
            // Ajustar pila
            expresionesCont -= 2; // Quitamos operandos
            expresiones[expresionesCont] = strdup(buffer);
            expresionesCont++; // Dejamos el resultado
        }
        else {
             $$.tipo = comprobarOperacionBinaria($1.tipo, $2.tipo, $3.tipo);
             INTER_binop($2.atrib, $1.tipo, $3.tipo);
        }
    }

    | Expresion OPSUMRESTA Expresion {
            if ($1.tipo == $3.tipo && ($1.tipo == ENTERO || $1.tipo == REAL)) {
                $$.tipo = $1.tipo;
                INTER_binop($2.atrib+16, $1.tipo, $3.tipo);
            } else {
                fprintf(stderr, "Error Semántico (Línea %d): Tipos incompatibles en suma/resta.\n", yylineno);
                $$.tipo = DESCONOCIDO;
            }
    }

    /* Acceso a lista: lista @ entero */
    | Expresion OPARR Expresion { 
        $$.tipo = comprobarAccesoLista($1.tipo, $3.tipo);

        // Guardamos los operandos antes de tocar la pila
        char *posStr = expresiones[expresionesCont-1];
        char *listStr = expresiones[expresionesCont-2];

        // DECIDIMOS EL CAST (¡ESTO ES LO IMPORTANTE!)
        char *cast = "";
        if ($$.tipo == ENTERO || $$.tipo == BOOLEANO) cast = "*(int*)";
        else if ($$.tipo == REAL) cast = "*(float*)";
        else if ($$.tipo == CARACTER) cast = "(char*)"; 

        // Generamos el código C con el cast: *(int*)obtenerDato(...)
        char buffer[200];
        snprintf(buffer, sizeof(buffer), "%s obtenerDato(%s, %s)", cast, listStr, posStr);
        
        // Ajustamos la pila
        expresionesCont -= 2; // Quitamos lista y pos
        expresiones[expresionesCont] = strdup(buffer); // Dejamos el código generado
        expresionesCont++;
    }

    /* Inserción en lista: lista @ entero ++ valor */
    | Expresion OPARR Expresion OPMASMAS Expresion { 
        $$.tipo = comprobarInsercionLista($1.tipo, $3.tipo, $5.tipo);
        
        // Guardar operandos antes de que la pila cambie
        char *valStr = expresiones[expresionesCont-1];
        char *posStr = expresiones[expresionesCont-2];
        char *listStr = expresiones[expresionesCont-3];

        // Declarar la lista temporal (esto empuja un elemento a la pila)
        // INTER_declLista($1.tipo);

        // Crear el puntero al dato usando la variable guardada valStr
        char valArg[100];
        if ($5.tipo == ENTERO || $5.tipo == BOOLEANO) 
            sprintf(valArg, "(int[]){%s}", valStr);
        else if ($5.tipo == REAL) 
            sprintf(valArg, "(float[]){%s}", valStr);
        else 
            strcpy(valArg, valStr);

        // Generar la llamada C usando las variables guardadas (listStr, posStr)
        char buffer[200];
        snprintf(buffer, sizeof(buffer), "DupInsertPos(%s, %s, %s)", 
                 listStr,  // Lista
                 valArg,   // Dato
                 posStr    // Posición
        );
        
        // Limpiar la pila: quitamos el temp de INTER_declLista y los 3 operandos
        expresionesCont -= 3; // Quitamos los operandos originales
        expresiones[expresionesCont] = strdup(buffer);
        expresionesCont++;
    }
    
    | Llamada_proced { 
        $$.tipo = DESCONOCIDO;
    }
    
    | ID {
        entradaTS *p = TS_BuscarIDENT($1.lexema);
        if (p == NULL) {
            fprintf(stderr, "Error Semántico (Línea %d): Variable '%s' no declarada.\n",
            yylineno, $1.lexema);
            $$.tipo = DESCONOCIDO;
        } else {
            verificarAcceso($1.lexema);
            $$.tipo = p->tipoDato;
        }
        expresiones[expresionesCont] = $1.lexema;
        expresionesCont++;
    }
    
    | LITERAL {
        $$.tipo = $1.tipo;
        char cadena[20];
        switch($1.tipo) {
            case ENTERO:
                sprintf(cadena, "%d", $1.entero);
                expresiones[expresionesCont] = strdup(cadena);
                break;
            case REAL:
                sprintf(cadena, "%f", $1.flotante);
                expresiones[expresionesCont] = strdup(cadena);
                break;
            case STRING:
            case CARACTER:
                expresiones[expresionesCont] = strdup($1.str);
                break;
            case BOOLEANO:
                if ($1.booleano) {
                    expresiones[expresionesCont] = "true";
                } else {
                    expresiones[expresionesCont] = "false";
                }
                break;
        }
        expresionesCont++;
    }
    
    /* Literales de Lista: [1, 2, 3] */
    | Lista { 
        if ($1.tipo != DESCONOCIDO) {
            $$.tipo = obtenerTipoLista($1.tipo);
        } else {
            $$.tipo = DESCONOCIDO;
        }

        /* Recolectar elementos del stack */
        int cant = 0;
        char buffer0[1024]; // Buffer grande para evitar overflows
        buffer0[0] = '\0';
        int start = -1;
        
        // Buscamos el corchete de apertura hacia atrás
        for (int i = expresionesCont - 1; i >= 0; i--) {
          if (expresiones[i] != NULL && strcmp(expresiones[i], "[") == 0) {
            start = i;
            break; 
          }
        }
        
        if (start != -1) {
            // Concatenamos los elementos
            for (int i = start + 1; i < expresionesCont; i++) {
                if (expresiones[i] == NULL) continue;
                if (cant > 0) strcat(buffer0, ", ");
                strcat(buffer0, expresiones[i]);
                cant++;
            }
            
            // Reconstruimos la pila: [ corchete, string_elementos, cantidad, marcador ]
            expresionesCont = start; 
            expresiones[expresionesCont] = "["; // Dummy, se va a borrar
            expresionesCont++;
            
            expresiones[expresionesCont] = strdup(buffer0); // Elementos "1, 2, 3"
            expresionesCont++;
            
            char cadena[20];
            sprintf(cadena, "%d", cant);
            expresiones[expresionesCont] = strdup(cadena); // Cantidad
            expresionesCont++;
            
            expresiones[expresionesCont] = "."; // Marcador
            expresionesCont++;
            
            INTER_declLista($$.tipo);
        }
    }
    
    | error { $$.tipo = DESCONOCIDO; }
    ;

Lista
    : CORCHIZQ {
        expresionesCont++;
        expresiones[expresionesCont-1] = "[";
    } Argumentos CORCHDER {
        /* Propagamos el tipo base de los argumentos */
        $$.tipo = $3.tipo;
    }
    | CORCHIZQ CORCHDER {
        $$.tipo = DESCONOCIDO; 
    }
    ;

%%
