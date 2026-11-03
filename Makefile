CXX := gcc
CXXFLAGS := -Wall -Wextra

SRC_DIR := src
BUILD_DIR := build

SRC := src/atendimento.c src/fila.c src/time.c src/processo.c
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEP := $(OBJ:.o=.d)

TARGET := atendimento

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJ)
	$(CXX) -o $@ $(OBJ)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Meio hardcoded essa makefile mas serve wtver
extras:
	$(CXX) $(SRC_DIR)/analista.c -o $(BUILD_DIR)/analista
	$(CXX) cliente.c -o $(BUILD_DIR)/cliente