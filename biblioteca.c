/*
 * ============================================================================
 *  Biblioteca Personal - Aplicacion de gestion con interfaz de texto (TUI)
 * ----------------------------------------------------------------------------
 *  Parte 1 del Trabajo Final (version SIN vulnerabilidad).
 *
 *  Gestiona una coleccion de libros con registro/login de usuarios,
 *  operaciones CRUD sobre los libros, persistencia en ficheros locales
 *  y varias operaciones de procesamiento (ordenacion, estadisticas e informes).
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------------------------- Colores ANSI -------------------------------- */
#define RESET    "\033[0m"
#define ROJO     "\033[31m"
#define VERDE    "\033[32m"
#define AMARILLO "\033[33m"
#define AZUL     "\033[34m"
#define CYAN     "\033[36m"
#define NEGRITA  "\033[1m"

/* ------------------------------- Limites ---------------------------------- */
#define MAX_LIBROS    100
#define MAX_USUARIOS   50
#define TAM_TXT       100
#define TAM_CORTO      50

/* ------------------------------ Ficheros ---------------------------------- */
#define FICH_LIBROS   "libros.dat"
#define FICH_USUARIOS "usuarios.dat"

/* ----------------------------- Estructuras -------------------------------- */
typedef struct {
    char titulo[TAM_TXT];
    char autor[TAM_TXT];
    char genero[TAM_CORTO];
    int  anio;
    int  valoracion;   /* de 1 a 5 */
    int  leido;        /* 0 = pendiente, 1 = leido */
} Libro;

typedef struct {
    char usuario[TAM_CORTO];
    char password[TAM_CORTO];
} Usuario;

/* -------------------------- Variables globales ---------------------------- */
Libro   libros[MAX_LIBROS];
int     num_libros = 0;

Usuario usuarios[MAX_USUARIOS];
int     num_usuarios = 0;

char    usuario_actual[TAM_CORTO] = "";

/* ==========================================================================
 *  Utilidades de entrada / salida
 * ========================================================================== */

/* Lee una linea completa y le quita el salto de linea final. */
void leer_linea(char *destino, int tam)
{
    if (fgets(destino, tam, stdin) != NULL)
        destino[strcspn(destino, "\n")] = '\0';
    else
        destino[0] = '\0';
}

/* Lee una linea y la convierte a entero. */
int leer_entero(void)
{
    char buffer[32];
    leer_linea(buffer, sizeof(buffer));
    return atoi(buffer);
}

void limpiar_pantalla(void)
{
    printf("\033[2J\033[H");
}

void pausa(void)
{
    int c;
    printf("\nPulsa " NEGRITA "ENTER" RESET " para continuar...");
    while ((c = getchar()) != '\n' && c != EOF)
        ; /* consume hasta el final de la linea */
}

/* ==========================================================================
 *  Persistencia en ficheros (lectura al inicio / escritura al modificar)
 * ========================================================================== */

void cargar_usuarios(void)
{
    FILE *f = fopen(FICH_USUARIOS, "rb");
    if (f == NULL)
        return;  /* primera ejecucion: aun no existe el fichero */

    if (fread(&num_usuarios, sizeof(int), 1, f) == 1) {
        if (num_usuarios < 0 || num_usuarios > MAX_USUARIOS)
            num_usuarios = 0;
        fread(usuarios, sizeof(Usuario), num_usuarios, f);
    }
    fclose(f);
}

void guardar_usuarios(void)
{
    FILE *f = fopen(FICH_USUARIOS, "wb");
    if (f == NULL) {
        printf(ROJO "Error: no se pudo guardar el fichero de usuarios.\n" RESET);
        return;
    }
    fwrite(&num_usuarios, sizeof(int), 1, f);
    fwrite(usuarios, sizeof(Usuario), num_usuarios, f);
    fclose(f);
}

void cargar_libros(void)
{
    FILE *f = fopen(FICH_LIBROS, "rb");
    if (f == NULL)
        return;

    if (fread(&num_libros, sizeof(int), 1, f) == 1) {
        if (num_libros < 0 || num_libros > MAX_LIBROS)
            num_libros = 0;
        fread(libros, sizeof(Libro), num_libros, f);
    }
    fclose(f);
}

