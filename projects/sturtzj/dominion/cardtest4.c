#include "dominion.h"
#include "dominion_helpers.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "rngs.h"
#include <stdlib.h>

/*--------------------------------------------------------------------------------------------
INPUT PARTITION 
Input case 1: Select treasure for trashing 
Input case 2: Select non-treasure for trashing (should return -1)
Input case 3: Select non-treasure for gaining after trashing treasure (should return -1)

IF treasure is selected for trashing,
    TEST 1: Player's handCount decrease by 1 (by playing Mine)
    TEST 2: Chosen card should be replaced with treasure no more than one tier higher than itself (i.e. copper can become silver but not gold)
    TEST 3: Player's total card count has not changed (i.e. deck + discard + hand + played stays the same)
    TEST 4: Supply pile from gained treasure should decrease by 1
    TEST 5: Gamestate variables unrelated to player should not change 
    TEST 6: Other player variables should not changed
    TEST 7: Mine card should be played, i.e. added to playedCards array and value in hand becomes -1
    TEST 8: discard should not change
IF non-treasure is selected for trashing or gaining, 
    TEST 10: function should return -1

-------------------------------------------------------------------------------------------*/

// sets hand to all coppers except last card, which will be Mine
int setHand(struct gameState* G, int player, int finalCard) {
  for (int i = 0; i < finalCard; i++) {
    G->hand[player][i] = copper;
  }
  G->hand[player][finalCard] = mine;
  G->handCount[player] = finalCard + 1;
  return 0;
}

// sets deck for player such that deck only has up to size elements of coppers(-1 after)
// adds ten cards to discard
int setDeck(struct gameState* G, int player, int size) {
  for (int i = 0; i < size; i++)
    G->deck[player][i] = copper;
  for (int i = size; i < MAX_DECK; i++)
    G->deck[player][i] = -1;
  for (int i = 0; i < 10; i++)
    G->discard[player][i] = copper;
  G->deckCount[player] = size;
  G->discardCount[player] = 10;
  return 0;
}

// runs test for whether hand increased by X
int testHandIncrease(int player, int X, struct gameState* G, struct gameState* testG) {

  int oldCount = G->handCount[player];
  int newCount = testG->handCount[player];
  if (oldCount + X == newCount) printf("\tPASSED: HandCount increased by %d\n", X); 
  else printf("\t\u274CFAILED: Expected handcount to increase by %d, but increased by %d\n", X, newCount - oldCount);
  return 0;
}

// runs test for whether total card count has changed (i.e. deck + discard + hand + played)
int testCardCount(int player, struct gameState* G, struct gameState* testG, int active) {
  
  int gCount = G->deckCount[player] + G->discardCount[player] + G->handCount[player];
   
  int testGCount = testG->deckCount[player] + testG->discardCount[player] + testG->handCount[player];
  
  // "active" is a boolean flag for whether the player is active, and therefore must consider the playedCardsCount
  if (active) {
    gCount += G->playedCardCount;
    testGCount += testG->playedCardCount;
  }
  
  if (gCount == testGCount) {
    printf("\tPASSED: Total card count is the same\n");
  } else if (testGCount < gCount) {
    printf("\t\u274CFAILED: Total card count of deck less than after card effect\n"); 
  } else {
    printf("\t\u274CFAILED: Total card count of deck greater than after card effect\n");
  }
  return 0;
}

// will check that mine was played, i.e. that it is appended to testG.playedCards
int testCardPlayed(int player, int handpos, struct gameState* G, struct gameState* testG) {
  if (testG->playedCardCount == G->playedCardCount + 1) {
    if (testG->playedCards[testG->playedCardCount - 1] == mine)
      printf("\tPASSED: Mine card was properly played, i.e. added to playedCard array\n");
    else
      printf("\t\u274CFAILED: Mine was not properly played, i.e. added to playedCard array\n");
  } 
  else printf("\t\u274CFAILED: Mine was not properly played, i.e. added to playedCard array\n");
  return 0;
}

