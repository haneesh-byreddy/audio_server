CXX = g++
FLAGS = -std=c++17 -w
TARGET = build
INC_1 = -I include
INC_2 = -I /usr/include
INCLUDES = $(INC_1) $(INC_2)
LFLAGS = -lpulse -lpulse-simple
LIB = -L /usr/lib/x86_64-linux-gnu/
SRC = src
OBJ = obj

all: directories ${TARGET}/server_app ${TARGET}/client_app

directories:
	@mkdir -p ${TARGET} ${OBJ}

${TARGET}/server_app: ${OBJ}/server.o ${OBJ}/server_app.o
	@${CXX} ${FLAGS} ${INCLUDES} ${LIB} ${OBJ}/server.o ${OBJ}/server_app.o -o $@ ${LFLAGS}

${OBJ}/server_app.o: ${SRC}/server_app.cpp
	@${CXX} ${FLAGS} ${INCLUDES} -c $< -o $@

${OBJ}/server.o: ${SRC}/server.cpp
	@${CXX} ${FLAGS} ${INCLUDES} -c $< -o $@

${TARGET}/client_app: ${OBJ}/client.o ${OBJ}/client_app.o
	@${CXX} ${FLAGS} ${INCLUDES} ${LIB} ${OBJ}/client.o ${OBJ}/client_app.o -o $@ ${LFLAGS}

${OBJ}/client_app.o: ${SRC}/client_app.cpp
	@${CXX} ${FLAGS} ${INCLUDES} -c $< -o $@

${OBJ}/client.o: ${SRC}/client.cpp
	@${CXX} ${FLAGS} ${INCLUDES} -c $< -o $@

clean:
	@rm -rf ${OBJ}/* ${TARGET}/*
