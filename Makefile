all:
	g++ cover_ean.cpp `pkg-config opencv zbar --libs --cflags` -o cover_ean
	
opencv4:
	g++ cover_ean.cpp `pkg-config opencv4 zbar --libs --cflags` -o cover_ean
