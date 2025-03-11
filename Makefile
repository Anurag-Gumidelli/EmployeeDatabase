TARGET = bin/dbview
SRC = $(wildcard src/*.c)
OBJ= $(patsubst src/%.c, obj/%.o, $(SRC))

run: clean default
	$(TARGET) -n -f databaseFiles/dbFile.db
	$(TARGET) -f databaseFiles/dbFile.db -l
	$(TARGET) -f databaseFiles/dbFile.db -a "Sherlock H.,221B Baker Street,150" -l
	$(TARGET) -f databaseFiles/dbFile.db -a "Dr. Watson,221B Baker Street,50" -l
	$(TARGET) -f databaseFiles/dbFile.db -a "James Moriarty,Reichenbach Falls,10" -l
	$(TARGET) -f databaseFiles/dbFile.db -r "Dr. Watson" -l

default: $(TARGET)

clean:
	rm -f obj/*.o
	rm -f bin/*
	rm -f databaseFiles/*.db

$(TARGET): $(OBJ)
	gcc -g -o $@ $?

obj/%.o : src/%.c
	gcc -g -c $< -o $@ -Iinclude
