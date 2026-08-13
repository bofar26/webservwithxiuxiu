NAME = webserv
CXX = c++
CXXFLAGS = -Wextra -Werror -Wall -std=c++98 -Iinclude
SRC = main.cpp \
	src/http-response/HttpResponse.cpp \
	src/server/Server.cpp \
	src/server/Connection.cpp \
	src/http-request/HttpRequest.cpp \
	src/router-static/Router.cpp \
	src/router-static/RouterFileUtils.cpp \
	src/router-static/RouterStatic.cpp \
	src/config-parser/ConfigParser.cpp \
	src/config-parser/ServerConfig.cpp \
	src/config-parser/LocationConfig.cpp
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

