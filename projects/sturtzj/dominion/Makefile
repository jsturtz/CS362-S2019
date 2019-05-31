CFLAGS= -Wall -fpic -coverage -lm -std=c99
NUMTRIES=10 

rngs.o: rngs.h rngs.c
	gcc -c rngs.c -g  $(CFLAGS)

dominion.o: dominion.h dominion.c rngs.o
	gcc -c dominion.c -g  $(CFLAGS)

playdom: dominion.o playdom.c
	gcc -o playdom playdom.c -g dominion.o rngs.o $(CFLAGS)
#To run playdom you need to entere: ./playdom <any integer number> like ./playdom 10*/
testDrawCard: testDrawCard.c dominion.o rngs.o
	gcc  -o testDrawCard -g  testDrawCard.c dominion.o rngs.o $(CFLAGS)


#########

badTestDrawCard: badTestDrawCard.c dominion.o rngs.o
	gcc -o badTestDrawCard -g  badTestDrawCard.c dominion.o rngs.o $(CFLAGS)

testBuyCard: testDrawCard.c dominion.o rngs.o
	gcc -o testDrawCard -g  testDrawCard.c dominion.o rngs.o $(CFLAGS)

testAll: dominion.o testSuite.c
	gcc -o testSuite testSuite.c -g  dominion.o rngs.o $(CFLAGS)

interface.o: interface.h interface.c
	gcc -c interface.c -g  $(CFLAGS)

runtests: testDrawCard 
	./testDrawCard &> unittestresult.out
	gcov dominion.c >> unittestresult.out
	cat dominion.c.gcov >> unittestresult.out

unittestresults: dominion.o rngs.o unittest1.c unittest2.c unittest3.c unittest4.c cardtest1.c cardtest2.c cardtest3.c cardtest4.c
	echo "Result for running Dominion tests:" > unittestresults.out
	for n in 1 2 3 4; do \
	    echo "unittest$$n.c:" >> unittestresults.out ; \
	    gcc dominion.c rngs.c unittest$$n.c -o unittest$$n $(CFLAGS) ; \
	    ./unittest$$n >> unittestresults.out ; \
	    gcov dominion.c >> unittestresults.out ; \
	    echo "cardtest$$n.c:" >> unittestresults.out ; \
	    gcc dominion.c rngs.c cardtest$$n.c -o cardtest$$n $(CFLAGS) ; \
	    ./cardtest$$n >> unittestresults.out ; \
	    gcov dominion.c >> unittestresults.out ; \
	done

randomtestresults: dominion.o rngs.o randomtestadventurer.c randomtestcard1.c randomtestcard2.c
	echo "OUTPUT FOR RUNNING TESTS $(NUMTRIES)times:" > randomtestresults.out
	echo "randomtestcard1.c:" >> randomtestresults.out
	gcc dominion.c rngs.c randomtestcard1.c -o randomtestcard1 $(CFLAGS) -DNUMTRIES=$(NUMTRIES)
	./randomtestcard1 >> randomtestresults.out 
	gcov dominion.c >> randomtestresults.out
	echo "randomtestcard2.c:" >> randomtestresults.out
	gcc dominion.c rngs.c randomtestcard2.c -o randomtestcard2 $(CFLAGS) -DNUMTRIES=$(NUMTRIES)
	./randomtestcard2 >> randomtestresults.out
	gcov dominion.c >> randomtestresults.out
	echo "randomtestadventurer.c:" >> randomtestresults.out
	gcc dominion.c rngs.c randomtestadventurer.c -o randomtestadventurer $(CFLAGS) -DNUMTRIES=$(NUMTRIES)
	./randomtestadventurer >> randomtestresults.out
	gcov dominion.c >> randomtestresults.out
	echo "FUNCTION AND BRANCH COVERAGE OVERALL" >> randomtestresults.out
	gcov dominion.c -bf >> randomtestresults.out

player: player.c interface.o
	gcc -o player player.c -g  dominion.o rngs.o interface.o $(CFLAGS)

all: playdom player 

clean:
	rm -f *.o playdom.exe playdom player player.exe  *.gcov *.gcda *.gcno *.so *.out testDrawCard testDrawCard.exe
