CC=gcc
CFLAGS=-Wall
SOURCE=os.hw3.2020069027.woojinshin.c
EXE=os.hw3.2020069027.woojinshin
all: $(EXE)

$(EXE): $(SOURCE)
	$(CC) -o $(EXE) $(SOURCE) $(CFLAGS)

clean:
	rm -rf $(OBJ) $(EXE)
