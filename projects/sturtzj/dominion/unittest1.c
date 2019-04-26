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
  if (oldCount + 2 == newCount) printf("PASSED: HandCount increased by 2\n"); 
  else printf("FAILED: Expected HandCount == %d, but HandCount == %d\n", newCount, oldCount);
 
  // TEST 2 - The two new cards are treasures
  int card1 = testG.hand[thisPlayer][oldCount];
  int card2 = testG.hand[thisPlayer][oldCount+1];
  if ((card1 <= gold && card1 >= copper) || (card2 <= gold && card2 >= copper)) printf("PASSED: Newly added cards are treasures\n");
  else printf("FAILED: At least one newly added cards is not a treasure\n");
  
  // TEST 3 - The original hand remains unchanged
  int pass = 1;
  for (int i = 0; i < G.handCount[thisPlayer]; i++) {
    if (G.hand[thisPlayer][i] != testG.hand[thisPlayer][i]) {
      pass = 0;
    }
  }
  if (pass) printf("PASSED: Original cards in hand unchanged");
  else printf("FAILED: Original cards in hand have been changed");

  // TEST 4 - All revealed treasures were put automatically into hand (no treasures were discarded)
  
  // get all cards that were drawn

  for (int i = 1; i < numPlayers; i++) {

  }
  
  //
  // TEST 6 - Deck is shuffled IFF drawn cards = remaining in deck

  
}