void guardar_libros(void)
{
    FILE *f = fopen(FICH_LIBROS, "wb");
    if (f == NULL) {
        printf(ROJO "Error: no se pudo guardar el fichero de libros.\n" RESET);
        return;
    }
    fwrite(&num_libros, sizeof(int), 1, f);
    fwrite(libros, sizeof(Libro), num_libros, f);
    fclose(f);
}

/* ==========================================================================
 *  Gestion de usuarios
 * ========================================================================== */

/* Devuelve 1 si el nombre de usuario ya existe, 0 en caso contrario. */
int existe_usuario(const char *nombre)
{
    int i;
    for (i = 0; i < num_usuarios; i++)
        if (strcmp(usuarios[i].usuario, nombre) == 0)
            return 1;
    return 0;
}

void registrar_usuario(void)
{
    char nombre[TAM_CORTO];
    char clave[TAM_CORTO];

    limpiar_pantalla();
    printf(CYAN NEGRITA "=== Registro de nuevo usuario ===\n\n" RESET);

    if (num_usuarios >= MAX_USUARIOS) {
        printf(ROJO "No se pueden registrar mas usuarios.\n" RESET);
        pausa();
        return;
    }

    printf("Nombre de usuario: ");
    leer_linea(nombre, sizeof(nombre));

    if (strlen(nombre) == 0) {
        printf(ROJO "El nombre no puede estar vacio.\n" RESET);
        pausa();
        return;
    }
    if (existe_usuario(nombre)) {
        printf(ROJO "Ese usuario ya existe. Prueba con otro nombre.\n" RESET);
        pausa();
        return;
    }

    printf("Contrasena: ");
    leer_linea(clave, sizeof(clave));

    if (strlen(clave) == 0) {
        printf(ROJO "La contrasena no puede estar vacia.\n" RESET);
        pausa();
        return;
    }

    strcpy(usuarios[num_usuarios].usuario, nombre);
    strcpy(usuarios[num_usuarios].password, clave);
    num_usuarios++;
    guardar_usuarios();

    printf(VERDE "\nUsuario '%s' registrado correctamente.\n" RESET, nombre);
    pausa();
}

/* Devuelve 1 si el login es correcto (y guarda el usuario activo), 0 si no. */
int iniciar_sesion(void)
{
    char nombre[TAM_CORTO];
    char clave[TAM_CORTO];
    int i;

    limpiar_pantalla();
    printf(CYAN NEGRITA "=== Inicio de sesion ===\n\n" RESET);

    printf("Usuario: ");
    leer_linea(nombre, sizeof(nombre));
    printf("Contrasena: ");
    leer_linea(clave, sizeof(clave));

    for (i = 0; i < num_usuarios; i++) {
        if (strcmp(usuarios[i].usuario, nombre) == 0 &&
            strcmp(usuarios[i].password, clave) == 0) {
            strcpy(usuario_actual, nombre);
            printf(VERDE "\nBienvenido/a, %s!\n" RESET, nombre);
            pausa();
            return 1;
        }
    }

    printf(ROJO "\nUsuario o contrasena incorrectos.\n" RESET);
    pausa();
    return 0;
}

void ver_perfil(void)
{
    limpiar_pantalla();
    printf(CYAN NEGRITA "=== Mi perfil ===\n\n" RESET);
    printf("Usuario activo : %s\n", usuario_actual);
    printf("Libros totales : %d\n", num_libros);
    pausa();
}

/* ==========================================================================
 *  Funcion objetivo del exploit (ret2win)
 * --------------------------------------------------------------------------
 *  No se invoca desde ninguna parte del flujo normal del programa: unicamente
 *  esta presente en el binario. El exploit de Buffer Overflow debera redirigir
 *  la ejecucion hasta aqui. Abre ./flag.txt y vuelca su contenido por pantalla.
 * ========================================================================== */
