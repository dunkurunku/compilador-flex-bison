#ifndef DEC_DAT_H
#define DEC_DAT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Tipos de datos soportados por la lista
typedef enum {
    ENTERO,
    REAL,
    CARACTER,
    BOOLEANO
} TipoDato;

// Nodo de la lista enlazada
typedef struct Nodo {
    void *dato;
    TipoDato tipo;
    struct Nodo *siguiente;
} Nodo;

// Estructura de la lista
typedef struct {
    Nodo *cabeza;
    Nodo *cursor;
    TipoDato tipo;
    int longitud;
} Lista;

// Crear una nueva lista vacía
static inline Lista* crearLista(TipoDato tipo) {
    Lista *lista = (Lista*)malloc(sizeof(Lista));
    if (lista == NULL) {
        return NULL;
    }
    lista->cabeza = NULL;
    lista->cursor = NULL;
    lista->tipo = tipo;
    lista->longitud = 0;
    return lista;
}

// Insertar un elemento al final de la lista
static inline void insertarLista(Lista *lista, void *dato) {
    if (lista == NULL) {
        return;
    }
    
    Nodo *nuevoNodo = (Nodo*)malloc(sizeof(Nodo));
    if (nuevoNodo == NULL) {
        return;
    }
    
    // Copiar el dato según el tipo
    switch (lista->tipo) {
        case ENTERO: {
            int *valorInt = (int*)malloc(sizeof(int));
            *valorInt = *(int*)dato;
            nuevoNodo->dato = valorInt;
            break;
        }
        case REAL: {
            float *valorFloat = (float*)malloc(sizeof(float));
            *valorFloat = *(float*)dato;
            nuevoNodo->dato = valorFloat;
            break;
        }
        case CARACTER: {
            char *str = (char*)dato;
            char *valorStr = (char*)malloc(strlen(str) + 1);
            strcpy(valorStr, str);
            nuevoNodo->dato = valorStr;
            break;
        }
        case BOOLEANO: {
            int *valorBool = (int*)malloc(sizeof(int));
            *valorBool = *(int*)dato;
            nuevoNodo->dato = valorBool;
            break;
        }
    }
    
    nuevoNodo->tipo = lista->tipo;
    nuevoNodo->siguiente = NULL;
    
    // Si la lista está vacía
    if (lista->cabeza == NULL) {
        lista->cabeza = nuevoNodo;
    } else {
        // Ir al final de la lista
        Nodo *actual = lista->cabeza;
        while (actual->siguiente != NULL) {
            actual = actual->siguiente;
        }
        actual->siguiente = nuevoNodo;
    }
    
    lista->longitud++;
}

// Liberar toda la memoria de la lista
static inline void liberarLista(Lista *lista) {
    if (lista == NULL) {
        return;
    }
    
    Nodo *actual = lista->cabeza;
    while (actual != NULL) {
        Nodo *siguiente = actual->siguiente;
        free(actual->dato);
        free(actual);
        actual = siguiente;
    }
    
    free(lista);
}

// Obtener un nodo por índice
static inline Nodo* obtenerNodo(Lista *lista, int indice) {
    if (lista == NULL || indice < 0 || indice >= lista->longitud) {
        return NULL;
    }
    
    Nodo *actual = lista->cabeza;
    for (int i = 0; i < indice; i++) {
        actual = actual->siguiente;
    }
    
    return actual;
}

// Obtener el dato de un nodo por índice
static inline void* obtenerDato(Lista *lista, int indice) {
    Nodo *nodo = obtenerNodo(lista, indice);
    if (nodo == NULL) {
        return NULL;
    }
    return nodo->dato;
}

// Imprimir la lista (para depuración)
static inline void imprimirLista(Lista *lista) {
    if (lista == NULL) {
        printf("Lista NULL\n");
        return;
    }
    
    printf("Lista [longitud=%d, tipo=", lista->longitud);
    switch (lista->tipo) {
        case ENTERO:
            printf("INT");
            break;
        case REAL:
            printf("REAL");
            break;
        case CARACTER:
            printf("CARACTER");
            break;
        case BOOLEANO:
            printf("BOOLEANO");
            break;
    }
    printf("]: ");
    
    Nodo *actual = lista->cabeza;
    while (actual != NULL) {
        switch (actual->tipo) {
            case ENTERO:
                printf("%d", *(int*)actual->dato);
                break;
            case REAL:
                printf("%.2f", *(float*)actual->dato);
                break;
            case CARACTER:
                printf("\"%s\"", (char*)actual->dato);
                break;
            case BOOLEANO:
                printf("%s", *(int*)actual->dato ? "true" : "false");
                break;
        }
        
        if (actual->siguiente != NULL) {
            printf(" -> ");
        }
        actual = actual->siguiente;
    }
    printf("\n");
}

