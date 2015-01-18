all: agent server move

agent:
	cd Agent/src/ && make

server:
	cd Server/src/ && make

move:
	mv Agent/src/Agent bin
	mv Server/src/server bin

.PHONY: move