void win(void)
{
    char linea[256];
    FILE *f = fopen("./flag.txt", "r");

    if (f == NULL) {
        printf(ROJO "win(): no se pudo abrir ./flag.txt\n" RESET);
        fflush(stdout);
        return;
    }

    printf(VERDE "\n*** win() alcanzada -- contenido de la flag: ***\n" RESET);
    while (fgets(linea, sizeof(linea), f) != NULL)
        fputs(linea, stdout);

    fclose(f);
    fflush(stdout);
}

/* ==========================================================================
 *  Gestion de libros (altas, listados, busquedas, modificaciones, bajas)
 * ========================================================================== */

/* Busca un libro por titulo exacto y devuelve un puntero a el (o NULL). */
Libro *buscar_por_titulo(const char *titulo)
{
    int i;
    for (i = 0; i < num_libros; i++)
        if (strcmp(libros[i].titulo, titulo) == 0)
            return &libros[i];
    return NULL;
}

void mostrar_libro(const Libro *l, int indice)
{
    printf(AMARILLO "[%d] %s\n" RESET, indice, l->titulo);
    printf("    Autor      : %s\n", l->autor);
    printf("    Genero     : %s\n", l->genero);
    printf("    Anio       : %d\n", l->anio);
    if (l->valoracion > 0)
        printf("    Valoracion : %d/5\n", l->valoracion);
    else
        printf("    Valoracion : Sin valorar\n");
    printf("    Estado     : %s\n", l->leido ? "Leido" : "Pendiente");
}

void anadir_libro(void)
{
    Libro nuevo;

    limpiar_pantalla();
    printf(CYAN NEGRITA "=== Anadir libro ===\n\n" RESET);

    if (num_libros >= MAX_LIBROS) {
        printf(ROJO "La biblioteca esta llena.\n" RESET);
        pausa();
        return;
    }

    printf("Titulo: ");
    leer_linea(nuevo.titulo, sizeof(nuevo.titulo));
    if (strlen(nuevo.titulo) == 0) {
        printf(ROJO "El titulo no puede estar vacio.\n" RESET);
        pausa();
        return;
    }
    if (buscar_por_titulo(nuevo.titulo) != NULL) {
        printf(ROJO "Ya existe un libro con ese titulo.\n" RESET);
        pausa();
        return;
    }

    printf("Autor: ");
    leer_linea(nuevo.autor, sizeof(nuevo.autor));

    printf("Genero: ");
    leer_linea(nuevo.genero, sizeof(nuevo.genero));

    printf("Anio de publicacion: ");
    nuevo.anio = leer_entero();
    if (nuevo.anio < 0 || nuevo.anio > 2100) {
        printf(ROJO "Anio no valido.\n" RESET);
        pausa();
        return;
    }

    printf("Leido? (1 = si, 0 = no): ");
    nuevo.leido = leer_entero() ? 1 : 0;

    if (nuevo.leido) {
        printf("Valoracion (1-5): ");
        nuevo.valoracion = leer_entero();
        if (nuevo.valoracion < 1 || nuevo.valoracion > 5) {
            printf(ROJO "La valoracion debe estar entre 1 y 5.\n" RESET);
            pausa();
            return;
        }
    } else {
        nuevo.valoracion = 0;   /* pendiente: todavia sin valorar */
    }

    libros[num_libros] = nuevo;
    num_libros++;
    guardar_libros();

    printf(VERDE "\nLibro anadido correctamente.\n" RESET);
    pausa();
}

void listar_libros(void)
{
    int i;
    limpiar_pantalla();
    printf(CYAN NEGRITA "=== Listado de libros (%d) ===\n\n" RESET, num_libros);

    if (num_libros == 0) {
        printf("No hay libros registrados todavia.\n");
    } else {
        for (i = 0; i < num_libros; i++) {
            mostrar_libro(&libros[i], i);
            printf("\n");
        }
    }
    pausa();
}

