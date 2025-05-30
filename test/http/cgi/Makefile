NAME = cgi_test.out

SRC_DIRS = ../../../src
TOOLBOX_DIR = ../../../toolbox

EXCLUDE_FILES = $(SRC_DIRS)/http/request/recv_request.cpp

HTTP_REQUEST_SRCS = $(filter-out $(EXCLUDE_FILES), $(wildcard $(SRC_DIRS)/http/request/*.cpp))

TEST_SRCS = $(wildcard $(SRC_DIRS)/http/*.cpp)\
			$(HTTP_REQUEST_SRCS)\
			$(wildcard $(SRC_DIRS)/http/parsing/*.cpp)\
			$(wildcard $(SRC_DIRS)/http/cgi/*.cpp)\
			$(wildcard $(SRC_DIRS)/config/*.cpp)\
			../../../src/http/response/response.cpp\
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