// Mover el cursor al inicio de la lista
static inline void cursorInicio(Lista *lista) {
    if (lista == NULL) {
        return;
    }
    lista->cursor = lista->cabeza;
}

// Mover el cursor un nodo hacia adelante
static inline int cursorAdelante(Lista *lista) {
    if (lista == NULL || lista->cursor == NULL) {
        return 0; // No se pudo mover
    }
    if (lista->cursor->siguiente == NULL) {
        return 0; // Ya está en el final
    }
    lista->cursor = lista->cursor->siguiente;
    return 1; // Movimiento exitoso
}

// Mover el cursor un nodo hacia atrás
static inline int cursorAtras(Lista *lista) {
    if (lista == NULL || lista->cursor == NULL || lista->cursor == lista->cabeza) {
        return 0; // No se puede mover hacia atrás
    }
    
    // Buscar el nodo anterior al cursor actual
    Nodo *actual = lista->cabeza;
    while (actual != NULL && actual->siguiente != lista->cursor) {
        actual = actual->siguiente;
    }
    
    if (actual != NULL) {
        lista->cursor = actual;
        return 1; // Movimiento exitoso
    }
    
    return 0; // Error
}

// Obtener el dato en la posición del cursor
static inline void* obtenerDatoCursor(Lista *lista) {
    if (lista == NULL || lista->cursor == NULL) {
        return NULL;
    }
    return lista->cursor->dato;
}

// Obtener la longitud de la lista
static inline int obtenerLongitud(Lista *lista) {
    if (lista == NULL) {
        return 0;
    }
    return lista->longitud;
}

// Crear una nueva lista, añadir un elemento en una posición específica y devolverla
static inline Lista* DupInsertPos(Lista *listaOriginal, void *dato, int posicion) {
    if (listaOriginal == NULL) {
        return NULL;
    }
    
    // Crear nueva lista del mismo tipo
    Lista *nuevaLista = crearLista(listaOriginal->tipo);
    if (nuevaLista == NULL) {
        return NULL;
    }
    
    // Copiar todos los elementos de la lista original
    Nodo *actual = listaOriginal->cabeza;
    int indice = 0;
    
    while (actual != NULL) {
        // Si llegamos a la posición de inserción, insertar el nuevo dato primero
        if (indice == posicion) {
            insertarLista(nuevaLista, dato);
        }
        
        // Insertar el elemento actual de la lista original
        insertarLista(nuevaLista, actual->dato);
        
        actual = actual->siguiente;
        indice++;
    }
    
    // Si la posición es al final o más allá del final
    if (posicion >= listaOriginal->longitud) {
        insertarLista(nuevaLista, dato);
    }
    
    return nuevaLista;
}

// Crear una nueva lista con el elemento en la posición especificada borrado
static inline Lista* DupBorrPos(Lista *listaOriginal, int posicion) {
    if (listaOriginal == NULL || posicion < 0 || posicion >= listaOriginal->longitud) {
        return NULL;
    }
    
    // Crear nueva lista del mismo tipo
    Lista *nuevaLista = crearLista(listaOriginal->tipo);
    if (nuevaLista == NULL) {
        return NULL;
    }
    
    // Copiar todos los elementos excepto el de la posición especificada
    Nodo *actual = listaOriginal->cabeza;
    int indice = 0;
    
    while (actual != NULL) {
        // Solo insertar si no es la posición a borrar
        if (indice != posicion) {
            insertarLista(nuevaLista, actual->dato);
        }
        
        actual = actual->siguiente;
        indice++;
    }
    
    return nuevaLista;
}