void buscar_libro(void)
{
    char criterio[64];   /* buffer de busqueda en pila */
    int opcion, i, encontrados = 0;

    limpiar_pantalla();
    printf(CYAN NEGRITA "=== Buscar libro ===\n\n" RESET);
    printf("Buscar por:\n");
    printf("  1) Titulo\n");
    printf("  2) Autor\n");
    printf("  3) Genero\n");
    printf("Opcion: ");
    opcion = leer_entero();

    if (opcion < 1 || opcion > 3) {
        printf(ROJO "Opcion no valida.\n" RESET);
        pausa();
        return;
    }

    /* VULNERABILIDAD (Buffer Overflow): se leen hasta 255 bytes sobre un
     * buffer de solo 64 -> se puede sobrescribir la direccion de retorno. */
    printf("Texto a buscar: ");
    fgets(criterio, 256, stdin);
    criterio[strcspn(criterio, "\n")] = '\0';
    printf("\n");

    for (i = 0; i < num_libros; i++) {
        char *campo;
        if (opcion == 1) campo = libros[i].titulo;
        else if (opcion == 2) campo = libros[i].autor;
        else campo = libros[i].genero;

        if (strstr(campo, criterio) != NULL) {
            mostrar_libro(&libros[i], i);
            printf("\n");
            encontrados++;
        }
    }

    if (encontrados == 0)
        printf("No se encontraron coincidencias.\n");
    else
        printf(VERDE "%d resultado(s) encontrado(s).\n" RESET, encontrados);

    pausa();
}

void modificar_libro(void)
{
    char titulo[TAM_TXT];
    Libro *l;

    limpiar_pantalla();
    printf(CYAN NEGRITA "=== Modificar libro ===\n\n" RESET);

    printf("Titulo del libro a modificar: ");
    leer_linea(titulo, sizeof(titulo));

    l = buscar_por_titulo(titulo);
    if (l == NULL) {
        printf(ROJO "No existe ningun libro con ese titulo.\n" RESET);
        pausa();
        return;
    }

    printf("\nIntroduce los nuevos datos:\n");

    printf("Autor (%s): ", l->autor);
    leer_linea(l->autor, sizeof(l->autor));

    printf("Genero (%s): ", l->genero);
    leer_linea(l->genero, sizeof(l->genero));

    printf("Anio (%d): ", l->anio);
    l->anio = leer_entero();

    printf("Leido? (1 = si, 0 = no): ");
    l->leido = leer_entero() ? 1 : 0;

    if (l->leido) {
        printf("Valoracion (1-5): ");
        l->valoracion = leer_entero();
        if (l->valoracion < 1) l->valoracion = 1;
        if (l->valoracion > 5) l->valoracion = 5;
    } else {
        l->valoracion = 0;   /* pendiente: sin valorar */
    }

    guardar_libros();
    printf(VERDE "\nLibro modificado correctamente.\n" RESET);
    pausa();
}

void eliminar_libro(void)
{
    char titulo[TAM_TXT];
    int i, pos = -1;

    limpiar_pantalla();
    printf(CYAN NEGRITA "=== Eliminar libro ===\n\n" RESET);

    printf("Titulo del libro a eliminar: ");
    leer_linea(titulo, sizeof(titulo));

    for (i = 0; i < num_libros; i++)
        if (strcmp(libros[i].titulo, titulo) == 0) {
            pos = i;
            break;
        }

    if (pos == -1) {
        printf(ROJO "No existe ningun libro con ese titulo.\n" RESET);
        pausa();
        return;
    }

    /* Desplaza el resto de elementos una posicion hacia atras. */
    for (i = pos; i < num_libros - 1; i++)
        libros[i] = libros[i + 1];
    num_libros--;

    guardar_libros();
    printf(VERDE "\nLibro eliminado correctamente.\n" RESET);
    pausa();
}

/* ==========================================================================
 *  Procesamiento de informacion (ordenacion, estadisticas e informes)
 * ========================================================================== */

