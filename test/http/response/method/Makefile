NAME = method_test
SRCS = main.cpp get.cpp head.cpp post.cpp \
../../../../src/http/response/get_method.cpp \
../../../../toolbox/stepmark.cpp ../../../../toolbox/string.cpp \
../../../../src/http/response/method_utils.cpp \
../../../../src/http/response/head_method.cpp \
../../../../src/http/request/http_fields.cpp \
../../../../src/http/http_namespace.cpp \
../../../../src/http/response/response.cpp \
../../../../src/http/response/post_method.cpp \
../../../../src/http/string_utils.cpp \
../../../../src/http/response/content_type_manager.cpp

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
	$(RM) stepmark.log
	$(RM) post_test.log
	$(RM) -rf uploads

re: fclean all

.PHONY: all clean fclean re