// Crear una nueva lista sin los elementos a partir de la posición especificada
static inline Lista* DupBorrResto(Lista *listaOriginal, int posicion) {
    if (listaOriginal == NULL || posicion < 0) {
        return NULL;
    }
    
    // Crear nueva lista del mismo tipo
    Lista *nuevaLista = crearLista(listaOriginal->tipo);
    if (nuevaLista == NULL) {
        return NULL;
    }
    
    // Si la posición es 0, devolver lista vacía
    if (posicion == 0) {
        return nuevaLista;
    }
    
    // Copiar solo los elementos hasta la posición (sin incluir la posición)
    Nodo *actual = listaOriginal->cabeza;
    int indice = 0;
    
    while (actual != NULL && indice < posicion) {
        insertarLista(nuevaLista, actual->dato);
        actual = actual->siguiente;
        indice++;
    }
    
    return nuevaLista;
}

// Crear una nueva lista que concatena dos listas
static inline Lista* DupConcat(Lista *lista1, Lista *lista2) {
    if (lista1 == NULL || lista2 == NULL) {
        return NULL;
    }
    
    // Verificar que ambas listas sean del mismo tipo
    if (lista1->tipo != lista2->tipo) {
        return NULL;
    }
    
    // Crear nueva lista del mismo tipo
    Lista *nuevaLista = crearLista(lista1->tipo);
    if (nuevaLista == NULL) {
        return NULL;
    }
    
    // Copiar todos los elementos de la primera lista
    Nodo *actual = lista1->cabeza;
    while (actual != NULL) {
        insertarLista(nuevaLista, actual->dato);
        actual = actual->siguiente;
    }
    
    // Copiar todos los elementos de la segunda lista
    actual = lista2->cabeza;
    while (actual != NULL) {
        insertarLista(nuevaLista, actual->dato);
        actual = actual->siguiente;
    }
    
    return nuevaLista;
}

// Sumar un valor a todos los elementos de la lista (modifica la lista original)
static inline void SumaValor(Lista *lista, void *valor) {
    if (lista == NULL || valor == NULL) {
        return;
    }
    
    // Solo funciona con tipos numéricos
    if (lista->tipo != ENTERO && lista->tipo != REAL) {
        return;
    }
    
    // Recorrer y sumar el valor a cada elemento
    Nodo *actual = lista->cabeza;
    while (actual != NULL) {
        if (lista->tipo == ENTERO) {
            *(int*)actual->dato += *(int*)valor;
        } else if (lista->tipo == REAL) {
            *(float*)actual->dato += *(float*)valor;
        }
        actual = actual->siguiente;
    }
}

// Multiplicar un valor a todos los elementos de la lista (modifica la lista original)
static inline void MultiplicaValor(Lista *lista, void *valor) {
    if (lista == NULL || valor == NULL) {
        return;
    }
    
    // Solo funciona con tipos numéricos
    if (lista->tipo != ENTERO && lista->tipo != REAL) {
        return;
    }
    
    // Recorrer y multiplicar el valor a cada elemento
    Nodo *actual = lista->cabeza;
    while (actual != NULL) {
        if (lista->tipo == ENTERO) {
            *(int*)actual->dato *= *(int*)valor;
        } else if (lista->tipo == REAL) {
            *(float*)actual->dato *= *(float*)valor;
        }
        actual = actual->siguiente;
    }
}

// Llenar una lista desde un array genérico
// Para ENTERO y REAL: pasar array de valores (int[] o float[])
// Para CARACTER: pasar array de char* (char*[])
// Para BOOLEANO: pasar array de int (int[])
static inline void LlenarLista(Lista *lista, void *elementos, int cantidad) {
    if (lista == NULL || elementos == NULL || cantidad <= 0) {
        return;
    }
    
    switch (lista->tipo) {
        case ENTERO: {
            int *arr = (int*)elementos;
            for (int i = 0; i < cantidad; i++) {
                insertarLista(lista, &arr[i]);
            }
            break;
        }
        case REAL: {
            float *arr = (float*)elementos;
            for (int i = 0; i < cantidad; i++) {
                insertarLista(lista, &arr[i]);
            }
            break;
        }
        case CARACTER: {
            char **arr = (char**)elementos;
            for (int i = 0; i < cantidad; i++) {
                insertarLista(lista, arr[i]);
            }
            break;
        }
        case BOOLEANO: {
            int *arr = (int*)elementos;
            for (int i = 0; i < cantidad; i++) {
                insertarLista(lista, &arr[i]);
            }
            break;
        }
    }
}

#endif