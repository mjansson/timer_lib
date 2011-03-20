SRC = timer.c
OBJ = $(SRC:.c=.o)
OUT = libtimer.a
TST = timertest
TSR = test.c
TOB = $(TSR:.c=.o)

CFLAGS = -O3
LDFLAGS = 

default: $(OUT) $(TST)

.c.o:
	gcc $(CFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)

$(TST): $(OUT) $(TOB)
	gcc $(CFLAGS) $(LDFLAGS) -o $(TST) $(TOB) $(OUT) -lrt

clean:
	rm -f $(OBJ) $(OUT) $(TOB) $(TST)
