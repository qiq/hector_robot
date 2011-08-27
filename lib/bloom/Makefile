all: bloomb bloomt

bloomb: bloomb.cc NgramBloomFilter.h
	g++ -o bloomb bloomb.cc -lm -Wall -ggdb -O0

bloomt: bloomt.cc NgramBloomFilter.h
	g++ -o bloomt bloomt.cc -lm -Wall -ggdb -O0

clean:
	rm -f bloomb bloomt
