all:
	g++ cover_ean.cpp `pkg-config opencv4 zbar --libs --cflags` -o cover_ean
	