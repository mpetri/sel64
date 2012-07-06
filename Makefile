
all: clean sel64 sel64dbg

sel64:
	g++ -O3 -I ~/include -L ~/lib/ -msse4.2 -o sel64 sel64.c -l sdsl

sel64dbg:
	g++ -g -I ~/include -L ~/lib/ -msse4.2 -o sel64dbg sel64.c -l sdsl

clean:
	rm -f sel64 sel64dbg