NAME = webserv

CXX = c++

CXX_FLAGS = -Wall -Werror -Wextra -std=c++98 -MMD -MP -Wold-style-cast -Wpedantic

SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

TEST_NAME = test_bin

SOURCES =
include make/sources.mk
HEADERS =
include make/headers.mk

OBJECTS = $(patsubst %,$(OBJ_DIR)/%,$(SOURCES:.cpp=.o))

DEBUG := 1
SAN := 0

ifeq ($(DEBUG), 1)
	CXX_FLAGS += -g
endif

ifeq ($(SAN), 1)
	CXX_FLAGS += -fsanitize=address
endif

INCLUDE = -I $(INC_DIR)

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CXX) $(CXX_FLAGS) -o $@ $^

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	@mkdir -p $(@D)
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o $@ $<

$(OBJ_DIR):
	mkdir -p $@

clean:
	$(RM) -r $(OBJ_DIR) $(TEST_OBJ_DIR)

fclean: clean
	$(RM) $(NAME) $(TEST_NAME)

re:
	make fclean
	make all

run: all
	./$(NAME)

files:
	./make/make_sources.sh

print:
	@echo "---SOURCES: $(SOURCES)" | xargs -n1
	@echo "---HEADERS: $(HEADERS)" | xargs -n1
	@echo "---OBJECTS: $(OBJECTS)" | xargs -n1

format: files
	clang-format -i $(SOURCES) $(HEADERS)

siege: all
	siege -R siege.conf

.PHONY: all clean fclean re run files print format siege

# ============================= #
# 			testing				#
# ============================= #

TEST_CXXFLAGS = -std=c++14 -Wall -Wextra
TEST_DIR = test
TEST_OBJ_DIR = $(TEST_DIR)/obj_test

TEST_SRCS=$(notdir $(wildcard $(TEST_DIR)/*.cpp))
TEST_HEADERS=$(wildcard $(TEST_DIR)/*.hpp)
TEST_OBJ=$(TEST_SRCS:%.cpp=$(TEST_OBJ_DIR)/%.o)
OBJ_WITHOUT_MAIN = $(filter-out $(OBJ_DIR)/$(SRC_DIR)/main.o, $(OBJECTS))

$(TEST_NAME): $(TEST_OBJ) $(OBJ_WITHOUT_MAIN)
	$(CXX) $(TEST_CXXFLAGS) -o $@ $^

$(TEST_OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp $(TEST_HEADERS) | $(TEST_OBJ_DIR)
	$(CXX) -c $(TEST_CXXFLAGS) -I $(INC_DIR) -I $(TEST_DIR) -o $@ $<

$(TEST_OBJ_DIR):
	mkdir $@

test: $(TEST_NAME)
	./$(TEST_NAME)

.PHONY: test test_exe

-include $(OBJECTS:.o=.d)
