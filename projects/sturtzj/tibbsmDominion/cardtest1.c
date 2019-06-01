#include "dominion.h"
#include "dominion_helpers.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "rngs.h"
#include <stdlib.h>

/*-------------------------------------------------------------------------------------------------
INPUT PARTITION 
Input case 3: There is only one treasure in the deck, the second is found after shuffling
Input case 4: There are no treasures in deck, both are found after shuffling

STANDARD TESTS (run same way for all inputs)
TEST 1: Adventurer card should be "played" i.e. added to playedCard variable
TEST 2: Player's hand should be increased by 2
TEST 3: The two new cards should be treasures
TEST 4: Player's original hand has not been changed (aside from two new treasures)
TEST 5: Player's card count has not changed (i.e. deck + discard + hand stays the same)
TEST 6: Gamestate has not changed for any non-player related variables
TEST 7: Gamestate has not changed for any other player

INPUT DEPENDENT TESTS
TEST 8: If deck must be shuffled to access 2nd treasure, discard pile should not have any treasures
-------------------------------------------------------------------------------------------------*/

// will check that adventurer was played, i.e. that it is appended to testG.playedCards
int testAdventurerPlayed(int player, int handpos, struct gameState* G, struct gameState* testG) {
  if (testG->playedCardCount == G->playedCardCount + 1) {
    if (testG->playedCards[testG->playedCardCount - 1] == adventurer)
      printf("\tPASSED: Adventurer card was properly played, i.e. smithy added to playedCard array\n");
    else
      printf("\t\u274CFAILED: Adventurer card was not properly played, i.e. added to playedCard array\n");
  } 
  else printf("\t\u274CFAILED: Adventurer card was not properly played, i.e. added to playedCard array\n");
  return 0;
}

// tests that handCount has increased by 1 (adds two treasure, but should subtract itself because card is played)
int testHandCount(int player, struct gameState* G, struct gameState* testG) {
  int oldCount = G->handCount[player];
  int newCount = testG->handCount[player];
  if (oldCount + 1 == newCount) printf("\tPASSED: HandCount increased by 1 (2 treasures - 1 adventurer)\n"); 
  else printf("\t\u274CFAILED: Handcount should have increased by 1, instead increased by %d\n", newCount - oldCount);
  return 0;
}

// tests that last two cards in hand are treasures (only works if adventurer is last card in hand)
int testBothTreasures(int player, struct gameState* G, struct gameState* testG) {

  int card1 = testG->hand[player][testG->handCount[player]-1];
  int card2 = testG->hand[player][testG->handCount[player]-2];
  if ((card1 <= gold && card1 >= copper) && (card2 <= gold && card2 >= copper)) printf("\tPASSED: Newly added cards are treasures\n");
  else printf("\t\u274CFAILED: At least one newly added cards is not a treasure\n");
  return 0;
}

