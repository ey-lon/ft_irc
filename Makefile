NAME = ircserv

CC = c++

RM = rm -rf

FLAGS = -Wall -Wextra -Werror -std=c++98

#----------------------------------------------
#FOLDERS

SRC_F = src/
OBJ_F = obj/

#----------------------------------------------
#SOURCE FILES

SRC =	main.cpp \
		Server.cpp \
		Channel.cpp \
		User.cpp \
		Utils.cpp \
		Errors.cpp

#----------------------------------------------
#OBJECT FILES

OBJ = $(SRC:.cpp=.o)
OBJ := $(addprefix $(OBJ_F),$(OBJ))

$(OBJ_F)%.o : $(SRC_F)%.cpp
	mkdir -p $(OBJ_F)
	$(CC) $(FLAGS) -c $< -o $@

#----------------------------------------------
#RULES

$(NAME): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

all: $(NAME)

clean:
	$(RM) $(OBJ)
	$(RM) $(OBJ_F)

fclean: clean
	$(RM) $(NAME)

re: fclean all

#----------------------------------------------

.PHONY: all clean fclean re
#.SILENT: