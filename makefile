all:
	arm-linux-gcc -o ./bin/main ./src/*.c -I ./inc -L ./lib -lfont -lm -lpthread