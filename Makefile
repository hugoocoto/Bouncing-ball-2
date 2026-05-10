BIN = main
LIB = ./raylib-6.0_linux_amd64/lib/libraylib.a -lm -lX11

all: $(BIN)

$(BIN): main.c
	gcc main.c -o $@ $(LIB) 
