# Compilador con Flex y Bison

[![CI](https://github.com/dunkurunku/compilador-flex-bison/actions/workflows/ci.yml/badge.svg)](https://github.com/dunkurunku/compilador-flex-bison/actions/workflows/ci.yml)

Compilador completo para un pequeño lenguaje imperativo, escrito en C con Flex y Bison. Cubre las fases clásicas de un procesador de lenguajes:

1. **Análisis léxico** (`Lexico.lex`): tokens, literales, comentarios y detección de caracteres no válidos.
2. **Análisis sintáctico** (`Semantico.y`): gramática LALR(1) con recuperación de errores y mensajes con número de línea.
3. **Análisis semántico** (`SemanticoGest.c`, `TS.c`): tabla de símbolos con ámbitos anidados, comprobación de tipos en expresiones, condiciones, entrada/salida y llamadas a procedimientos.
4. **Generación de código intermedio**: cuádruplos (operación, argumento 1, argumento 2, resultado) con temporales y etiquetas.
5. **Generación de código final**: los cuádruplos se traducen a C (`salida/main.c`) y se compilan con `gcc` hasta obtener un ejecutable.

Proyecto de la asignatura Procesadores de Lenguajes del Grado en Ingeniería Informática (Universidad de Granada).

## El lenguaje fuente

```text
init:main

proc main() {
  <var>
    int n, curr, d;
  <var_>

  out << "introduce número : ";
  in >> n;
  curr = 2;

  while (curr <= n) {
    d = n / curr;
    if d * curr == n {
      out << "* " << curr << " ";
      n = n / curr;
    }if_ else {
      curr = curr + 1;
    }else_
  }while_

  out << "\n";
}
```

- **Tipos**: `int`, `float`, `char`, `bool`, `string` y listas homogéneas (`list int`, `list float`…).
- **Procedimientos** anidados con parámetros y ámbito léxico propio.
- **Control**: `if … if_ else … else_`, `while … while_`, `repeat … repeat_ until expr;`.
- **Entrada y salida**: `in >> variable`, `out << expr << expr`.
- **Operadores**: aritméticos, relacionales, lógicos, potencia (`**`) y desplazamientos.
- **Listas**: literal `[1, 2, 3]`, acceso `lista @ i`, inserción `lista @ i ++ valor`, borrado `lista -- i`, longitud `#lista`, aritmética elemento a elemento (`lista + 5`) y cursor (`$lista` inicia, `lista >>` avanza, `lista <<` retrocede, `?lista` lee).

Los programas de ejemplo están en [`prueba/`](prueba/).

## Compilar y usar

Requisitos: `gcc`, `flex` y `bison` 3 o superior.

```bash
make sintact                       # genera el compilador ./Semantico
./Semantico prueba/factorizacion2.txt
echo 360 | ./salida/programa       # ejecuta el programa compilado
```

El compilador escribe el C generado en `salida/main.c` y `salida/dec_fun.h`, lo compila con `gcc` y deja el ejecutable en `salida/programa`. Si el programa fuente tiene errores léxicos, sintácticos o semánticos, los lista con su línea y se detiene sin generar código:

```text
Error Semántico (Línea 9): Tipos incompatibles en suma/resta.
Error Semántico (Línea 10): Variable 'c' no declarada.
Error Semántico (Línea 11): La condición del IF debe ser bool.
```

En macOS el `bison` del sistema es la 2.3 y no reconoce `%define parse.error verbose`; instala uno moderno con `brew install bison` y úsalo en el `PATH`.

## Pruebas

```bash
make test
```

`tests/run.sh` compila cada programa de `prueba/`, ejecuta el binario resultante (con la entrada de `tests/input/` si la hay) y compara la salida con `tests/expected/`. Los programas inválidos se comparan por sus mensajes de error. El flujo de GitHub Actions ejecuta estas pruebas en cada push.

## Estructura

| Fichero | Contenido |
|---|---|
| `Lexico.lex` | Especificación Flex del analizador léxico |
| `Semantico.y` | Gramática Bison con las acciones semánticas y de generación |
| `TS.c`, `TS.h` | Tabla de símbolos con marcas de ámbito y descriptores de control |
| `SemanticoGest.c`, `SemanticoGest.h` | Comprobación de tipos y generación de cuádruplos y de C |
| `main.c` | Programa principal: abre el fichero, lanza el análisis y genera código |
| `salida/dec_dat.h` | Biblioteca de listas en C que enlaza el programa generado |
| `prueba/` | Programas de ejemplo, válidos e inválidos |
| `tests/` | Pruebas automáticas |

---

## English summary

A complete compiler for a small imperative language, written in C with Flex and Bison: lexer, LALR(1) parser with error recovery, semantic analysis with a scoped symbol table and type checking, intermediate code as quadruples, and final code generation to C (compiled with `gcc` into an executable). The language has integer, real, character, boolean and string types, homogeneous lists with cursor operations, nested procedures, `if`/`while`/`repeat` control flow and console I/O. Build with `make sintact`, run the test suite with `make test`. University project for the Language Processors course (Computer Engineering, University of Granada).
