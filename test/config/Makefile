NAME = config_test.out
CXX = g++
CXXFLAGS = -std=c++98 -Wall -Wextra -I src/

TEST_SRCS = $(wildcard *.cpp)
CONFIG_SRCS = $(wildcard ../../src/config/*.cpp)
TOOLBOX_SRCS = $(wildcard ../../toolbox/*.cpp)
SRCS = $(TEST_SRCS) $(CONFIG_SRCS) $(TOOLBOX_SRCS)

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME):$(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(NAME)

re: fclean all

fclean: clean
	rm -f $(NAME)
	rm -f config_test_syntax.log config_test_directive.log

run: fclean all
	chmod +x config_test.sh
	./config_test.sh

.PHONY: all clean fclean re run
