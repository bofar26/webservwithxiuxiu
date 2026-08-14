NAME = webserv
CXX = c++

CXXFLAGS = -Wextra -Werror -Wall -std=c++98 -Iinclude -MMD -MP
SRC = main.cpp \
	src/http-response/HttpResponse.cpp \
	src/server/Server.cpp \
	src/server/Connection.cpp \
	src/cgi/CgiHandler.cpp \
	src/http-request/HttpRequest.cpp \
	src/router-static/Router.cpp \
	src/router-static/RouterFileUtils.cpp \
	src/router-static/RouterStatic.cpp \
	src/config-parser/ConfigParser.cpp \
	src/config-parser/ServerConfig.cpp \
	src/config-parser/LocationConfig.cpp
OBJ = $(SRC:.cpp=.o)
DEP = $(SRC:.cpp=.d)

all:$(NAME)

$(NAME):$(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean:
	rm -f $(OBJ) $(DEP)

fclean:
	rm -f $(OBJ) $(DEP) $(NAME)

re:fclean all

-include $(DEP)

.PHONY:all clean fclean re
