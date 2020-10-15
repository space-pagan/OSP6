SRC_DIR	:= src
OBJ_DIR := obj
BIN_DIR := bin

EXE1 := $(BIN_DIR)/oss
FIL1 := $(OBJ_DIR)/oss_child.o
EXE2 := $(BIN_DIR)/oss_child
FIL2 := $(OBJ_DIR)/oss.o
SRC  := $(wildcard $(SRC_DIR)/*.cpp)
OBJ  := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

CC		 := g++ -std=c++11
CPPFLAGS := -Iinclude -MMD -MP
CFLAGS 	 := -Wall -g

.PHONY: all clean cleanrun

all: $(EXE1) $(EXE2)

$(EXE1): $(filter-out $(FIL1), $(OBJ)) | $(BIN_DIR)
	$(CC) $^ -o $@

$(EXE2): $(filter-out $(FIL2), $(OBJ)) | $(BIN_DIR)
	$(CC) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

cleanrun:
	@$(RM) -rv $(BIN_DIR)/*.out $(BIN_DIR)/*.log

-include $(OBJ:.o=.d)