// tests that original hand is unchanged
int testOriginalHand(int player, struct gameState* G, struct gameState* testG) {
  int pass = 1;
  for (int i = 0; i < G->handCount[player]; i++) {
    if (G->hand[player][i] != testG->hand[player][i]) {
      pass = 0;
    }
  }
  if (pass) printf("\tPASSED: Original cards in hand unchanged\n");
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

// will run the standard tests that should be run on all four input types
int runTests(int player, int handpos, struct gameState* G, struct gameState* testG) {
  
  testAdventurerPlayed(player, handpos, G, testG);
  testHandCount(player, G, testG);                      // TEST 1 - HandCount increased by two
  testBothTreasures(player, G, testG);                  // TEST 2 - The two new cards are treasures
  testOriginalHand(player, G, testG);                   // TEST 3 - The original hand remains unchanged
  testCardCount(player, G, testG);                      // TEST 4 - Total card count remains the same
  testGameState(player, G, testG);                      // TEST 5 - Tests equivalence of all gameState variables unrelated to players
  testOtherPlayers(player, G, testG);                   // TEST 5 - Tests equivalence of all player variables aside from active player
  return 0;
}

int main() {
  
  int nextCard;                         // used for tracking cards through loops
  int pass;                             // used as bool for whether test has passed
  int seed = 1000;                      // seed for initializeGame
  int numPlayers = 4;                   // numPlayers
  int player;                           // the player who will use the card
  int handpos;                          // used to indicate handposition of adventurer
  int coin_bonus = 0;
  struct gameState G, testG, blankG;    // blankG = at first initialization, G = before function, testG = after function


  // initialize a game state and player cards
  int k[10] = {adventurer, embargo, village, minion, mine, cutpurse, sea_hag, tribute, smithy, council_room};
  initializeGame(numPlayers, k, seed, &G);
  player = whoseTurn(&G);
  memcpy(&blankG, &G, sizeof(struct gameState));
  
  printf("TESTING FUNCTION \"cardEffect\" for Adventurer card\n");

  // Input case 3: There is only one treasure in deck, so second treasure found after shuffling
  printf("Input Case 3: There is only one treasure in deck, so second treasure must be found after shuffling discard\n");
  
  // manually set deck so only one treasure
  G.deckCount[player] = 4;                                              
  G.deck[player][0] = G.deck[player][1] = G.deck[player][2] = estate;
  G.deck[player][3] = copper;

  // add at least one copper to discard so guaranteed to pull second treasure
  G.discard[player][G.discardCount[player]] = copper;
  G.discardCount[player]++;

  // add adventurer to hand 
  handpos = G.handCount[player];                                        
  G.hand[player][handpos] = adventurer;     
  G.handCount[player]++;                                
  
  // call function to compare results
  memcpy(&testG, &G, sizeof(struct gameState    ));
  cardEffect(adventurer, 0, 0, 0, &testG, handpos, &coin_bonus); // the 0s are booleans for choice 0, 1, & 2, not relevant for adventurer

  // runs same tests for all inputs
  runTests(player, handpos, &G, &testG);                         

  // TEST: Player discard pile has no treasures
  pass = 1;
  for (int i = 0; i < testG.discardCount[player]; i++) {
    nextCard = testG.discard[player][i];
    if ((nextCard >= copper && nextCard <= gold)) {
      pass = 0; 
      break;
    }
  }
  if (pass) printf("\tPASSED: Player discard pile has no treasures\n");
  else printf("\t\u274CFAILED: Player discard pile has treasures\n");

  // Input case 4: There are no treasures in deck, so second treasure found after shuffling
  printf("Input Case 4: There are no treasures in deck, so second treasure must be found after shuffling discard\n");

  // reset gamestates
  memcpy(&G, &blankG, sizeof(struct gameState));
  
  // put no treasures in deck
  G.deckCount[player] = 3;
  G.deck[player][0] = G.deck[player][1] = G.deck[player][2] = estate;

  // add treasures to discard to ensure enough treasures in discard pile
  G.discard[player][G.discardCount[player]] = copper;
  G.discard[player][G.discardCount[player]+1] = copper;
  G.discardCount[player] = G.discardCount[player] + 2;

  // add adventurer to hand 
  handpos = G.handCount[player];                                        
  G.hand[player][handpos] = adventurer;     
  G.handCount[player]++;                                
  
  memcpy(&testG, &G, sizeof(struct gameState));
  cardEffect(adventurer, 0, 0, 0, &testG, handpos, &coin_bonus); // the 0s are booleans for choice 0, 1, & 2, not relevant for adventurer

  // runs same tests for all inputs
  runTests(player, handpos, &G, &testG);                         

  // TEST: Player discard pile has no treasures
  pass = 1;
  for (int i = 0; i < testG.discardCount[player]; i++) {
    nextCard = testG.discard[player][i];
    if ((nextCard >= copper && nextCard <= gold)) {
      pass = 0; 
      break;
    }
  }
  if (pass) printf("\tPASSED: Player discard pile has no treasures\n");
  else printf("\t\u274CFAILED: Player discard pile has treasures\n");
  printf("\n");
}