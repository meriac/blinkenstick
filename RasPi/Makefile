TARGET:=blinkenstick
SOURCES:=main.cpp  rgb.cpp

$(TARGET): $(SOURCES)
	g++ -Wall -Werror -O3 -o $(TARGET) $^ -lpng

install:
	sudo apt-get install g++ libpng12-0 libpng12-dev libpng++-dev

clean:
	rm -f $(TARGET)