// runs tests checking all gamestate variables unrelated to players. Ignores supply pile for silver.
int testGameState(struct gameState* G, struct gameState* testG) {
  
  int partial_pass;
  int total_pass = 1;
  if (G->numPlayers != testG->numPlayers) {
    printf("\t\u274CFAILED: Gamestate numPlayers has changed\n");
    total_pass = 0;
  }
  
  partial_pass = 1;
  for (int i = 0; i < treasure_map+1; i++) {
    if (i != silver) {
      if (G->supplyCount[i] != testG->supplyCount[i]) {
        partial_pass = 0;
        total_pass = 0;
      }
    }
  }
  if (!partial_pass) printf("\t\u274CFAILED: Gamestate supply piles that are not copper or silver have changed\n");

  partial_pass = 1;
  for (int i = 0; i < treasure_map+1; i++) {
    if (G->embargoTokens[i] != testG->embargoTokens[i]) {
      partial_pass = 0;
      total_pass = 0;
    }
  }
  if (!partial_pass) printf("\t\u274CFAILED: Gamestate embargo tokens on piles have changed\n");
  
  if (G->outpostPlayed != testG->outpostPlayed) {
    printf("\t\u274CFAILED: Gamestate outpostPlayed has changed\n");
    total_pass = 0;
  }

  if (G->outpostTurn != testG->outpostTurn) {
    printf("\t\u274CFAILED: Gamestate outpostTurn has changed\n");
    total_pass = 0;
  }

  if (G->whoseTurn != testG->whoseTurn) {
    printf("\t\u274CFAILED: Gamestate whoseTurn has changed\n");
    total_pass = 0;
  }

  if (G->phase != testG->phase) {
    printf("\t\u274CFAILED: Gamestate phase has changed\n");
    total_pass = 0;
  }

  if (G->numActions != testG->numActions) {
    printf("\t\u274CFAILED: Gamestate numActions has changed\n");
    total_pass = 0;
  }

  if (G->coins != testG->coins) {
    printf("\t\u274CFAILED: Gamestate coins has changed\n");
    total_pass = 0;
  }

  if (G->numBuys != testG->numBuys) {
    printf("\t\u274CFAILED: Gamestate numBuys has changed\n");
    total_pass = 0;
  }

  for (int i = 0; i < G->playedCardCount; i++) {
    if (G->playedCards[i] != testG->playedCards[i]) {
      printf("\t\u274CFAILED: Played cards has changed aside from mine\n");
      total_pass = 0;
    }
  }
  
  if (total_pass) printf("\tPASSED: Non-player-related gamestate variables have not changed \n");
  return 0;
}

/* struct gameState { */
/*   int numPlayers; //number of players */
/*   int supplyCount[treasure_map+1];  //this is the amount of a specific type of card given a specific number. */
/*   int embargoTokens[treasure_map+1]; */
/*   int outpostPlayed; */
/*   int outpostTurn; */
/*   int whoseTurn; */
/*   int phase; */
/*   int numActions; /1* Starts at 1 each turn *1/ */
/*   int coins; /1* Use as you see fit! *1/ */
/*   int numBuys; /1* Starts at 1 each turn *1/ */
/*   int hand[MAX_PLAYERS][MAX_HAND]; */
/*   int handCount[MAX_PLAYERS]; */
/*   int deck[MAX_PLAYERS][MAX_DECK]; */
/*   int deckCount[MAX_PLAYERS]; */
/*   int discard[MAX_PLAYERS][MAX_DECK]; */
/*   int discardCount[MAX_PLAYERS]; */
/*   int playedCards[MAX_DECK]; */
/*   int playedCardCount; */
/* }; */

