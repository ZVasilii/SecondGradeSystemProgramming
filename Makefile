GCC = gcc -g -std=gnu11 -Wall -Wextra  -fsanitize=bool -fsanitize=enum -fsanitize=float-cast-overflow -fsanitize=float-divide-by-zero -fsanitize=integer-divide-by-zero -fsanitize=leak -fsanitize=nonnull-attribute -fsanitize=null -fsanitize=object-size -fsanitize=return -fsanitize=returns-nonnull-attribute -fsanitize=shift -fsanitize=signed-integer-overflow -fsanitize=unreachable -fsanitize=vla-bound -fsanitize=vptr -lm -lrt

all:
	@$(GCC) race.c -o RACE

time:
	@$(GCC) -DTIME race.c -o RACE
	
