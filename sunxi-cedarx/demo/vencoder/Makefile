
CC=arm-linux-gnueabihf-gcc

all:
	$(CC) VideoTest.c \
			-L/home/bz/tmp/out/lib/full-package-name/ \
			-lcedar_vencoder -lcedar_common -lcedar_base\
			-I../../libs/base/include/ \
			-I../../libs/vencoder/include/ \

clean:
	rm a.out

