all:
	g++ cover_ean.cpp `pkg-config opencv zbar --libs --cflags` -o cover_ean
