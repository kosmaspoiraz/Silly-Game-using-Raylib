# paths
LIB = ../../lib
MODULES = ../../modules
INCLUDE = ../../include

# compiler
CC = gcc

# Compile options. Το -I<dir> λέει στον compiler να αναζητήσει εκεί include files
CFLAGS =  -Wall -Werror -g -I$(INCLUDE)
LDFLAGS = -lm

# Αρχεία .o
#Για state.c
OBJS = game.o $(MODULES)/state.o $(MODULES)/interface.o $(LIB)/k08.a $(LIB)/libraylib.a

#Για state_alt.c
#OBJS = game.o $(MODULES)/state_alt.o $(MODULES)/set_utils.o $(MODULES)/interface.o $(LIB)/k08.a $(LIB)/libraylib.a

# Το εκτελέσιμο πρόγραμμα
EXEC = game

# Παράμετροι για δοκιμαστική εκτέλεση
ARGS =

# Παράμετροι της βιβλιοθήκης raylib
include $(LIB)/libraylib.mk


$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC) $(LDFLAGS)
	@if [ -f $(EXEC).exe ]; then ln -sf $(EXEC).exe $(EXEC); fi

# Για να φτιάξουμε τα k08.a/libraylib.a τρέχουμε το make στο lib directory.
$(LIB)/%.a:
	$(MAKE) -C $(LIB) $*.a

clean:
	rm -f $(OBJS) $(EXEC)

run: $(EXEC)
	 ./$(EXEC) $(ARGS)

valgrind: $(EXEC)
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./$(EXEC) $(ARGS)