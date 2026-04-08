CC = gcc
CFLAGS = -Wall -Wextra -std=c11
NAME = myls
SRCS = main.c options.c directory.c entry.c sort.c display.c error.c utils.c
OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
