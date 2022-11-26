NAME = webserv

CXX = c++

CXX_FLAGS = -Wall -Werror -Wextra -std=c++11 -MMD -MP -Wold-style-cast -Wpedantic -Wno-unknown-pragmas

SRC_DIR = src
OBJ_DIR = obj


SOURCES =
include make/sources.mk
HEADERS =
include make/headers.mk
INCLUDE = 
include make/include.mk

OBJECTS = $(patsubst %,$(OBJ_DIR)/%,$(SOURCES:.cpp=.o))

export MAKEFLAGS = "-j 4"

DEBUG := 1
SAN := 0

ifeq ($(DEBUG), 1)
	CXX_FLAGS += -D DEBUG -g3
	ifeq ($(SAN), 1)
		CXX_FLAGS += -fsanitize=address,undefined
	endif
else
	CXX_FLAGS += -O2
endif

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

siege:
	siege -iR siege.conf

lsof:
	lsof -c webserv | tail +8

FILE := lorem_ipsum
upload:
	curl localhost:8080/cgi-bin/upload.py -F "userfile=@$(FILE)" -H "Expect:" -v

.PHONY: all clean fclean re run files print format siege lsof

# ============================= #
# 			testing				#
# ============================= #

TEST_NAME = catch
TEST_DIR = test
TEST_OBJ_DIR = $(TEST_DIR)/obj_test
TEST_CXXFLAGS = -std=c++14 -Wall -Wextra

TEST_SRCS=$(notdir $(wildcard $(TEST_DIR)/*.cpp))
TEST_HEADERS=$(wildcard $(TEST_DIR)/*.hpp)
TEST_OBJ=$(TEST_SRCS:%.cpp=$(TEST_OBJ_DIR)/%.o)
OBJ_WITHOUT_MAIN = $(filter-out $(OBJ_DIR)/$(SRC_DIR)/main.o, $(OBJECTS))

$(TEST_NAME): $(TEST_OBJ) $(OBJ_WITHOUT_MAIN)
	$(CXX) $(TEST_CXXFLAGS) -o $@ $^

$(TEST_OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp $(TEST_HEADERS) | $(TEST_OBJ_DIR)
	$(CXX) -c $(TEST_CXXFLAGS) $(INCLUDE) -I $(TEST_DIR) -o $@ $<

$(TEST_OBJ_DIR):
	mkdir $@

test: $(TEST_NAME)
	./$(TEST_NAME)

.PHONY: test test_exe

FUZZ_DIR = fuzzer
FUZZ_NAME = $(FUZZ_DIR)/fuzzer

fuzz: $(FUZZ_NAME)

dofuzz:
	mkdir -p $(FUZZ_DIR)/CORPUS
	(cd $(FUZZ_DIR) && ./fuzzer CORPUS configs -dict=dict)
	mv $(FUZZ_DIR)/crash-*  crash

merge:
	mkdir -p $(FUZZ_DIR)/Corpus
	./$(FUZZ_NAME) -merge=1 $(FUZZ_DIR)/CORPUS $(FUZZ_DIR)/CORPUS

$(FUZZ_NAME): $(OBJ_WITHOUT_MAIN) $(FUZZ_DIR)/fuzzer.o
	$(CXX) $(CXXFLAGS) -fsanitize=fuzzer,address -o $@ $^

$(FUZZ_DIR)/fuzzer.o: $(FUZZ_DIR)/fuzzer.cpp
	$(CXX) -c $(CXXFLAGS) -fsanitize=fuzzer,address $(INCLUDE) -o $@ $<

-include $(OBJECTS:.o=.d)
