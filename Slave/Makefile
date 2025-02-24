#***************************************************************************************#
#*       ,--,                                                                          *#
#*    ,---.'|                                                                          *#
#*    |   | :                        ,-.----.                  ___                     *#
#*    :   : |                        \    /  \               ,--.'|_                   *#
#*    |   ' :                        ;   :    \              |  | :,'        ,----,    *#
#*    ;   ; '              .--.--.   |   | .\ :              :  : ' :      .'   .`|    *#
#*    '   | |__   ,---.   /  /    '  .   : |: |   ,--.--.  .;__,'  /    .'   .'  .'    *#
#*    |   | :.'| /     \ |  :  /`./  |   |  \ :  /       \ |  |   |   ,---, '   ./     *#
#*    '   :    ;/    /  ||  :  ;_    |   : .  / .--.  .-. |:__,'| :   ;   | .'  /      *#
#*    |   |  ./.    ' / | \  \    `. ;   | |  \  \__\/: . .  '  : |__ `---' /  ;--,    *#
#*    ;   : ;  '   ;   /|  `----.   \|   | ;\  \ ," .--.; |  |  | '.'|  /  /  / .`|    *#
#*    |   ,/   '   |  / | /  /`--'  /:   ' | \.'/  /  ,.  |  ;  :    ;./__;     .'     *#
#*    '---'    |   :    |'--'.     / :   : :-' ;  :   .'   \ |  ,   / ;   |  .'        *#
#*              \   \  /   `--'---'  |   |.'   |  ,     .-./  ---`-'  `---'            *#
#*               `----'              `---'      `--`---'                               *#
#***************************************************************************************#

# ################ #
#*    COULEURS    *#
# ################ #

CLASSIC	= \e[0m
WHITE	= \e[38;2;238;232;213m
BOLD	= \e[1m
# BLACK	= \e[38;2;0;0;0m
BLACK	= \e[30m
RED   	= \e[31m
GREEN  	= \e[32m
BLUE 	= \e[38;2;0;36;148m
PURPLE 	= \e[38;2;211;54;130m
CYAN   	= \e[36m
YELLOW 	= \e[38;2;250;189;0m

# ############### #
#*   VARIABLES   *#
# ############### #

NAME = slave

CC = @cc

LMAKE = @make --no-print-directory -C

SRCS =	./../global/ratz.c \
		./slave.c

OBJS_DIR = .objs

OBJS = $(SRCS:%.c=$(OBJS_DIR)/%.obj)

DEPS = $(OBJS:%.obj=%.d)

CFLAGS = -Wall -Wextra -Wno-unused-result -Wno-format -MMD -g3

LIB = -lc

INCLUDES = -I. -I../global/

# ############## #
#*    REGLES    *#
# ############## #

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIB) -o $@
	@echo "$(BOLD)$(CYAN)Exécutable $(NAME) créé avec succès!$(CLASSIC)"

$(OBJS_DIR)/%.obj: %.c
	@mkdir -p $(@D)
	@echo "$(GREEN)🗸 Compilation $(BOLD)$(YELLOW)$<$(CLASSIC)"
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

clean:
	@echo "$(BOLD)$(RED)"
	rm $(OBJS_DIR) -rf
	@echo "$(CLASSIC)"

fclean: clean
	@echo "$(BOLD)$(RED)"
	rm $(NAME) -f
	@echo "$(BOLD)$(GREEN)Tout a été supprimé... 🗑️\n$(CLASSIC)"

re: fclean all

.PHONY: all clean fclean re
-include $(DEPS)
