TARGET = bin/dbview
SRC = $(wildcard src/*.c)
OBJ= $(patsubst src/%.c, obj/%.o, $(SRC))

run: clean default
	$(TARGET) -n -f databaseFiles/dbFile.db
	$(TARGET) -f databaseFiles/dbFile.db

default: $(TARGET)

clean:
	rm -f obj/*.o
	rm -f bin/*
	rm -f databaseFiles/*.db

$(TARGET): $(OBJ)
	gcc -g -o $@ $?

obj/%.o : src/%.c
	gcc -g -c $< -o $@ -Iinclude
