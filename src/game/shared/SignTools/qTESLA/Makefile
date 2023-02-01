CC=gcc
ifeq "$(CC)" "gcc"
    COMPILER=gcc
else ifeq "$(CC)" "clang"
    COMPILER=clang
endif

ARCHITECTURE=_AMD64_
ifeq "$(ARCH)" "x64"
    ARCHITECTURE=_AMD64_
else ifeq "$(ARCH)" "x86"
    ARCHITECTURE=_X86_
else ifeq "$(ARCH)" "ARM"
    ARCHITECTURE=_ARM_
    ARM_SETTING=-lrt
else ifeq "$(ARCH)" "ARM64"
    ARCHITECTURE=_ARM64_
    ARM_SETTING=-lrt
endif

AR=ar rcs
RANLIB=ranlib

CFLAGS = -std=gnu11 -O3 -D $(ARCHITECTURE) -D __LINUX__ -fomit-frame-pointer
LDFLAGS=-lm -L/usr/lib/ -lssl -lcrypto 

ifeq "$(CC)" "gcc"
    CFLAGS+= -march=native
endif

DFLAG=
ifeq "$(STATS)" "TRUE"
    DFLAG=-DSTATS
endif

OBJECTS_p_I = objs_p_I/sign.o objs_p_I/pack.o objs_p_I/sample.o objs_p_I/gauss.o objs_p_I/poly.o objs_p_I/consts.o objs/fips202.o objs/random.o
OBJECTS_p_III = objs_p_III/sign.o objs_p_III/pack.o objs_p_III/sample.o objs_p_III/gauss.o objs_p_III/poly.o objs_p_III/consts.o objs/fips202.o objs/random.o
SOURCE_TEST = tests/cpucycles.c tests/test_qtesla.c
SOURCE_KATS_GEN  = tests/rng.c tests/PQCgenKAT_sign.c
SOURCE_KATS_TEST = tests/rng.c tests/PQCtestKAT_sign.c

all: lib_p_I lib_p_III tests

objs_p_I/%.o: %.c
	@mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $(DFLAG) -D _qTESLA_p_I_ $< -o $@

objs_p_III/%.o: %.c
	@mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $(DFLAG) -D _qTESLA_p_III_ $< -o $@

objs/random.o: random/random.c
	@mkdir -p $(@D)
	$(CC) -c $(CFLAGS) random/random.c -o objs/random.o

objs/fips202.o: sha3/fips202.c
	@mkdir -p $(@D)
	$(CC) -c $(CFLAGS) sha3/fips202.c -o objs/fips202.o

lib_p_I: $(OBJECTS_p_I)
	rm -rf lib_p_I
	mkdir lib_p_I
	$(AR) lib_p_I/libqtesla.a $^
	$(RANLIB) lib_p_I/libqtesla.a

lib_p_III: $(OBJECTS_p_III)
	rm -rf lib_p_III
	mkdir lib_p_III
	$(AR) lib_p_III/libqtesla.a $^
	$(RANLIB) lib_p_III/libqtesla.a

tests: lib_p_I lib_p_III
	$(CC) $(CFLAGS) -L./lib_p_I -D _qTESLA_p_I_ $(SOURCE_TEST) $(DFLAG) -lqtesla $(LDFLAGS) -o test_qtesla-p-I $(ARM_SETTING)
	$(CC) $(CFLAGS) -L./lib_p_I -D _qTESLA_p_I_ $(SOURCE_KATS_GEN) $(DFLAG) -lqtesla $(LDFLAGS) -o PQCgenKAT_sign-p-I $(ARM_SETTING)
	$(CC) $(CFLAGS) -L./lib_p_I -D _qTESLA_p_I_ $(SOURCE_KATS_TEST) $(DFLAG) -lqtesla $(LDFLAGS) -o PQCtestKAT_sign-p-I $(ARM_SETTING)
	$(CC) $(CFLAGS) -L./lib_p_III -D _qTESLA_p_III_ $(SOURCE_TEST) $(DFLAG) -lqtesla $(LDFLAGS) -o test_qtesla-p-III $(ARM_SETTING)
	$(CC) $(CFLAGS) -L./lib_p_III -D _qTESLA_p_III_ $(SOURCE_KATS_GEN) $(DFLAG) -lqtesla $(LDFLAGS) -o PQCgenKAT_sign-p-III $(ARM_SETTING)
	$(CC) $(CFLAGS) -L./lib_p_III -D _qTESLA_p_III_ $(SOURCE_KATS_TEST) $(DFLAG) -lqtesla $(LDFLAGS) -o PQCtestKAT_sign-p-III $(ARM_SETTING)

.PHONY: clean

clean:
	rm -rf objs* lib* test_qtesla-* PQCgenKAT_sign-* PQCtestKAT_sign-* PQCsignKAT_qTesla*
