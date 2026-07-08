NAME = webserv
CXX = c++
CXXFLAGS = -Wextra -Werror -Wall -std=c++98 -Iinclude
SRC = main.cpp \
	src/http-response/http-response.cpp
OBJ = $(SRC:.c=.o)

all:$(NAME)

$(NAME):$(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean:
	rm -f $(OBJ)

fclean:
	rm -f $(OBJ) $(NAME)

re:fclean all

.PHONY:all clean fclean re

