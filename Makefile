all:
	g++ -std=c++11 -o Debug/subs *.cpp -levent -lpthread

clean:
	rm Debug/dictt
