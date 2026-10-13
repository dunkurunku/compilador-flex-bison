#!/usr/bin/env bash
# Compila cada programa de prueba/ y compara el resultado con tests/expected/.
#   <nombre>.out  -> salida del ejecutable generado (si el programa es válido)
#   <nombre>.err  -> mensajes de error del compilador (si el programa es inválido)
# Para regenerar los resultados esperados: tests/run.sh --update
set -u
cd "$(dirname "$0")/.."
update=0; [ "${1:-}" = "--update" ] && update=1
make -s sintact >/dev/null || { echo "No compila"; exit 1; }
mkdir -p tests/.tmp
fallos=0
for src in prueba/*.txt; do
  nombre=$(basename "$src" .txt)
  rm -f salida/programa salida/main.c salida/dec_fun.h
  ./Semantico "$src" >tests/.tmp/compilador.out 2>&1
  if [ -x salida/programa ]; then
    entrada=/dev/null; [ -f "tests/input/$nombre.txt" ] && entrada="tests/input/$nombre.txt"
    ./salida/programa <"$entrada" >tests/.tmp/actual 2>&1
    esperado="tests/expected/$nombre.out"
  else
    grep -E "Error" tests/.tmp/compilador.out | sed 's/[[:space:]]*$//' >tests/.tmp/actual
    esperado="tests/expected/$nombre.err"
  fi
  if [ $update -eq 1 ]; then cp tests/.tmp/actual "$esperado"; echo "actualizado $esperado"; continue; fi
  if [ -f "$esperado" ] && diff -q "$esperado" tests/.tmp/actual >/dev/null; then
    echo "OK    $nombre"
  else
    echo "FALLO $nombre"; diff "$esperado" tests/.tmp/actual 2>&1 | head -20; fallos=$((fallos+1))
  fi
done
[ $fallos -eq 0 ] && echo "Todas las pruebas pasan." || { echo "$fallos prueba(s) fallan."; exit 1; }
