SRC_DIR	:= src
OBJ_DIR := obj
BIN_DIR := bin
TST_DIR := tests

EXE1 := $(BIN_DIR)/oss
EXE2 := $(BIN_DIR)/user
FIL1 := $(OBJ_DIR)/user.o
FIL2 := $(OBJ_DIR)/oss.o
SRC  := $(wildcard $(SRC_DIR)/*.cpp)
OBJ  := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
TST  := $(wildcard $(TST_DIR)/*.cpp)
TBJ  := $(TST:$(TST_DIR)/%.cpp=$(TST_DIR)/%.o)
TXE  := $(TBJ:$(TST_DIR)/%.o=$(TST_DIR)/%)

CC		 := g++ -std=c++11
CPPFLAGS := -Iinclude -MMD -MP
CFLAGS 	 := -Wall -g

.PHONY: all tests clean cleanrun

all: $(EXE1) $(EXE2)

tests: $(TXE)

$(EXE1): $(filter-out $(FIL1), $(OBJ)) | $(BIN_DIR)
	$(CC) $^ -o $@

$(EXE2): $(filter-out $(FIL2), $(OBJ)) | $(BIN_DIR)
	$(CC) $^ -o $@

$(TST_DIR)/%: $(TST_DIR)/%.o $(filter-out $(FIL1) $(FIL2), $(OBJ))
	$(CC) $^ -o $@
	@$(RM) $(patsubst $(TST_DIR)/%,$(TST_DIR)/%.d,$@)

$(TST_DIR)/%.o: $(TST_DIR)/%.cpp
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR) $(TXE)

cleanrun:
	@$(RM) -rv $(BIN_DIR)/*.out $(BIN_DIR)/*.log

-include $(OBJ:.o=.d)
