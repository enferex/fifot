APP = fifot
CFLAGS = -Wall -std=c99 -pedantic -g3 -O3

$(APP): main.c
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	$(RM) $(APP)
