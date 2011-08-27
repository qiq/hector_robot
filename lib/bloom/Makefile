all: bloomb bloomt

bloomb: bloomb.cc NgramBloomFilter.h
	g++ -o bloomb bloomb.cc -lm -Wall -ggdb -O3

bloomt: bloomt.cc NgramBloomFilter.h
	g++ -o bloomt bloomt.cc -lm -Wall -ggdb -O3

clean:
	rm -f bloomb bloomt
