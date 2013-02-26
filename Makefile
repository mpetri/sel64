
all: clean sel64 run

sel64:
	g++ -O3 -msse4.2 -o sel64 sel64.c 

run:
	./sel64

clean:
	rm -f sel64
