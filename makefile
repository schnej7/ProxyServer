#Jerry Schneider
all:
	gcc -Wall -o server ./proxy.c ./string_manip.c ./networkIO.c 

debug:
	gcc -Wall -g -o server ./proxy.c ./string_manip.c ./networkIO.c 

clean:
	rm server

