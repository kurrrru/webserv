NAME = test.out

SRC_DIRS = ../../../src
TOOLBOX_DIR = ../../../toolbox

TEST_SRCS = $(filter-out $(SRC_DIRS)/core/main.cpp, $(wildcard $(SRC_DIRS)/*/*.cpp)\
			$(wildcard $(SRC_DIRS)/*/*/*.cpp) $(wildcard $(TOOLBOX_DIR)/*.cpp))\
			$(wildcard *.cpp) \
            $(wildcard http/*.cpp) \
            $(wildcard http/request/*.cpp) \

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I..

TEST_OBJS = $(TEST_SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(TEST_OBJS)

clean:
	$(RM) $(TEST_OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

run: all
	./$(NAME)

.PHONY: all clean fclean re run