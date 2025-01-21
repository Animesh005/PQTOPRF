TFHE_PREFIX = /usr/local
TFHE_OPTIONS = C_INCLUDE_PATH=$$C_INCLUDE_PATH:${TFHE_PREFIX}/include CPLUS_INCLUDE_PATH=$$CPLUS_INCLUDE_PATH:${TFHE_PREFIX}/include LIBRARY_PATH=$$LIBRARY_PATH:${TFHE_PREFIX}/lib LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:${TFHE_PREFIX}/lib
TFHE_LIB = -ltfhe-spqlios-fma
BLAS_LIB = -lopenblas

all: bin/keygen bin/encrypt bin/compute bin/decrypt bin/keysplit bin/bootstrap_test bin/bootstrap_tlwe \
bin/bootstrap_modules bin/profile bin/libthfhe.a bin/toprf_eval

bin/libthfhe.a: src/libthfhe.cpp src/thfhe.hpp
	rm -f bin/libthfhe.a
	${TFHE_OPTIONS} g++ -g -fopenmp -O3 -c src/libthfhe.cpp -o bin/libthfhe.o ${TFHE_LIB} ${BLAS_LIB}
	ar -crs bin/libthfhe.a bin/libthfhe.o

bin/profile: src/profile.cpp
	${TFHE_OPTIONS} g++ -g -O3 src/profile.cpp -o bin/profile ${TFHE_LIB}

bin/bootstrap_modules: src/bootstrap_modules.cpp
	${TFHE_OPTIONS} g++ -g src/bootstrap_modules.cpp -o bin/bootstrap_modules ${TFHE_LIB}

bin/bootstrap_tlwe: src/bootstrap_tlwe.cpp
	${TFHE_OPTIONS} g++ -g src/bootstrap_tlwe.cpp -o bin/bootstrap_tlwe ${TFHE_LIB}

bin/share.o: src/share.cpp
	g++ -g -c src/share.cpp -o bin/share.o

bin/threshold_decryption_functions.o: src/threshold_decryption_functions.cpp
	g++ -g -O3 -c -fopenmp src/threshold_decryption_functions.cpp -o bin/threshold_decryption_functions.o ${BLAS_LIB}

bin/bootstrap_test: src/bootstrap_test.cpp
	${TFHE_OPTIONS} g++ -g src/bootstrap_test.cpp -o bin/bootstrap_test ${TFHE_LIB}

bin/keygen: src/KeyGen.cpp
	${TFHE_OPTIONS} g++ src/KeyGen.cpp -o bin/keygen ${TFHE_LIB}

bin/encrypt: src/Encrypt.cpp
	${TFHE_OPTIONS} g++ src/Encrypt.cpp -o bin/encrypt ${TFHE_LIB}

bin/compute: src/Compute.cpp
	${TFHE_OPTIONS} g++ src/Compute.cpp -o bin/compute ${TFHE_LIB}

bin/decrypt: src/Decrypt.cpp
	${TFHE_OPTIONS} g++ src/Decrypt.cpp -o bin/decrypt ${TFHE_LIB}

bin/keysplit: src/KeySplit.cpp
	${TFHE_OPTIONS} g++ src/KeySplit.cpp -o bin/keysplit ${TFHE_LIB}

bin/tlweprofile: src/TlweProfile.cpp
	${TFHE_OPTIONS} g++ -g src/TlweProfile.cpp -o bin/tlweprofile ${TFHE_LIB}

bin/toprf_eval: bin/threshold_decryption_functions.o src/toprf_eval.cpp
	${TFHE_OPTIONS} g++ -g -fopenmp src/toprf_eval.cpp bin/threshold_decryption_functions.o -o bin/toprf_eval ${TFHE_LIB} ${BLAS_LIB}

.PHONY: clean
clean:
	rm -rf bin/* test/*

.PHONY: install
install: bin/libthfhe.a
	cp src/thfhe.hpp /usr/local/include
	cp bin/libthfhe.a /usr/local/lib