void ordenar_libros(void)
{
    int opcion, i, j;

    limpiar_pantalla();
    printf(CYAN NEGRITA "=== Ordenar libros ===\n\n" RESET);
    printf("  1) Por anio (ascendente)\n");
    printf("  2) Por valoracion (descendente)\n");
    printf("Opcion: ");
    opcion = leer_entero();

    if (opcion < 1 || opcion > 2) {
        printf(ROJO "Opcion no valida.\n" RESET);
        pausa();
        return;
    }

    /* Ordenacion por seleccion sobre el array de libros. */
    for (i = 0; i < num_libros - 1; i++) {
        int sel = i;
        for (j = i + 1; j < num_libros; j++) {
            int cambiar = 0;
            if (opcion == 1 && libros[j].anio < libros[sel].anio)
                cambiar = 1;
            if (opcion == 2 && libros[j].valoracion > libros[sel].valoracion)
                cambiar = 1;
            if (cambiar) sel = j;
        }
        if (sel != i) {
            Libro tmp = libros[i];
            libros[i] = libros[sel];
            libros[sel] = tmp;
        }
    }

    guardar_libros();
    printf(VERDE "Libros ordenados. Usa el listado para verlos.\n" RESET);
    pausa();
}

void mostrar_estadisticas(void)
{
    int i, j;
    int leidos = 0, pendientes = 0;
    int suma_valoracion = 0, valorados = 0;

    limpiar_pantalla();
    printf(CYAN NEGRITA "=== Estadisticas ===\n\n" RESET);

    if (num_libros == 0) {
        printf("No hay libros para analizar.\n");
        pausa();
        return;
    }

    for (i = 0; i < num_libros; i++) {
        if (libros[i].valoracion > 0) {
            suma_valoracion += libros[i].valoracion;
            valorados++;
        }
        if (libros[i].leido) leidos++;
        else pendientes++;
    }

    printf("Total de libros   : %d\n", num_libros);
    printf("Leidos            : %d\n", leidos);
    printf("Pendientes        : %d\n", pendientes);
    if (valorados > 0)
        printf("Valoracion media  : %.2f / 5 (sobre %d valorados)\n",
               (double) suma_valoracion / valorados, valorados);
    else
        printf("Valoracion media  : -- (ningun libro valorado)\n");

    /* Conteo de libros por genero (sin repetir generos ya mostrados). */
    printf("\n" NEGRITA "Libros por genero:\n" RESET);
    for (i = 0; i < num_libros; i++) {
        int ya_mostrado = 0;
        int cuenta = 0;

        for (j = 0; j < i; j++)
            if (strcmp(libros[j].genero, libros[i].genero) == 0) {
                ya_mostrado = 1;
                break;
            }
        if (ya_mostrado) continue;

        for (j = 0; j < num_libros; j++)
            if (strcmp(libros[j].genero, libros[i].genero) == 0)
                cuenta++;

        printf("  - %-20s : %d\n", libros[i].genero, cuenta);
    }

    pausa();
}

void informe_pendientes(void)
{
    int i, encontrados = 0;

    limpiar_pantalla();
    printf(CYAN NEGRITA "=== Informe de libros pendientes ===\n\n" RESET);

    for (i = 0; i < num_libros; i++) {
        if (!libros[i].leido) {
            printf(AMARILLO "- %s" RESET " (%s, %d)\n",
                   libros[i].titulo, libros[i].autor, libros[i].anio);
            encontrados++;
        }
    }

    if (encontrados == 0)
        printf(VERDE "No tienes libros pendientes. Enhorabuena!\n" RESET);
    else
        printf("\nTotal de pendientes: %d\n", encontrados);

    pausa();
}

/* ==========================================================================
 *  Menus (principal y submenus)
 * ========================================================================== */

void pantalla_bienvenida(void)
{
    limpiar_pantalla();
    printf(CYAN NEGRITA);
    printf("  ____  _ _     _ _       _                 \n");
    printf(" | __ )(_) |__ | (_) ___ | |_ ___  ___ __ _ \n");
    printf(" |  _ \\| | '_ \\| | |/ _ \\| __/ _ \\/ __/ _` |\n");
    printf(" | |_) | | |_) | | | (_) | ||  __/ (_| (_| |\n");
    printf(" |____/|_|_.__/|_|_|\\___/ \\__\\___|\\___\\__,_|\n");
    printf(RESET);
    printf(AZUL "        Gestor de Biblioteca Personal\n" RESET);
    printf("        ---------------------------------\n\n");
    printf("Pulsa " NEGRITA "ENTER" RESET " para comenzar...");
    {
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }
}

