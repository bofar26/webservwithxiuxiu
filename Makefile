NAME = webserv
CXX = c++
CXXFLAGS = -Wextra -Werror -Wall -std=c++98 -Iinclude
SRC = main.cpp \
	src/http-response/HttpResponse.cpp \
	src/server/Server.cpp \
	src/http-request/HttpRequest.cpp
OBJ = $(SRC:.cpp=.o)

all:$(NAME)

$(NAME):$(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean:
	rm -f $(OBJ)

fclean:
	rm -f $(OBJ) $(NAME)

re:fclean all

.PHONY:all clean fclean re

