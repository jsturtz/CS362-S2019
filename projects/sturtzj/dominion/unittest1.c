#include "dominion.h"
#include "dominion_helpers.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "rngs.h"
#include <stdlib.h>

int main() {
  
  int newCards = 0;
  int discarded = 1;
  int xtraCoins = 0;
  int shuffledCards = 0;
  int i, j, m;
  int handpos = 0, choice1 = 0, choice2 = 0, choice3 = 0, bonus = 0;
  int remove1, remove2;
  int seed = 1000;
  int numPlayers = 4;
  int thisPlayer = 0;
  struct gameState G, testG;
  int k[10] = {adventurer, embargo, village, minion, mine, cutpurse, 
    sea_hag, tribute, smithy, council_room};

  // initialize a game state and player cards
  initializeGame(numPlayers, k, seed, &G);
  
  // copy state into testG
  memcpy(&testG, &G, sizeof(struct gameState));
  
  // call adventurerEffect
  adventurerEffect(&testG, thisPlayer); 

  // TEST 1 - HandCount increased by two
  int oldCount = G.handCount[thisPlayer];
  int newCount = testG.handCount[thisPlayer];
  assert(oldCount + 2 == newCount);
 
  // TEST 2 - The two new cards are treasures
  printf("TEST 2 - New cards are treasures");
  int card1 = testG.hand[thisPlayer][oldCount];
  int card2 = testG.hand[thisPlayer][oldCount+1];
  
  assert(card1 <= gold && card1 >= copper);
  assert(card2 <= gold && card2 >= copper);
   
  
  // TEST 3 - The original hand remains unchanged
  //
  // TEST 4 - Any revealed treasures were put automatically into hand
  printf("TEST 1 - HandCount increased by two");
  for (int i = 1; i < numPlayers; i++) {
  }
  
  //
  // TEST 6 - Deck is shuffled IFF drawn cards = remaining in deck

  
}

