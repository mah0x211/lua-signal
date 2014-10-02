TARGET=$(PACKAGE).$(LIB_EXTENSION)
SRC=signal.c
OBJ=signal.o


all: preprocess $(TARGET)

$(TARGET):
	$(CC) $(CFLAGS) $(WARNINGS) $(CPPFLAGS) -o $(OBJ) -c $(SRC)
	$(CC) -o $(TARGET) $(LDFLAGS) $(OBJ)

preprocess:
	lua ./signogen.lua

install:
	mkdir -p $(LIBDIR)
	cp $(TARGET) $(LIBDIR)
	rm -f $(OBJ) $(TARGET)

