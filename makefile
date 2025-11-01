CC := clang++
CCFLAG := -std=c++17 -Ilib/glm -Ilib/include -I/opt/homebrew/include
LDFLAGS := -L/opt/homebrew/lib -lglfw -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

all: app 

app: main.o glad.o
	$(CC) $(CCFLAG) -o app main.o glad.o $(LDFLAGS)

glad.o: lib/src/glad.c
	$(CC) $(CCFLAG) -c lib/src/glad.c -o glad.o

%.o: %.cpp
	$(CC) $(CCFLAG) -c $< -o $@ 

.PHONY: clean
clean: 
	rm -f app main.o glad.o 

