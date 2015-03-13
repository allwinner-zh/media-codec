
all:
	make -C LIBRARY
	make -C openmax

clean:
	make clean -C LIBRARY
	make clean -C openmax

cleanall:
	make cleanall -C LIBRARY
	make cleanall -C openmax

release:
	mkdir ./out
	cp ./LIBRARY/EXTERNAL/lib32/lgnueabihf/libvencoder.so ./out/
	cp ./LIBRARY/EXTERNAL/lib32/lgnueabihf/libvdecoder.so ./out/
	cp ./LIBRARY/VE/libVE.so ./out/
	cp ./LIBRARY/MEMORY/libMemAdapter.so ./out/
	cp ./openmax/libOmxCore.so ./out/
	cp ./openmax/libOmxVdec.so ./out/
	cp ./openmax/libOmxVenc.so ./out/

