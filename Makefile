CC = g++
CFLAGS = -g -c -std=c++11
.C.o:
	$(CC) $(CFLAGS) $< -o $@

csim: csim.o Cache.o Simulate.o

Cache.o: Cache.h

csim.o: Cache.h Simulate.h

Simulate.o: Cache.h Simulate.h

#
# Clean the src dirctory
#
clean:
	rm -rf *.o
	rm -f csim