// Checks that nothing has changed at all in both gameStates
int testTotalGameState(struct gameState* G, struct gameState* testG) {

  int partial_pass;
  int total_pass = 1;
  if (G->numPlayers != testG->numPlayers) {
    printf("\t\u274CFAILED: Gamestate altered (numPlayers)\n");
    total_pass = 0;
  }
  
  partial_pass = 1;
  for (int i = 0; i < treasure_map+1; i++) {
    if (G->supplyCount[i] != testG->supplyCount[i]) {
      partial_pass = 0;
      total_pass = 0;
    }
  }
  if (!partial_pass) printf("\t\u274CFAILED: Gamestate altered(supply piles)\n");

  partial_pass = 1;
  for (int i = 0; i < treasure_map+1; i++) {
    if (G->embargoTokens[i] != testG->embargoTokens[i]) {
      partial_pass = 0;
      total_pass = 0;
    }
  }
  if (!partial_pass) printf("\t\u274CFAILED: Gamestate altered (embargo tokens)\n");
  
  if (G->outpostPlayed != testG->outpostPlayed) {
    printf("\t\u274CFAILED: Gamestate altered(outpostPlayed)\n");
    total_pass = 0;
  }

  if (G->outpostTurn != testG->outpostTurn) {
    printf("\t\u274CFAILED: Gamestate altered(outpostTurn)\n");
    total_pass = 0;
  }

  if (G->whoseTurn != testG->whoseTurn) {
    printf("\t\u274CFAILED: Gamestate altered(whoseTurn)\n");
    total_pass = 0;
  }

  if (G->phase != testG->phase) {
    printf("\t\u274CFAILED: Gamestate altered(phase)\n");
    total_pass = 0;
  }

  if (G->numActions != testG->numActions) {
    printf("\t\u274CFAILED: Gamestate altered(numActions)\n");
    total_pass = 0;
  }

  if (G->coins != testG->coins) {
    printf("\t\u274CFAILED: Gamestate altered(coins)\n");
    total_pass = 0;
  }

  if (G->numBuys != testG->numBuys) {
    printf("\t\u274CFAILED: Gamestate altered(numBuys)\n");
    total_pass = 0;
  }

  // testing gamestate for players
  for (int p = 0; p < G->numPlayers; p++) {
    // Decks are identical
    if (G->deckCount[p] == testG->deckCount[p]) {
      partial_pass = 1;
      for (int i = 0; i < G->deckCount[p]; i++) {
        if (G->deck[p][i] != testG->deck[p][i]) {
          partial_pass = 0;
        }
      }
      if (!partial_pass) printf("\t\u274CFAILED: Player %d does not have identical deck after card effect\n", p);
    } 
    else printf("\t\u274CFAILED: Player %d does not have identical deck after card effect\n", p);

    // Hands are identical
    if (G->handCount[p] == testG->handCount[p]) {
      partial_pass = 1;

      for (int i = 0; i < G->handCount[p]; i++) {
        if (G->hand[p][i] != testG->hand[p][i]) {
          partial_pass = 0;
        }
      }
      if (!partial_pass) printf("\t\u274CFAILED: Player %d hand has changed after card effect\n", p);
    } 
    else printf("\t\u274CFAILED: Player %d hand has changed after card effect\n", p);

    // Discards are identical
    if (G->discardCount[p] == testG->discardCount[p]) {
      
      partial_pass = 1;
      for (int i = 0; i < G->discardCount[p]; i++) {
        if (G->discard[p][i] != testG->discard[p][i]) {
          partial_pass = 0;
        }
      }
      if (!partial_pass) printf("\t\u274CFAILED: Player %d discard has changed\n", p);
    }
    else printf("\t\u274CFAILED: Player %d discardCount has changed\n", p);
  }


  for (int i = 0; i < G->playedCardCount; i++) {
    if (G->playedCards[i] != testG->playedCards[i]) {
      printf("\t\u274CFAILED: Gamestate altered(played cards)\n");
      total_pass = 0;
    }
  }

  if (G->playedCardCount != testG->playedCardCount) {
    printf("\t\u274CFAILED: Gamestate altered(playedCardCount)\n");
    total_pass = 0;
  }
  
  if (total_pass) printf("\tPASSED: Gamestates are identical\n");
  return 0;
}

