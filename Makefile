default:
	rm -rf build
	mkdir build
	cc src/*.c -o build/speedtest -lcurl
build: 
	mkdir build
	cc src/*.c -o build/speedtest -lcurl
clean:
	rm -rf build