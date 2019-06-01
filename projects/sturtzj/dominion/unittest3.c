#include "dominion.h"
#include "dominion_helpers.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "rngs.h"
#include <stdlib.h>

/*--------------------------------------------------------------------------------------------
INPUT PARTITION 
Input case 1: Deck has more than four cards (for active player) and Deck has more than one card (for non-active player)
Input case 2: Deck has less than four cards (for active player) and Deck has exactly one card (for non-active player)

STANDARD TESTS (run same way for all inputs)
TEST 1: Player's hand should be increased by 3 (draw four, play council room)
TEST 2: Player's original hand has not been changed 
TEST 3: Player's card count has not changed (i.e. deck + discard + hand stays the same)
TEST 4: Gamestate has not changed except +1 Buy
TEST 5: Other players' hand increased by 1
TEST 6: New card in each non-active player's hand should come from top of deck

INPUT DEPENDENT TESTS

For active player: 
IF deckCount < 4, 
     TEST 5: hand should have all cards currently in deck
     TEST 6: discard pile should be empty
IF deckCount >= 4
     TEST 7: new cards in hand should be same as top four cards in deck
     TEST 8: deckCount should decrease by four
     TEST 9: discard pile should not change

For non-active player:
IF deckCount = 0, 
    TEST 10: discard pile should be empty
IF deckCount > 0, 
    TEST 11: deckCount should decrease by 1
    TEST 12: discard pile should not change
-------------------------------------------------------------------------------------------*/

// sets hand to all coppers except last card, which will be council room
int setHand(struct gameState* G, int player, int finalCard) {
  for (int i = 0; i < finalCard; i++) {
    G->hand[player][i] = copper;
  }
  G->hand[player][finalCard] = council_room;
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

// runs test for whether hand increased by X. Used for both active and non-active player
int testHandIncrease(int player, int X, struct gameState* G, struct gameState* testG) {

  int oldCount = G->handCount[player];
  int newCount = testG->handCount[player];
  if (oldCount + X == newCount) printf("\tPASSED: For Player %d, HandCount increased by %d\n", player, X); 
  else printf("\t\u274CFAILED: For Player %d, expected handcount to increase by %d, but increased by %d\n", player, X, newCount - oldCount);
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
  if (pass) printf("\tPASSED: For player %d, original cards in hand unchanged except for council room played\n", player);
  else printf("\t\u274CFAILED: For player %d, original cards in hand have been changed\n", player);
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
    printf("\tPASSED: For player %d, total card count is the same\n", player);
  } else if (gCount < testGCount) {
    printf("\t\u274CFAILED: For player %d, total card count of deck less than after card effect\n", player); 
  } else {
    printf("\t\u274CFAILED: For player %d, total card count of deck greater than after card effect\n", player);
  }
  return 0;
}

// will check that council room was played, i.e. that it is appended to testG.playedCards
int testCardPlayed(int player, int handpos, struct gameState* G, struct gameState* testG) {
  if (testG->playedCardCount == G->playedCardCount + 1) {
    if (testG->playedCards[testG->playedCardCount - 1] == council_room)
      printf("\tPASSED: Council Room card was properly played, i.e. added to playedCard array\n");
    else
      printf("\t\u274CFAILED: Council Room card was not properly played, i.e. added to playedCard array\n");
  } 
  else printf("\t\u274CFAILED: Council Room card was not properly played, i.e. added to playedCard array\n");
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

  if (G->numActions != testG->numActions) {
    printf("\t\u274CFAILED: Gamestate numActions has changed\n");
    total_pass = 0;
  }

  if (G->coins != testG->coins) {
    printf("\t\u274CFAILED: Gamestate coins has changed\n");
    total_pass = 0;
  }

  if (G->numBuys != testG->numBuys - 1) {
    printf("\t\u274CFAILED: Gamestate numBuys did not increase by 1\n");
    total_pass = 0;
  }

  for (int i = 0; i < G->playedCardCount; i++) {
    if (G->playedCards[i] != testG->playedCards[i]) {
      printf("\t\u274CFAILED: Played cards has changed aside from council room\n");
      total_pass = 0;
    }
  }
  
  if (total_pass) printf("\tPASSED: Non-player-related gamestate has not changed except for numBuys\n");
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
    if (pass) printf("\tPASSED: Discard pile has not changed for player %d\n", player);
    else printf("\t\u274CFAILED: Discard pile has changed for player %d\n", player);
  } 
  else
    printf("\t\u274CFAILED: Discard pile has changed for player %d\n", player);
  return 0;
}

int runTests(int active, int nonActive, int handpos, struct gameState* G, struct gameState* testG) {
  testHandIncrease(active, 3, G, testG);                // HandCount increased by three for active player
  testHandIncrease(nonActive, 1, G, testG);             // HandCount increased by one for non active player
  testOriginalHand(active, handpos, G, testG);          // The original hand remains unchanged for both players
  testOriginalHand(nonActive, handpos, G, testG);       
  testCardCount(active, G, testG, 1);                   // Total card count remains the same for both players
  testCardCount(nonActive, G, testG, 0);                // Final argument a boolean for whether player is active              
  testCardPlayed(active, handpos, G, testG);            // ensures council room card is played, i.e. added to playedCard array
  testGameState(G, testG);                              // Tests equivalence of all non-player related gameState variables except numBuys which is +1
  return 0;
}

