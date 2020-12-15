GCC = gcc -g  -pthread -O2 -std=gnu11 -Wall -Wextra  -fsanitize=bool -fsanitize=enum -fsanitize=float-cast-overflow -fsanitize=float-divide-by-zero -fsanitize=integer-divide-by-zero -fsanitize=leak -fsanitize=nonnull-attribute -fsanitize=null -fsanitize=object-size -fsanitize=return -fsanitize=returns-nonnull-attribute -fsanitize=shift -fsanitize=signed-integer-overflow -fsanitize=unreachable -fsanitize=vla-bound -fsanitize=vptr -lm -lrt

all:
	$(GCC) bridge.c -o BR
print:
	$(GCC) bridge.c -DPRINT -o  BR