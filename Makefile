NAME = webserv

# sources
SRCS = $(wildcard toolbox/*.cpp) $(wildcard src/*/*.cpp) $(wildcard src/*/*/*.cpp)

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

setup-test:
	chmod +x setup.sh
	./setup.sh

clean-test:
	chmod +x clean.sh
	./clean.sh

.PHONY: all clean fclean re setup-test clean-test
