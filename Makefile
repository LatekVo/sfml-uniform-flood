LINKS= -lsfml-graphics -lsfml-window -lsfml-system

floodapp: *.o
	g++ $? $(LINKS) -o $@ 

main.o: main.cpp
	g++ -c $? -o $@

