CC_OPTS=-pedantic -Wall


ATEST_LIBS=-L $(ATEST_PREFIX) -latest -latcheck
ATEST_FLAGS=-I $(ATEST_PREFIX)

DEBUG_FLAG=$(if $(N_DEBUG),-g -DN_DEBUG,)
TEST_FLAG=$(if $(N_TEST),-DN_TEST,)

CC_FLAGS=$(CC_OPTS) $(DEBUG_FLAG) $(TEST_FLAG) $(CFLAGS) $(ATEST_FLAGS)

COMMON_SRCS=$(wildcard src/common/*.c)
COMMON_OBJS=$(COMMON_SRCS:src/common/%.c=build/nuvm/common/%.o)

EVAL_SRCS=$(wildcard src/eval/*.c)
EVAL_OBJS=$(EVAL_SRCS:src/eval/%.c=build/nuvm/eval/%.o)

ASM_SRCS=$(wildcard src/asm/*.c)
ASM_OBJS=$(ASM_SRCS:src/asm/%.c=build/nuvm/asm/%.o)

TEST_HELPER_SRCS=$(wildcard test/*.c)


COMMON_TSTS=$(wildcard test/common/*.c)
COMMON_RUNS=$(COMMON_TSTS:test/common/%.c=build/test/common/%.run)

EVAL_TSTS=$(wildcard test/eval/*.c)
EVAL_RUNS=$(EVAL_TSTS:test/eval/%.c=build/test/eval/%.run)

ASM_TSTS=$(wildcard test/asm/*.c)
ASM_RUNS=$(ASM_TSTS:test/asm/%.c=build/test/asm/%.run)

TEST_CC_FLAGS=-I "src/"
TEST_CC_LIBS=-L "dist/" -lnuvm-eval -lnuvm-asm -lnuvm-common $(ATEST_LIBS)

.PHONY: all
all: build test

.PHONY: build
build: dist/libnuvm-common.a dist/libnuvm-eval.a dist/libnuvm-asm.a

.PHONY: test
test: $(COMMON_RUNS) $(EVAL_RUNS) $(ASM_RUNS)


-include $(patsubst %.o, %.d, $(COMMON_OBJS) $(EVAL_OBJS) $(ASM_OBJS))

dist/libnuvm-common.a: $(COMMON_OBJS)
	@ar rcs $@ $^


dist/libnuvm-eval.a: $(EVAL_OBJS)
	@ar rcs $@ $^


dist/libnuvm-asm.a: $(ASM_OBJS)
	@ar rcs $@ $^


build/nuvm/%.o: src/%.c
	@$(CC) -c $< -o $@ $(CC_FLAGS)
	@$(CC) -MM -MT $@ $(CC_FLAGS) $< > $(@:.o=.d)


build/test/%.run: test/%.c \
                  dist/libnuvm-eval.a \
                  dist/libnuvm-asm.a \
                  dist/libnuvm-common.a
	@$(CC) -o $@ $< $(TEST_HELPER_SRCS) $(CC_FLAGS) \
                    $(TEST_CC_FLAGS) $(TEST_CC_LIBS)


.PHONY: clean
clean: clean-tests
	@rm -f build/nuvm/common/*.o build/nuvm/common/*.d
	@rm -f build/nuvm/eval/*.o build/nuvm/eval/*.d
	@rm -f build/nuvm/asm/*.o build/nuvm/asm/*.d
	@rm -f dist/*

.PHONY: clean-tests
clean-tests:
	@rm -f build/test/common/*.run
	@rm -f build/test/eval/*.run
