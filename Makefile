ifeq ($(OS), Windows_NT) # Windows
    LIBS += -lws2_32
    CURSES = pdcurses
else
    CURSES = curses
endif

all: obj bin AChat_server AChat_client

obj/%.o: src/%.c
	gcc $(CFLAGS) -c $< -o $@

AChat_server: obj/AChat_server.o
	gcc $(CFLAGS) $? -o bin/$@ $(LIBS)

AChat_client: LIBS += -lpthread -l$(CURSES)
AChat_client: obj/AChat_client.o
	gcc $(CFLAGS) $? -o bin/$@ $(LIBS)

obj:
	@mkdir obj

bin:
	@mkdir bin

clean:
	rm -rf obj bin