void menu_libros(void)
{
    int opcion;
    do {
        limpiar_pantalla();
        printf(CYAN NEGRITA "=== Gestion de libros ===\n\n" RESET);
        printf("  1) Anadir libro\n");
        printf("  2) Listar libros\n");
        printf("  3) Buscar libro\n");
        printf("  4) Modificar libro\n");
        printf("  5) Eliminar libro\n");
        printf("  0) Volver\n\n");
        printf("Opcion: ");
        opcion = leer_entero();

        switch (opcion) {
            case 1: anadir_libro();    break;
            case 2: listar_libros();   break;
            case 3: buscar_libro();    break;
            case 4: modificar_libro(); break;
            case 5: eliminar_libro();  break;
            case 0: break;
            default:
                printf(ROJO "Opcion no valida.\n" RESET);
                pausa();
        }
    } while (opcion != 0);
}

void menu_procesamiento(void)
{
    int opcion;
    do {
        limpiar_pantalla();
        printf(CYAN NEGRITA "=== Procesamiento de informacion ===\n\n" RESET);
        printf("  1) Ordenar libros\n");
        printf("  2) Ver estadisticas\n");
        printf("  3) Informe de pendientes\n");
        printf("  0) Volver\n\n");
        printf("Opcion: ");
        opcion = leer_entero();

        switch (opcion) {
            case 1: ordenar_libros();      break;
            case 2: mostrar_estadisticas(); break;
            case 3: informe_pendientes();  break;
            case 0: break;
            default:
                printf(ROJO "Opcion no valida.\n" RESET);
                pausa();
        }
    } while (opcion != 0);
}

/* Menu principal una vez el usuario ha iniciado sesion. */
void menu_principal(void)
{
    int opcion;
    do {
        limpiar_pantalla();
        printf(CYAN NEGRITA "=== Menu principal ===" RESET
               "  (sesion: %s)\n\n", usuario_actual);
        printf("  1) Gestion de libros\n");
        printf("  2) Procesamiento de informacion\n");
        printf("  3) Ver mi perfil\n");
        printf("  4) Cerrar sesion\n");
        printf("  0) Salir del programa\n\n");
        printf("Opcion: ");
        opcion = leer_entero();

        switch (opcion) {
            case 1: menu_libros();        break;
            case 2: menu_procesamiento(); break;
            case 3: ver_perfil();         break;
            case 4:
                printf(VERDE "\nSesion cerrada.\n" RESET);
                usuario_actual[0] = '\0';
                pausa();
                return;  /* vuelve al menu de acceso */
            case 0:
                printf(AZUL "\nHasta pronto!\n" RESET);
                exit(0);
            default:
                printf(ROJO "Opcion no valida.\n" RESET);
                pausa();
        }
    } while (1);
}

/* Menu de acceso: registro / login / salir. */
void menu_acceso(void)
{
    int opcion;
    do {
        limpiar_pantalla();
        printf(CYAN NEGRITA "=== Acceso ===\n\n" RESET);
        printf("  1) Registrarse\n");
        printf("  2) Iniciar sesion\n");
        printf("  0) Salir\n\n");
        printf("Opcion: ");
        opcion = leer_entero();

        switch (opcion) {
            case 1:
                registrar_usuario();
                break;
            case 2:
                if (iniciar_sesion())
                    menu_principal();
                break;
            case 0:
                printf(AZUL "\nHasta pronto!\n" RESET);
                return;
            default:
                printf(ROJO "Opcion no valida.\n" RESET);
                pausa();
        }
    } while (1);
}

/* ==========================================================================
 *  Funcion principal
 * ========================================================================== */

int main(void)
{
    /* Sin buffer en la E/S: necesario para que funcione servido por socat. */
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin,  NULL, _IONBF, 0);

    /* Carga de datos de ejecuciones anteriores. */
    cargar_usuarios();
    cargar_libros();

    pantalla_bienvenida();
    menu_acceso();

    return 0;
}
