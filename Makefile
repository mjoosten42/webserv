NAME := webserv
TEST := catch
FUZZ := fuzzer

CXX = c++

CXX_FLAGS := -Wall -Werror -Wextra -std=c++11 -MMD -MP -Wold-style-cast -Wpedantic -Wno-unknown-pragmas

SRC_DIR = src
OBJ_DIR = obj
TEST_DIR = test

export MAKEFLAGS = "-j 4"

include make/sources.mk
include make/headers.mk
include make/include.mk

.SECONDEXPANSION:

OBJECTS = $(patsubst %,$(OBJ_DIR)/%,$(SOURCES:.cpp=.o))

DEBUG := 1
SAN := 1

ifeq ($(DEBUG), 1)
	CXX_FLAGS += -O0 -D DEBUG -g3
	ifeq ($(SAN), 1)
		CXX_FLAGS += -fsanitize=address,undefined
	endif
else
	CXX_FLAGS += -O2
endif

all: $(NAME)

$(NAME): SOURCES += $(SRC_DIR)/main.cpp
$(NAME): $$(OBJECTS)
	$(CXX) $(CXX_FLAGS) -o $@ $^

$(TEST): SOURCES += $(wildcard $(TEST_DIR)/*.cpp)
$(TEST): INCLUDE += -I $(TEST_DIR)
$(TEST): CXX_FLAGS += -std=c++14 -fprofile-instr-generate -fcoverage-mapping
$(TEST): $$(OBJECTS)
	$(CXX) $(CXX_FLAGS) -o $@ $^

$(FUZZ): SOURCES += $(SRC_DIR)/fuzzer.cpp
$(FUZZ): CXX_FLAGS += -g3 -fsanitize=fuzzer -fprofile-instr-generate -fcoverage-mapping
$(FUZZ): $$(OBJECTS)
	$(CXX) $(CXX_FLAGS) -o $@ $^

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	@mkdir -p $(@D)
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o $@ $<

$(OBJ_DIR):
	mkdir -p $@

clean:
	$(RM) -r $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME) $(TEST) $(FUZZ)
	$(RM) $(wildcard *_profdata)

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

.PHONY: all clean fclean re run files print format

# ============================= #
# 			webserver			#
# ============================= #

FILE ?= lorem_ipsum

siege:
	siege -iR siege.conf

lsof:
	lsof -c webserv | tail +8

upload:
	curl localhost:8080/cgi-bin/upload.py -F "userfile=@$(FILE)" -H "Expect:" -v

.PHONY: siege lsof upload

# ============================= #
# 			testing				#
# ============================= #

test: $(TEST)
	-LLVM_PROFILE_FILE="$(TEST).profraw" ./$(TEST)
	llvm-profdata merge -sparse $(TEST).profraw -o $(TEST)_profdata
	$(RM) $(TEST).profraw

.PHONY: test

# ============================= #
# 			fuzzing				#
# ============================= #

fuzz: $(FUZZ)
	mkdir -p CORPUS
	LLVM_PROFILE_FILE="$(FUZZ).profraw" ASAN_OPTIONS=detect_container_overflow=0 ./$(FUZZ) CORPUS configs -dict=dict -jobs=4 -workers=4 -only_ascii=1 -use_value_profile=1

fuzzpost:
	llvm-profdata merge -sparse $(FUZZ).profraw -o $(FUZZ)_profdata
	$(RM) $(FUZZ).profraw
	$(RM) $(wildcard fuzz-*.log)

merge:
	ASAN_OPTIONS=detect_container_overflow=0 ./$(FUZZ) -merge=1 CORPUS CORPUS

.PHONY: fuzz merge

# ============================= #
# 			coverage			#
# ============================= #

PROF ?= $(TEST)

show:
	llvm-cov show ./$(PROF) -instr-profile=$(PROF)_profdata

report:
	llvm-cov report ./$(PROF) -instr-profile=$(PROF)_profdata

.PHONY: show report

-include $(OBJECTS:.o=.d)
