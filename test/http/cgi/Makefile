NAME = cgi_test.out

SRC_DIRS = ../../../src
TOOLBOX_DIR = ../../../toolbox

TEST_SRCS = $(wildcard $(SRC_DIRS)/http/*.cpp)\
			$(wildcard $(SRC_DIRS)/http/parsing/*.cpp)\
			$(SRC_DIRS)/http/cgi/cgi_field_parser.cpp\
			$(SRC_DIRS)/http/cgi/cgi_response_parser.cpp\
			$(SRC_DIRS)/http/cgi/cgi_response.cpp\
			$(wildcard $(SRC_DIRS)/http/request/*.cpp)\
			$(wildcard $(SRC_DIRS)/config/*.cpp)\
			$(wildcard $(SRC_DIRS)/http/response/*.cpp)\
			../../../src/core/client.cpp\
			$(wildcard $(TOOLBOX_DIR)/*.cpp)\
			$(wildcard *.cpp) \
            $(wildcard http/*.cpp) \
	
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
	$(RM) cgi_test.log

re: fclean all

run: all
	./$(NAME)

.PHONY: all clean fclean re run