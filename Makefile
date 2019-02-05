##
## Makefile for malloc in /home/manass_j/rendu/PSU_2016_malloc
## 
## Made by jonathan manassen
## Login   <jonathan.manassen@epitech.eu@epitech.net>
## 
## Started on  Fri Jan 27 12:00:08 2017 jonathan manassen
## Last update Thu Feb  9 12:38:06 2017 jonathan manassen
##

NAME	= mypam.so

CC	= gcc

RM	= rm -f

SRC	= main.c

OBJ	= $(SRC:.c=.o)

CFLAGS	+= -fpic

all: $(NAME)

$(NAME):	$(OBJ)
	$(CC) -shared -o $(NAME) $(OBJ)

install:
	@mkdir -p /lib/security
	@[ -f "/etc/pam.d/common-auth-save" ] && (cp /etc/pam.d/common-auth-save /etc/pam.d/common-auth) || echo -n ""
	@[ -f "/etc/pam.d/common-password-save" ] && (cp /etc/pam.d/common-password-save /etc/pam.d/common-password) || echo -n ""
	@cp /etc/pam.d/common-auth /etc/pam.d/common-auth-save
	@cp /etc/pam.d/common-password /etc/pam.d/common-password-save
	@echo "auth sufficient /lib/security/mypam.so" >> /etc/pam.d/common-auth
	@echo "password sufficient /lib/security/mypam.so" >> /etc/pam.d/common-password
	@cp mypam.so /lib/security/
	@echo "install complete"

uninstall:
	@[ ! -f "/lib/security/mypam.so" -o ! -f "/etc/pam.d/common-auth-save" -o ! -f "/etc/pam.d/common-password-save" ] && echo "mypam is not installed" || (rm -f /lib/security/mypam.so && mv /etc/pam.d/common-auth-save /etc/pam.d/common-auth && mv /etc/pam.d/common-password-save /etc/pam.d/common-password && echo "uninstall complete")

check:
	@[ ! -f "/lib/security/mypam.so" -o ! -f "/etc/pam.d/common-auth-save" -o ! -f "/etc/pam.d/common-password-save" ] && echo "mypam is not installed" || echo "mypam is installed"

test:

clean :
	$(RM) $(OBJ)
	$(RM) $(NAME)

re : clean all
