#include "dominion.h"
#include "dominion_helpers.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "rngs.h"
#include <stdlib.h>

/*--------------------------------------------------------------------------------------------
INPUT PARTITION 
Input case 1: Deck has more than three cards

STANDARD TESTS (run same way for all inputs)
TEST 1: Player's hand should be increased by 3
TEST 2: Player's original hand has not been changed
TEST 3: Player's card count has not changed (i.e. deck + discard + hand stays the same)
TEST 4: Gamestate has not changed for any other player, or any non-player variable 

INPUT DEPENDENT TESTS
IF deckCount >= 3
     TEST 5: new cards in hand should be same as top three cards in deck
     TEST 6: deckCount should decrease by three
     TEST 7: discard pile should not change
-------------------------------------------------------------------------------------------*/

// sets hand to all coppers except last card, which will be smithy
int setHand(struct gameState* G, int player, int finalCard) {
  for (int i = 0; i < finalCard; i++) {
    G->hand[player][i] = copper;
  }
  G->hand[player][finalCard] = smithy;
  G->handCount[player] = finalCard + 1;
  return 0;
}

// sets deck for player such that deck only has up to size elements of coppers(-1 after)
int setDeck(struct gameState* G, int player, int size) {
  for (int i = 0; i < size; i++)
    G->deck[player][i] = copper;
  for (int i = size; i < MAX_DECK; i++)
    G->deck[player][i] = -1;
  return 0;
}

// runs test for whether hand increased by two (draws three, loses one because smithy is played)
int testHandIncrease(int player, struct gameState* G, struct gameState* testG) {

  int oldCount = G->handCount[player];
  int newCount = testG->handCount[player];
  if (oldCount + 2 == newCount) printf("\tPASSED: HandCount increased by 2\n"); 
  else printf("\t\u274CFAILED: New handCount == %d, but oldCount == %d\n", newCount, oldCount);
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
  if (pass) printf("\tPASSED: Original cards in hand unchanged except for smithy played\n");
  else printf("\t\u274CFAILED: Original cards in hand have been changed\n");
  return 0;
}

// runs test for whether total card count has changed (i.e. deck + discard + hand + played)
int testCardCount(int player, struct gameState* G, struct gameState* testG) {

  int gCount = G->deckCount[player] + G->discardCount[player] + G->handCount[player] + G->playedCardCount;
  int testGCount = testG->deckCount[player] + testG->discardCount[player] + testG->handCount[player] + testG->playedCardCount;
  
  if (gCount == testGCount) {
    printf("\tPASSED: Total card count is the same: deck + discard + hand + played\n");
  } else if (gCount < testGCount) {
    printf("\t\u274CFAILED: Total card count of deck (deck + discard + hand + played) less than after card effect\n"); 
  } else {
    printf("\t\u274CFAILED: Total card count of deck (deck + discard + hand + played) greater than after card effect\n");
  }
  return 0;
}

// will check that smithy was played, i.e. that it is appended to testG.playedCards
int testSmithyPlayed(int player, int handpos, struct gameState* G, struct gameState* testG) {
  if (testG->playedCardCount == G->playedCardCount + 1) {
    if (testG->playedCards[testG->playedCardCount - 1] == smithy)
      printf("\tPASSED: Smithy card was properly played, i.e. smithy added to playedCard array\n");
    else
      printf("\t\u274CFAILED: Smithy card was not properly played, i.e. added to playedCard array\n");
  } 
  else printf("\t\u274CFAILED: Smithy card was not properly played, i.e. added to playedCard array\n");
  return 0;
}

// runs tests checking all gamestate variables unrelated to players
int testGameState(int player, struct gameState* G, struct gameState* testG) {
  
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

  if (G->numBuys != testG->numBuys) {
    printf("\t\u274CFAILED: Gamestate numBuys has changed\n");
    total_pass = 0;
  }

  for (int i = 0; i < G->playedCardCount; i++) {
    if (G->playedCards[i] != testG->playedCards[i]) {
      printf("\t\u274CFAILED: Played cards has changed aside from smithy\n");
      total_pass = 0;
    }
  }
  
  if (total_pass) printf("\tPASSED: Non-player-related gamestate has not changed\n");
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

int runTests(int player, int handpos, struct gameState* G, struct gameState* testG) {
  testHandIncrease(player, G, testG);           // TEST 1 - HandCount increased by three
  testOriginalHand(player, handpos, G, testG);           // TEST 2 - The original hand remains unchanged
  testCardCount(player, G, testG);              // TEST 3 - Total card count remains the same
  testGameState(player, G, testG);              // TEST 4 - Tests equivalence of all gameState variables unrelated to player
  testOtherPlayers(player, G, testG);           // TEST 5 - Tests whether other player variables have been changed
  testSmithyPlayed(player, handpos, G, testG);  // TEST 6 - Tests whether smithy was played
  return 0;
}

int main() {
  
  int pass;                             // used as bool for whether test has passed
  int seed = 1000;                      // seed for initializeGame
  int numPlayers = 4;                   // numPlayers
  int player = 0;                       // all tests will be performed on first player
  struct gameState G, testG, blankG;    // blankG = at first initialization, G = before function, testG = after function
  int handpos = 4;                      // smithy will be placed at last position in fresh hand of five

  // initialize a game state and player cards
  int k[10] = {adventurer, embargo, village, minion, mine, cutpurse, sea_hag, tribute, smithy, council_room};
  initializeGame(numPlayers, k, seed, &G);
  memcpy(&blankG, &G, sizeof(struct gameState));
  
  printf("TESTING FUNCTION \"SmithyEffect\"\n");

  // Input case 1: Deck has more than three cards in it
  printf("Input Case 1: Deck has more than three cards in it\n");

  // add three copper to deck to ensure has enough
  for (int i = 0; i < 3; i++) {
    G.deck[player][G.deckCount[player]+i] = copper; 
  }
  G.deckCount[player] = G.deckCount[player] + 3;
  
  setHand(&G, player, handpos);                 // sets hand to ensure errors from initialization do not affect
  memcpy(&testG, &G, sizeof(struct gameState)); // copy into new test variable
  cardEffect(smithy, 0, 0, 0, &testG, 0, 0);

  // runs the same tests for all inputs because these tests do not depend on particular input 
  runTests(player, handpos, &G, &testG);
  
  // TEST: Cards added to hand should match top three of deck
  pass = 1;
  for (int i = 0; i < 3; i++) {
    int handCard = testG.hand[player][G.handCount[player] - 1 + i];
    int deckCard = G.deck[player][G.deckCount[player] - 1 - i];
    if (handCard != deckCard) {
      pass = 0;
    }
  }
  if (pass) printf("\tPASSED: Cards added to hand match top three cards of deck\n");
  else printf("\t\u274CFAILED: Cards added to hand do not match top three cards of deck\n");
  
  // TEST: Deckcount should go down by three
  if (G.deckCount[player] == testG.deckCount[player] + 3) 
    printf("\tPASSED: Deck has decreased by exactly three\n");
  else
    printf("\t\u274CFAILED: Deck has not decreased by exactly three\n");

  // TEST: Discard pile should not change
  if (G.discardCount[player] == testG.discardCount[player]) {
    pass = 1;
    for (int i = 0; i < G.discardCount[player]; i++) {
      if (G.discard[player][i] != testG.discard[player][i]) {
        pass = 0;
      }
    }
    if (pass) printf("\tPASSED: Discard pile has not changed\n");
    else printf("\t\u274CFAILED: Discard pile has changed\n");
  } 
  else
    printf("\t\u274CFAILED: Discard pile has changed\n");
  printf("\n");
}
