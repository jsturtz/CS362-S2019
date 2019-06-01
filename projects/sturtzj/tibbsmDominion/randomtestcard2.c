#include "dominion.h"
#include "dominion_helpers.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include "rngs.h"
#include <stdlib.h>

#ifndef NUMTRIES
#define NUMTRIES 1000000
#endif

#define STR(x) #x
#ifndef ASSERT_TRUE
#define ASSERT_TRUE(x) if (!(x)) { printf("Assert failed: (%s), line %d.\n", STR(x), __LINE__);}
#endif 

// Will produce a random kingdom except first card will be card parameter
void randomKingdom(int k[10], int card) {
  int i = 0; 
  k[0] = card;

  while (i < 9) {
    int randCard = rand() % (treasure_map+1);
    if (randCard != card) {
      k[i+1] = randCard;
      i++;
    }
  }
}

// will produce a random deck for player with up to maxSize elements
void randomDeck(struct gameState* state, int player, int maxSize) {
  int deckSize = rand() % (maxSize + 1);
  state->deckCount[player] = deckSize;

  // treasure_map is the last valid card in the ENUM in dominion.h 
  for (int i = 0; i < deckSize; i++) {
    state->deck[player][i] = rand() % (treasure_map+1);
  } 
}

// will produce a random hand for player with up to maxSize elements
void randomHand(struct gameState* state, int player, int maxSize) {
  int handSize = rand() % (maxSize + 1);
  state->handCount[player] = handSize;

  for (int i = 0; i < handSize; i++) {
    state->hand[player][i] = rand() % (treasure_map+1);
  }
}

// will produce a random discard for player wth up to maxSize elements
void randomDiscard(struct gameState* state, int player, int maxSize) {
  int discardSize = rand() % (maxSize + 1);
  state->discardCount[player] = discardSize;

  for (int i = 0; i < discardSize; i++) {
    state->discard[player][i] = rand() % (treasure_map+1);
  }
}

// compare function used as subroutine for isSameSet (C does not permit functions to be defined inside functions)
int isGreaterThan(const void* x, const void* y) {
  int xInt = *(const int*)x;
  int yInt = *(const int*)y;
  return xInt > yInt;
}

// will compare two arrays of ints to see if they form identical sets
int isSameSet(int* A1, int* A2, int size, int (*compare)(const void*, const void*)) {
  
  qsort(A1, size, sizeof(int), compare);
  qsort(A2, size, sizeof(int), compare);
  if (memcmp(A1, A2, sizeof(int) * size) == 0)
    return 1;
  else
    return 0;
}

int main() {

  clock_t start, end;
  start = clock();  // ensure time is under 5 minutes

  int i, j;
  int numPlayers, player;  
  int handpos;

  int k[10];        // holds kingdom cards (always has adventurer)
  struct gameState G, testG;

  srand(time(0));

  // loop numtries times, set in precompiler
  i = 0;
  while (i < NUMTRIES) {

    int seed = rand();
    
    // initialize random game with random Kingdom except smithy is guaranteed
    randomKingdom(k, smithy);
    
    // initialize a game state and player cards
    numPlayers = rand() % 3 + 2;                // will always produce values from 2 - 4
    initializeGame(numPlayers, k, seed, &G);    // helpful to set initial values
    G.playedCardCount = 0;                      // initializeGame fails to initialize this value to 0, causing segfaults.

    /* // set random deck, hand, and discard for players */
    for (j = 0; j < numPlayers; j++) {
      
      randomDeck(&G, j, MAX_DECK);
      randomHand(&G, j, MAX_HAND - 1);
      G.hand[j][G.handCount[j]] = council_room;
      G.handCount[j]++;

      // discard + deck shouldn't exceed MAX_DECK
      randomDiscard(&G, j, MAX_DECK - G.deckCount[j]);
    }

    // call councilRoomEffect
    memcpy(&testG, &G, sizeof(struct gameState));
    player = rand() % numPlayers;
    handpos = G.handCount[player] - 1;
    councilRoomCard(&testG, player, handpos);

    // TESTS START HERE
    ASSERT_TRUE(testG.handCount[player] == G.handCount[player] + 3);
    ASSERT_TRUE(testG.playedCards[testG.playedCardCount-1] == council_room);

    // hand should be the same for all cards up to council_room
    ASSERT_TRUE(memcmp(testG.hand[player], G.hand[player], G.handCount[player] - 1) == 0);

    // total card count should not have changed (i.e. no gaining or trashing)
    ASSERT_TRUE(G.handCount[player] + G.discardCount[player] + G.deckCount[player] + G.playedCardCount == 
                testG.handCount[player] + testG.discardCount[player] + testG.deckCount[player] + testG.playedCardCount);

    // tests overall gamestate: nothing should have changed for non-player variables except numBuys
    ASSERT_TRUE(G.numPlayers == testG.numPlayers);
    ASSERT_TRUE(memcmp(G.supplyCount, testG.supplyCount, (treasure_map+1) * sizeof(int)) == 0);
    ASSERT_TRUE(memcmp(G.embargoTokens, testG.embargoTokens, (treasure_map+1) * sizeof(int)) == 0);
    ASSERT_TRUE(G.outpostPlayed == testG.outpostPlayed); 
    ASSERT_TRUE(G.outpostTurn == testG.outpostTurn); 
    ASSERT_TRUE(G.whoseTurn == testG.whoseTurn); 
    ASSERT_TRUE(G.phase == testG.phase); 
    ASSERT_TRUE(G.numActions == testG.numActions); 
    ASSERT_TRUE(G.coins == testG.coins); 
    ASSERT_TRUE(G.numBuys + 1 == testG.numBuys); 

    // tests gamestate for other players
    for (j = 0; j < numPlayers; j++) {
      if (j != player) {
        ASSERT_TRUE(G.handCount[j] + 1 == testG.handCount[j]);
  
        // Assertions depend on whether deck must be shuffled for draw, i.e. deckCount == 0
        if (G.deckCount[j] != 0) {
          ASSERT_TRUE(G.deckCount[j] == testG.deckCount[j] + 1);
          ASSERT_TRUE(G.discardCount[j] == testG.discardCount[j]);
         }
        else
          ASSERT_TRUE(testG.discardCount[j] == 0);
      }
    }
    
    // All cards left in deck should be in hand
    int topDeck[4];
    int topHand[4];
    memset(topDeck, '\0', 4);
    memset(topHand, '\0', 4);

    // check as many cards as possible from topdeck (cannot check after shuffle because of indeterminism)
    int numSet;
    if (G.deckCount[player] >= 4)
      numSet = 4;
    else
      numSet = G.deckCount[player];
    
    // copy cards into arrays, call isSameSet
    memcpy(topDeck, &G.deck[player][G.deckCount[player] - numSet], sizeof(int) * numSet);
    memcpy(topHand, &testG.hand[player][testG.handCount[player] - numSet], sizeof(int) * numSet);
    ASSERT_TRUE(isSameSet(topDeck, topHand, numSet, isGreaterThan));
    
    // END TESTS
    i++;
  }

  end = clock();
  printf("Minutes elapsed: %8.2lf\n", ((double) (end - start)) / CLOCKS_PER_SEC / 60);
  return 0;
}