int main() {
  
  int nextCard;                         // used for tracking cards through loops
  int pass;                             // used as bool for whether test has passed
  int seed = 1000;                      // seed for initializeGame
  int numPlayers = 2;                   // numPlayers
  int active = 0;                       // active player will use council room
  int nonActive = 1;                    // nonactive will get benefit of +1 Card
  struct gameState G, testG, blankG;    // blankG = at first initialization, G = before function, testG = after function
  int handpos = 4;                      // council room will be placed at last position in fresh hand of five

  // initialize a game state and player cards
  int k[10] = {adventurer, embargo, village, minion, mine, cutpurse, sea_hag, tribute, smithy, council_room};
  initializeGame(numPlayers, k, seed, &G);
  memcpy(&blankG, &G, sizeof(struct gameState));
  
  printf("TESTING FUNCTION \"CouncilRoomEffect\"\n");
  printf("Player 0 = active player that plays council room\n");
  printf("Player 1 = non-active player\n");

  // Input case 1: Deck has more than four cards in it for active player, more than one card for non-active
  printf("Input Case 1: Deck has more than four cards in it for active, more than one for non-active\n");

  
  setDeck(&G, active, 5);                       // set both decks to have five copper
  setDeck(&G, nonActive, 5);
  setHand(&G, active, handpos);                 // sets hand to ensure errors from initialization do not affect
  memcpy(&testG, &G, sizeof(struct gameState)); 
  cardEffect(council_room, 0, 0, 0, &testG, 0, 0);

  // runs the same tests for all inputs because these tests do not depend on particular input 
  runTests(active, nonActive, handpos, &G, &testG);

  // TEST: Cards added to hand should match top four of deck for active
  pass = 1;
  for (int i = 0; i < 4; i++) {
    int handCard = testG.hand[active][G.handCount[active] - 1 + i];
    int deckCard = G.deck[active][G.deckCount[active] - 1 - i];
    if (handCard != deckCard) {
      pass = 0;
    }
  }
  if (pass) printf("\tPASSED: Cards added to hand match top four cards of deck\n");
  else printf("\t\u274CFAILED: Cards added to hand do not match top four cards of deck\n");
  
  // TEST: Card added to hand should match top card of deck for nonActive
  if (G.deck[nonActive][G.deckCount[nonActive]-1] == testG.hand[nonActive][testG.handCount[nonActive]-1])
    printf("\tPASSED: For player 1, new card matches top of deck\n");
  else
    printf("\t\u274CFAILED: For player 1, new card does not match top of deck\n");
  
  // TEST: Deckcount should go down by four for active
  if (G.deckCount[active] == testG.deckCount[active] + 4) 
    printf("\tPASSED: For player 0, deck has decreased by exactly four\n");
  else
    printf("\t\u274CFAILED: For player 0, deck has not decreased by exactly four\n");

  // TEST: Deckcount should go down by one for nonActive
  if (G.deckCount[nonActive] == testG.deckCount[nonActive] + 1) 
    printf("\tPASSED: For player 1, deck has decreased by exactly one\n");
  else
    printf("\t\u274CFAILED: For player 1, deck has not decreased by exactly one\n");

  // TEST: Discard pile should not change for either player
  testSameDiscard(active, &G, &testG);
  testSameDiscard(nonActive, &G, &testG);

  // Input case 2: active player has three cards in deck and nonActive has zero
  printf("Input Case 2: Active player has three cards in deck and non active has zero\n");

  memcpy(&G, &blankG, sizeof(struct gameState));        // reset gamestates

  setDeck(&G, active, 3);                               // will set deck to value and also add ten coppers to discard
  setDeck(&G, nonActive, 0);
  setHand(&G, active, handpos);                         // sets hand to ensure errors from initialization do not affect
  memcpy(&testG, &G, sizeof(struct gameState));         
  cardEffect(council_room, 0, 0, 0, &testG, 0, 0);

  // runs the same tests for all inputs because these tests do not depend on particular input 
  runTests(active, nonActive, handpos, &G, &testG);

  // TEST: Hand should have all three cards from deck + 1 unknown from shuffled for active
  pass = 1;
  for (int i = 0; i < 3; i++) {
    nextCard = G.deck[active][G.deckCount[active] - 1 - i];
    if (nextCard != testG.hand[active][G.handCount[active]+i]) {
      pass = 0; 
    }
  }

  if (pass) printf("\tPASSED: For player 0, the three remaining cards on deck were added to hand\n");
  else printf("\t\u274CFAILED: For player 0, expected three cards from hand to match remaining three cards in deck\n");

  // TEST: Discard should be empty for both players
  if (testG.discardCount[active] == 0)
    printf("\tPASSED: For player 0, discard count is 0\n");
  else
    printf("\t\u274CFAILED: For player 0, discard count is not 0\n");

  if (testG.discardCount[nonActive] == 0)
    printf("\tPASSED: For player 1, discard count is 0\n");
  else
    printf("\t\u274CFAILED: For player 1, discard count is not 0\n");

}
