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
 
// Will produce a random kingdom except first card will be adventurer
void randomKingdom(int k[10]) {
  int i = 0; 
  k[0] = adventurer;

  while (i < 9) {
    int randCard = rand() % (treasure_map+1);
    if (randCard != adventurer) {
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
    
    // initialize random game with random Kingdom except Adventurer is guaranteed
    randomKingdom(k);
    
    // initialize a game state and player cards
    numPlayers = rand() % 3 + 2;                // will always produce values from 2 - 4
    initializeGame(numPlayers, k, seed, &G);    // helpful to set initial values
    G.playedCardCount = 0;                      // initializeGame fails to initialize this value to 0, causing segfaults.

    /* // set random deck, hand, and discard for players */
    for (j = 0; j < numPlayers; j++) {
     
      // randSpot needed to guarantee two treasures in deck 
      randomDeck(&G, j, MAX_DECK);
      int randSpot = rand() % (G.deckCount[j] - 1);
      G.deck[j][randSpot] = copper;
      G.deck[j][randSpot+1] = copper;

      randomHand(&G, j, MAX_HAND - 1);
      G.hand[j][G.handCount[j]] = adventurer;
      G.handCount[j]++;

      // discard + deck shouldn't exceed MAX_DECK
      randomDiscard(&G, j, MAX_DECK - G.deckCount[j]);
    }

    // call adventurerCard
    memcpy(&testG, &G, sizeof(struct gameState));
    player = rand() % numPlayers;
    handpos = G.handCount[player] - 1;
    adventurerCard(&testG, player);

    // TESTS
    ASSERT_TRUE(testG.playedCardCount == G.playedCardCount + 1);
    ASSERT_TRUE(testG.playedCards[testG.playedCardCount-1] == adventurer);
    ASSERT_TRUE(testG.handCount[player] == G.handCount[player] + 1);
    ASSERT_TRUE(testG.hand[player][testG.handCount[player] - 1] <= gold && testG.hand[player][testG.handCount[player] - 1] >= copper);
    ASSERT_TRUE(testG.hand[player][testG.handCount[player] - 2] <= gold && testG.hand[player][testG.handCount[player] - 2] >= copper);

    // hand should be the same if adventurer played is last card in hand
    ASSERT_TRUE(memcmp(testG.hand[player], G.hand[player], G.handCount[player] - 1) == 0);

    // total card count should not have changed
    ASSERT_TRUE(G.handCount[player] + G.discardCount[player] + G.deckCount[player] + G.playedCardCount == 
                testG.handCount[player] + testG.discardCount[player] + testG.deckCount[player] + testG.playedCardCount);

    // tests overall gamestate: nothing should have changed 
    ASSERT_TRUE(G.numPlayers == testG.numPlayers);
    ASSERT_TRUE(memcmp(G.supplyCount, testG.supplyCount, (treasure_map+1) * sizeof(int)) == 0);
    ASSERT_TRUE(memcmp(G.embargoTokens, testG.embargoTokens, (treasure_map+1) * sizeof(int)) == 0);
    ASSERT_TRUE(G.outpostPlayed == testG.outpostPlayed); 
    ASSERT_TRUE(G.outpostTurn == testG.outpostTurn); 
    ASSERT_TRUE(G.whoseTurn == testG.whoseTurn); 
    ASSERT_TRUE(G.phase == testG.phase); 
    ASSERT_TRUE(G.numActions == testG.numActions); 
    ASSERT_TRUE(G.coins == testG.coins); 
    ASSERT_TRUE(G.numBuys == testG.numBuys); 

    // tests gamestate for other players
    for (j = 0; j < numPlayers; j++) {
      if (j != player) {
        ASSERT_TRUE(memcmp(G.hand[j], testG.hand[j], sizeof(int) * (G.handCount[j] - 1)) == 0);
        ASSERT_TRUE(memcmp(G.discard[j], testG.discard[j], sizeof(int) * (G.discardCount[j] - 1)) == 0);
        ASSERT_TRUE(memcmp(G.deck[j], testG.deck[j], sizeof(int) * (G.deckCount[j] - 1)) == 0);
      }
    }
    i++;
  }

  end = clock();
  printf("Minutes elapsed: %8.2lf\n", ((double) (end - start)) / CLOCKS_PER_SEC / 60);
  return 0;
}

