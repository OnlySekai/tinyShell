SRC	=	$(wildcard *.c)
OBJ	=	${SRC:.c=.o}
HEA = $(wildcard *.h)

test: $(OBJ)

run: sh
	./sh

%.c:	$(HEA)

%.o:%.c
	gcc -D_GNU_SOURCE -c -Wall -O2 -o $@ $<

sh: $(OBJ) 
	gcc -o $@ $(OBJ)

clean:
	rm $(OBJ)    
