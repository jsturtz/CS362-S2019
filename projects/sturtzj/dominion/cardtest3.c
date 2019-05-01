#include "dominion.h"
#include "dominion_helpers.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "rngs.h"
#include <stdlib.h>

/*--------------------------------------------------------------------------------------------
INPUT PARTITION 
Input case 1: Deck has more than 0 cards 
Input case 2: Deck has 0 cards left 

STANDARD TESTS (run same way for all inputs)
TEST 1: Player's handCount should not change (+1 Card negated by playing Great Hall)
TEST 2: Player's original hand has not been changed (except for playing Great Hall)
TEST 3: Player's total card count has not changed (i.e. deck + discard + hand + played stays the same)
TEST 4: Gamestate has not changed except +1 Actions
TEST 5: Other player variables have not changed
TEST 6: Great Hall card should be played, i.e. added to playedCards array

INPUT DEPENDENT TESTS

IF deckCount > 0, 
     TEST 7: hand should have same card as top of deck
     TEST 8: deckCount should decrease by 1
     TEST 9: discard should not change
IF deckCount == 0
     TEST 10: discard pile should be empty

-------------------------------------------------------------------------------------------*/

// sets hand to all coppers except last card, which will be great hall
int setHand(struct gameState* G, int player, int finalCard) {
  for (int i = 0; i < finalCard; i++) {
    G->hand[player][i] = copper;
  }
  G->hand[player][finalCard] = great_hall;
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

// runs test for whether original hand has changed
int testOriginalHand(int player, int handpos, struct gameState* G, struct gameState* testG) {
  int pass = 1;
  for (int i = 0; i < G->handCount[player]; i++) {
    if (i != handpos) {
      if (G->hand[player][i] != testG->hand[player][i]) {
        pass = 0;
      }
    }
  }
  if (pass) printf("\tPASSED: Original cards in hand unchanged except for great hall played\n");
  else printf("\t\u274CFAILED: Original cards in hand have been changed\n");
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
  } else if (gCount < testGCount) {
    printf("\t\u274CFAILED: Total card count of deck less than after card effect\n"); 
  } else {
    printf("\t\u274CFAILED: Total card count of deck greater than after card effect\n");
  }
  return 0;
}

// will check that great hall was played, i.e. that it is appended to testG.playedCards
int testCardPlayed(int player, int handpos, struct gameState* G, struct gameState* testG) {
  if (testG->playedCardCount == G->playedCardCount + 1) {
    if (testG->playedCards[testG->playedCardCount - 1] == great_hall)
      printf("\tPASSED: Great Hall card was properly played, i.e. added to playedCard array\n");
    else
      printf("\t\u274CFAILED: Great Hall card was not properly played, i.e. added to playedCard array\n");
  } 
  else printf("\t\u274CFAILED: Great Hall card was not properly played, i.e. added to playedCard array\n");
  return 0;
}

// runs tests checking all gamestate variables unrelated to players (except for numBuys +1)
int testGameState(struct gameState* G, struct gameState* testG) {
  
  int partial_pass;
  int total_pass = 1;
  if (G->numPlayers != testG->numPlayers) {
    printf("\t\u274CFAILED: Gamestate numPlayers has changed\n");
    total_pass = 0;
  }
  
  partial_pass = 1;
  for (int i = 0; i < treasure_map+1; i++) {
    if (G->supplyCount[i] != testG->supplyCount[i]) {
      partial_pass = 0;
      total_pass = 0;
    }
  }
  if (!partial_pass) printf("\t\u274CFAILED: Gamestate supply piles have changed\n");

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

  if (G->numActions != testG->numActions - 1) {
    printf("\t\u274CFAILED: Gamestate numActions has not increased by 1\n");
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
      printf("\t\u274CFAILED: Played cards has changed aside from great hall\n");
      total_pass = 0;
    }
  }
  
  if (total_pass) printf("\tPASSED: Non-player-related gamestate has not changed except for +2 Actions\n");
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


int runTests(int player, int handpos, struct gameState* G, struct gameState* testG) {
  testHandIncrease(player, 0, G, testG);                // HandCount increased by 
  testOriginalHand(player, handpos, G, testG);          // The original hand remains unchanged for both players
  testCardCount(player, G, testG, 1);                   // Total card count remains the same for both players
  testCardPlayed(player, handpos, G, testG);            // ensures great hall card is played, i.e. added to playedCard array
  testGameState(G, testG);                              // Tests equivalence of all non-player related gameState variables except numBuys which is +1
  testOtherPlayers(player, G, testG);                   // Tests equivalence of all non-player related gameState variables except numBuys which is +1
  return 0;
}

int main() {
  
  int seed = 1000;                      // seed for initializeGame
  int numPlayers = 2;                   // numPlayers
  struct gameState G, testG, blankG;    // blankG = at first initialization, G = before function, testG = after function
  int handpos = 4;                      // great hall will be placed at last position in fresh hand of five
  int player = 0;                       // Player that uses card
  int coin_bonus = 0;

  // initialize a game state and player cards
  int k[10] = {adventurer, embargo, village, minion, mine, cutpurse, sea_hag, tribute, smithy, council_room};
  initializeGame(numPlayers, k, seed, &G);
  memcpy(&blankG, &G, sizeof(struct gameState));
  
  printf("TESTING FUNCTION \"cardEffect\" with card Great Hall\n");

  // Input case 1: Deck has at least one card
  printf("Input Case 1: Deck has at least one card\n");
  
  setDeck(&G, player, 5);                       // set deck to have five coppers
  setHand(&G, player, handpos);                 // sets hand to ensure errors from initialization do not affect
  memcpy(&testG, &G, sizeof(struct gameState)); 
  cardEffect(great_hall, 0, 0, 0, &testG, handpos, &coin_bonus); // the 0s are booleans for choice 0, 1, & 2, not relevant for adventurer

  // runs the same tests for all inputs because these tests do not depend on particular input 
  runTests(player, handpos, &G, &testG);

  // TEST: Hand should have same card as top of deck
  if (G.deck[player][G.deckCount[player]-1] == testG.hand[player][testG.handCount[player]-1]) 
    printf("\tPASSED: Top card in hand equals top of deck\n");
  else printf("\t\u274CFAILED: Card added to hand does not match top of deck\n");
  
  // TEST: deckCount should decrease by 1
  if (G.deckCount[player] == testG.deckCount[player] + 1)
    printf("\tPASSED: Deckcount decreased by 1\n");
  else printf("\t\u274CFAILED: Expected deckcount to have decreased by 1, actually decreased by %d\n", G.deckCount[player] - testG.deckCount[player]);

  // TEST: discard should not change
  testSameDiscard(player, &G, &testG);

  // Input case 1: Deck has at least one card
  printf("Input Case 2: Deck has no cards\n");
  
  setDeck(&G, player, 0);                       // set deck to be empty, but discard will have ten coppers
  setHand(&G, player, handpos);                 // sets hand to ensure errors from initialization do not affect
  memcpy(&testG, &G, sizeof(struct gameState)); 
  cardEffect(great_hall, 0, 0, 0, &testG, handpos, &coin_bonus); // the 0s are booleans for choice 0, 1, & 2, not relevant for adventurer

  // runs the same tests for all inputs because these tests do not depend on particular input 
  runTests(player, handpos, &G, &testG);

  // TEST: discard should be empty
  if (testG.discardCount[player] == 0)
    printf("\tPASSED: Discard count is 0\n");
  else
    printf("\t\u274CFAILED: Discard count is not 0\n");
}
