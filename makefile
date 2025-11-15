CC=gcc

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin
INCLUDE_DIR = include
CFLAGS = -I$(INCLUDE_DIR)

TARGET =$(BIN_DIR)/grupo2

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC))
INCLUDES = $(wildcard $(INCLUDE_DIR)/*.h)


all: $(TARGET)

# producir el ejecutable  /bin/grupo2
$(TARGET): $(OBJ)
	@echo "COMPILANDO | GRUPO 2 |"
	@mkdir -p $(BIN_DIR)
	@$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)
	@echo "Compilación finalizada."
	@echo "Ejecutar con ./bin/grupo2"


# regla para compilar los archivos .c a .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(INCLUDES)
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@


.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(BIN_DIR)