int testOtherPlayers(int player, struct gameState* G, struct gameState* testG) {
  
  int partial_pass;
  int total_pass = 1;
  
  // testing gamestate for other players except "player"
  for (int p = 0; p < G->numPlayers; p++) {
    if (p != player) {

      // Decks are identical
      if (G->deckCount[p] == testG->deckCount[p]) {
        partial_pass = 1;
        for (int i = 0; i < G->deckCount[p]; i++) {
          if (G->deck[p][i] != testG->deck[p][i]) {
            partial_pass = 0;
          }
        }
        if (!partial_pass) printf("\t\u274CFAILED: Player %d does not have identical deck after card effect\n", p);
      } 
      else printf("\t\u274CFAILED: Player %d does not have identical deck after card effect\n", p);

      // Hands are identical
      if (G->handCount[p] == testG->handCount[p]) {
        partial_pass = 1;

        for (int i = 0; i < G->handCount[p]; i++) {
          if (G->hand[p][i] != testG->hand[p][i]) {
            partial_pass = 0;
          }
        }
        if (!partial_pass) printf("\t\u274CFAILED: Player %d hand has changed after card effect\n", p);
      } 
      else printf("\u274CFAILED: Player %d hand has changed after card effect\n", p);

      // Discards are identical
      if (G->discardCount[p] == testG->discardCount[p]) {
        
        partial_pass = 1;
        for (int i = 0; i < G->discardCount[p]; i++) {
          if (G->discard[p][i] != testG->discard[p][i]) {
            partial_pass = 0;
          }
        }
        if (!partial_pass) printf("\t\u274CFAILED: Player %d discard has changed\n", p);
      }
      else printf("\t\u274CFAILED: Player %d discardCount has changed\n", p);
    }
  }
  if (total_pass) printf("\tPASSED: Other player variables have not been affected\n");
  return 0;
}

int testSameDiscard(int player, struct gameState* G, struct gameState* testG) {
  int pass;
  if (G->discardCount[player] == testG->discardCount[player]) {
    pass = 1;
    for (int i = 0; i < G->discardCount[player]; i++) {
      if (G->discard[player][i] != testG->discard[player][i]) {
        pass = 0;
      }
    }
    if (pass) printf("\tPASSED: Discard pile has not changed\n");
    else printf("\t\u274CFAILED: Discard pile has changed\n");
  } 
  else
    printf("\t\u274CFAILED: Discard pile has changed\n");
  return 0;
}

