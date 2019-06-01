#include "dominion.h"
#include "dominion_helpers.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include "rngs.h"
#include <stdlib.h>

#ifndef NUMTRIES
#define NUMTRIES 100000
#endif

#define STR(x) #x
#ifndef ASSERT_TRUE
#define ASSERT_TRUE(x) if (!(x)) { printf("Assert failed: (%s), line %d.\n", STR(x), __LINE__);}
#endif 

/* struct cards { */
/*   int copper;           // 60 max */
/*   int silver;           // 40 max */
/*   int gold;             // 30 max */

/*   int estate;           // 24 max */
/*   int duchy;            // 12 max */
/*   int province;         // 12 max */
/*   int curse;            // 30 max */

/*   int kingdom[10]; */      
/*   int *kIDs; */        
/* }; */

/* struct cards* newCards(int *kIDs, int numPlayers, int hasCurse) */ 
/* { */

/*   struct cards* newGame = malloc(sizeof(struct cards)); */
/*   newGame->copper = 60; */
/*   newGame->silver = 40; */
/*   newGame->gold = 30; */
  
/*   switch (numPlayers) { */
/*     case 2: */ 
/*       newGame->estate   = 8  + (2 * 3); */
/*       newGame->duchy    = 8; */
/*       newGame->province = 8; */
/*       if (hasCurse) newGame->curse = 20; */
/*       break; */
/*     case 3: */ 
/*       newGame->estate   = 12  + (3 * 3); */
/*       newGame->duchy    = 12; */
/*       newGame->province = 12; */
/*       if (hasCurse) newGame->curse = 30; */
/*       break; */
/*     case 4: */ 
/*       newGame->estate   = 12  + (4 * 3); */
/*       newGame->duchy    = 12; */
/*       newGame->province = 12; */
/*       if (hasCurse) newGame->curse = 40; */
/*       break; */
/*     default: */ 
/*       return NULL; */
/*   } */

/*   if (!hasCurse) newGame->curse = 0; */

/*   // give each kingdom card ten */
/*   for (int k = 0; k < 10; k++) */ 
/*   { */
/*     newGame->kingdom[k] = 10; */
/*   } */
/*   // set pointer to kIDs */
/*   newGame->kIDs = kIDs; */
/*   return newGame; */
/* } */

/* void deleteCards(struct cards* cards) { */
/*   free(cards); */
/*   cards = NULL; */
/* } */

// Will produce a random kingdom except first card will be includeThisCard
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
  if (maxSize > 1) 
  {
    int discardSize = rand() % (maxSize - 1 );
    state->discardCount[player] = discardSize;

    for (int i = 0; i < discardSize; i++) {
      state->discard[player][i] = rand() % (treasure_map+1);
    }
  }
  else 
  {
    state->discardCount[player] = 0;
  }
}

int main() {

  clock_t start, end;
  start = clock();  // ensure time is under 5 minutes

  int i, j;
  int numPlayers, player;  
  int handpos;
  int coinBonus = 0;

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
     
      // guarantee two spots for copper in deck
      randomDeck(&G, j, MAX_DECK);
      if (G.deckCount[j] >= 2) 
      {
        G.deck[j][0] = copper;
        G.deck[j][1] = copper;
      }
      else 
      {
        G.discard[j][0] = copper;
        G.discard[j][1] = copper;
      }

      randomHand(&G, j, MAX_HAND - 1);
      G.hand[j][G.handCount[j]] = adventurer;
      G.handCount[j]++;

      // discard + deck shouldn't exceed MAX_DECK
      /* printf("MAX_DECK - G.deckCount[j]: %d\n", MAX_DECK - G.deckCount[j]); */
      randomDiscard(&G, j, MAX_DECK - G.deckCount[j]);
    }

    // call adventurerCard
    player = rand() % numPlayers;
    G.whoseTurn = player;
    memcpy(&testG, &G, sizeof(struct gameState));
    handpos = G.handCount[player] - 1;
    cardEffect(adventurer, 0, 0, 0, &testG, handpos, &coinBonus);

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

