NAME = webserv


# sources
SRCS = $(wildcard toolbox/*.cpp) $(wildcard src/*/*.cpp) $(wildcard src/*/*/*.cpp)
SRCS := $(filter-out src/http/cgi/%, $(SRCS))
# SRCS := $(filter-out src/http/request/recv_request.cpp, $(SRCS))

OBJS = $(SRCS:.cpp=.o)

# compiler
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

# rules
all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
