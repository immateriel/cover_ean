CDEPS = `pkg-config opencv zbar --libs --cflags || pkg-config opencv4 zbar --libs --cflags`
all:
	g++ cover_ean.cpp $(CDEPS) -o cover_ean
