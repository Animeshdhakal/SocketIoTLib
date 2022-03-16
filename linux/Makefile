CXX = g++
CXXFLAGS = -lssl -lcrypto
SRC = main.cpp ../SocketIoT/SocketIoTSpec.cpp ../SocketIoT/SocketIoTHandler.cpp
INCLUDES = -I../SocketIoT

build:
	$(CXX) $(SRC) $(INCLUDES) $(CXXFLAGS) -o SocketIoT
