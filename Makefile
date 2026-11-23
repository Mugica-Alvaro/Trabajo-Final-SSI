# Makefile - Biblioteca Personal (64 bits)
#
# El binario asignado al grupo es de 32 bits, por lo que -segun el enunciado-
# esta aplicacion se compila en 64 bits.
#
# Flags relevantes para la vulnerabilidad (Parte 1):
#   -fno-stack-protector  -> desactiva el canario de pila (permite el overflow)
#   -no-pie               -> direcciones fijas: win() tiene una direccion estable
#
# Uso:
#   make          -> compila el binario 'biblioteca'
#   make clean    -> elimina el binario y los ficheros de datos generados

CC      = gcc
CFLAGS  = -Wall -Wextra -fno-stack-protector -no-pie
TARGET  = biblioteca
SRC     = biblioteca.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) libros.dat usuarios.dat

.PHONY: clean
