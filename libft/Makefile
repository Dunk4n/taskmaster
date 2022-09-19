# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: niduches <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2019/10/08 14:45:08 by niduches          #+#    #+#              #
#    Updated: 2021/05/13 17:13:01 by niduches         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Libft v2.0

CC		=	gcc

SRC		=	ft_atoi.c					\
			ft_atoui.c					\
			ft_hex_str_to_ui.c			\
			ft_isalpha.c				\
			ft_isspace.c				\
			ft_isalnum.c				\
			ft_isascii.c				\
			ft_calloc.c					\
			ft_isdigit.c				\
			ft_isprint.c				\
			ft_bzero.c					\
			ft_itoa.c					\
			ft_memccpy.c				\
			ft_memchr.c					\
			ft_memcmp.c					\
			ft_memcpy.c					\
			ft_putchar_fd.c				\
			ft_putendl_fd.c				\
			ft_memmove.c				\
			ft_putstr_fd.c				\
			ft_putnbr_fd.c				\
			ft_strchr.c					\
			ft_strdup.c					\
			ft_strjoin.c				\
			ft_strlcat.c				\
			ft_strlen.c					\
			ft_memset.c					\
			ft_strncmp.c				\
			ft_strmapi.c				\
			ft_strrchr.c				\
			ft_strnstr.c				\
			ft_strtrim.c				\
			ft_tolower.c				\
			ft_toupper.c				\
			ft_split.c					\
			ft_substr.c					\
			ft_strlcpy.c				\
			ft_pow.c					\
			ft_revstr.c					\
			ft_strcat.c					\
			ft_strncat.c				\
			ft_strcmp.c					\
			ft_strcmp_no_case.c			\
			ft_strcpy.c					\
			ft_strncpy.c				\
			get_next_line.c				\
			ft_str_is_number.c			\

LIB_DIR	=	printf						\

LIB		=	printf/printf.a				\

OBJ		=	$(SRC:%.c=%.o)

CFLAGS	+=	-Wall -Wextra -Werror

%.o: %.c $(INC)
	@mkdir -p $(@D)
	@echo "  CC       $<"
	@$(CC) $(CFLAGS) -c $< -o $@

NAME	=	libft.a

all:	$(NAME)
	@$(MAKE) -C $(LIB_DIR) --no-print-directory

clean:
	@$(MAKE) -C $(LIB_DIR) clean --no-print-directory
	@rm -f $(OBJ)
	@rm -f $(SRCB:%.c=%.o)

fclean: clean
	@$(MAKE) -C $(LIB_DIR) fclean --no-print-directory
	@rm -f $(NAME)

$(NAME): $(OBJ)
	@$(MAKE) -C $(LIB_DIR) --no-print-directory
	@ar rcT $(NAME) $(OBJ) $(LIB)

re:	fclean $(NAME)

.PHONY: all clean fclean re
