all:
	g++ -Wall -Isrc/h src/cpp/*.cpp -lpng -lpthread -o raycast

	#-Ofast
	#-pg