int main() {
  
  int seed = 1000;                      // seed for initializeGame
  int numPlayers = 2;                   // numPlayers
  struct gameState G, testG, blankG;    // blankG = at first initialization, G = before function, testG = after function
  int handpos = 4;                      // mine will be placed at last position in fresh hand of five
  int player = 0;                       // Player that uses card
  int coin_bonus = 0;                   // needed for cardEffect function
  int choice0, choice1;                 // choice0 = position of card to be trashed, choice1 = card to be gained
  int result;                           // used to hold return value of calling function 

  // initialize a game state and player cards
  int k[10] = {adventurer, embargo, village, minion, mine, cutpurse, sea_hag, tribute, smithy, council_room};
  initializeGame(numPlayers, k, seed, &G);
  memcpy(&blankG, &G, sizeof(struct gameState));
  
  printf("TESTING FUNCTION \"cardEffect\" with card Mine\n");

  // Input case 1: Treasure is selected for trashing
  printf("Input Case 1: Treasure is selected for trashing\n");
  
  setDeck(&G, player, 5);                       // set deck to have five coppers
  setHand(&G, player, handpos);                 // sets hand to have four coppers and Mine
  memcpy(&testG, &G, sizeof(struct gameState)); 
  choice0 = 0;                                  // will be trashing first card, which is a copper
  choice1 = silver;                             // will be gaining silver 
  cardEffect(mine, choice0, choice1, 0, &testG, handpos, &coin_bonus); // the 0s are booleans for choice 0, 1, & 2, not relevant for adventurer

  testHandIncrease(player, -1, &G, &testG);               // HandCount increased by -1
  testCardCount(player, &G, &testG, 1);                   // Total card count remains the same (+1 gain offsets -1 trash)
  testCardPlayed(player, handpos, &G, &testG);            // ensures mine is played, i.e. added to playedCard array
  testGameState(&G, &testG);                              // Tests equivalence of all non-player related gameState variables except supply pile for copper and silver
  testOtherPlayers(player, &G, &testG);                   // Tests equivalence of all player related gameState variables except current player

  // TEST: First copper is replaced with silver
  if (testG.hand[player][0] == silver)
    printf("\tPASSED: First copper was replaced with silver\n");
  else printf("\t\u274CFAILED: First copper was not replaced with silver\n");

  // TEST: Supply pile of silver decreased by 1
  if (G.supplyCount[silver] == testG.supplyCount[silver] + 1) 
    printf("\tPASSED: Silver supply pile decreased by 1\n");
  else printf("\t\u274CFAILED: Silver supply pile did not decrease by 1, instead decreased by %d\n", G.supplyCount[silver] - testG.supplyCount[silver]);
  
  // TEST: Discard count should not have changed
  if (G.discardCount[player] == testG.discardCount[player])
    printf("\tPASSED: Discard pile remained the same\n");
  else printf("\t\u274CFAILED: Discard count should not have changed, instead increased by %d\n", testG.supplyCount[silver] - G.supplyCount[silver]);

  // Input case 2: A non-treasure is selected for trashing
  printf("Input Case 2: A Non treasure is selected for trashing\n");
  
  memcpy(&G, &blankG, sizeof(struct gameState));        
  setDeck(&G, player, 5);                       // set deck to have five coppers
  setHand(&G, player, handpos);                 // sets hand to have four coppers and Mine
  G.hand[player][0] = village;                  // replace first copper in hand with village for test

  memcpy(&testG, &G, sizeof(struct gameState)); 
  choice0 = 0;                                  // will be trashing first card, which is a village
  choice1 = silver;                             // will be gaining silver 

  result = cardEffect(mine, choice0, choice1, 0, &testG, handpos, &coin_bonus);

  // TEST: Result should be -1
  if (result == -1)
    printf("\tPASSED: Function returned -1 (not allowed to trash non-treasure)\n");
  else printf("\t\u274CFAILED: Function did not return -1, so it continued unexpectedly\n");

  // TEST: Gamestate should not have changed
  testTotalGameState(&G, &testG);

  // Input case 3: A non-treasure is selected for gaining
  printf("Input Case 3: A Non treasure is selected for gaining\n");
  
  memcpy(&G, &blankG, sizeof(struct gameState));        
  setDeck(&G, player, 5);                       // set deck to have five coppers
  setHand(&G, player, handpos);                 // sets hand to have four coppers and Mine

  memcpy(&testG, &G, sizeof(struct gameState)); 
  choice0 = 0;                                  // will be trashing first card, which is a copper
  choice1 = village;                            // will be attempting to gain village

  result = cardEffect(mine, choice0, choice1, 0, &testG, handpos, &coin_bonus); 

  // TEST: Result should be -1
  if (result == -1)
    printf("\tPASSED: Function returned -1 (not allowed to gain non-treasure)\n");
  else printf("\t\u274CFAILED: Function did not return -1, so it continued unexpectedly\n");

  // TEST: Gamestate should not have changed
  testTotalGameState(&G, &testG);

  printf("\n");
}
