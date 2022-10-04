NAME = webserv

CXX = c++

CXX_FLAGS = -Wall -Werror -Wextra -std=c++98 -MMD -MP -Wold-style-cast

SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

SOURCES =
include make/sources.mk
DEPS = 
include make/dependecies.mk

OBJECTS = $(patsubst %,$(OBJ_DIR)/%,$(SOURCES:.cpp=.o))

ifdef DEBUG
	CXX_FLAGS += -g -fsanitize=address
endif

INCLUDE = -I $(INC_DIR)

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CXX) $(CXX_FLAGS) $(OBJECTS) -o $(NAME)

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	@mkdir -p $(@D)
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o $@ $<

$(OBJ_DIR):
	mkdir -p $@

clean:
	$(RM) -r $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean
	make all

run: all
	./$(NAME)

files:
	./make/make_sources.sh

print:
	@echo "---SOURCES: $(SOURCES)" | xargs -n1
	@echo "---OBJECTS: $(OBJECTS)" | xargs -n1

.PHONY: all files clean fclean re print run

-include $(DEPS)
