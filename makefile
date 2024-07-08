all:
	arm-linux-gcc -o ./bin/main ./src/*.c -I ./inc/ -L ./lib/ -lm -lpthread -lfont ./lib/libjpeg.so.9.1.0