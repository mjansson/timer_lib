SRC = timer.c
OBJ = $(SRC:.c=.o)
OUT = libtimer.a

CFLAGS = -O3
LDFLAGS = 

default: $(OUT)

.c.o:
	gcc $(CFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)

clean:
	rm -f $(OBJ) $(OUT)
