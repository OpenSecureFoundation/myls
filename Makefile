CC = gcc
CFLAGS = -Wall -Wextra -std=c11
NAME = myls
PREFIX ?= $(HOME)/.local
BINDIR ?= $(PREFIX)/bin
SRCS = main.c options.c directory.c entry.c sort.c display.c error.c utils.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean fclean re install uninstall

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

install: $(NAME)
	mkdir -p $(DESTDIR)$(BINDIR)
	install -m 755 $(NAME) $(DESTDIR)$(BINDIR)/$(NAME)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(NAME)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
