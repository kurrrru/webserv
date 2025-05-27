NAME = test.out

SRC_DIRS = ../../../src
TOOLBOX_DIR = ../../../toolbox

TEST_SRCS = $(wildcard *.cpp) \
            $(wildcard ../../../src/config/*.cpp) \
            $(wildcard ../../../src/http/*.cpp) \
			$(wildcard ../../../toolbox/*.cpp)

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
	$(RM) request_test.log

re: fclean all

run: all
	./$(NAME)

.PHONY: all clean fclean re run