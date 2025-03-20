
TARGET:=GradientsSP

SRC:=../GradientsSP

all: $(TARGET)

run: $(TARGET)
	@./$(TARGET) ; echo $$?

%: $(SRC)/%.ino
#	$(CXX) -x c++ -o $@ -include missing.h $^ -I . missing.cpp -Wall -Werror
	$(CXX) -g -O0 -x c++ -o $@ -I. -Wall -Werror -include missing.h $^ `sdl2-config --cflags --libs`

clean:
	$(RM) $(TARGET)
