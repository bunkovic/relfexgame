#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <ea_init.h>
#include <stdlib.h>

#include "configAppl.h"

#include "lcd.h"
#include "key.h"
#include "react.h"
#include "hw.h"

#include "i2c.h"
#include "startup/config.h"
#include <framework.h>
#include <lpc2xxx.h>

static tU8 whichArrow;
static tU8 cursor   = 0;

#define MAX_LENGTH 25

/*******************************************
 *                                         *
 *  Define game states:                    *
 *                                         *
 *******************************************/
#define GAME_NOT_STARTED 0
#define GAME_RUNNING     1
#define GAME_OVER        2
#define GAME_END         3
#define GAME_START       4
#define GAME_SCORED      5
#define GAME_NOT_SCORED  6
#define GAME_SHOW_SCORE  7
#define GAME_WON         8
#define GAME_LOST        9

#define SCREEN_WIDTH  ((tU8)130)
#define SCREEN_HEIGHT ((tU8)130)

#define BOARD_GRID_COLOR  ((tU8)0)    // black
#define BOARD_BKG_COLOR   ((tU8)0xff) // white
#define GAME_BKG_COLOR_OK    ((tU8)0x0c) // green
#define GAME_BKG_COLOR_ERR   ((tU8)0xe0) // red

#define CHAR_HEIGHT 14
#define CHAR_WIDTH   8
#define CENTER_X(numchars) ((SCREEN_WIDTH - numchars*CHAR_WIDTH)/2)

#define tU8   unsigned char
#define tU16  unsigned short
#define tU32  unsigned int
#define tS8   char
#define tS16  short
#define tS32  int
#define tBool unsigned char;
#define TRUE  1
#define FALSE 0

#define LED_GREEN  1
#define LED_RED    2

static tU8 gameStatus;
static tU8 gameMode;
static tBool arrow;
static tBool finish;
static int score;
static tU8 rounds;
static tU8 currentRound;
static tU8 eepromTest;

// --------------------------------------

void showPreStartupScreen(void){
  // clear screen
  lcdColor(GAME_BKG_COLOR_OK,BOARD_GRID_COLOR);
  lcdClrscr();
  lcdRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GAME_BKG_COLOR_OK);
  printf("setup screen method");

  // print message
  lcdGotoxy(CENTER_X(14), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
  lcdPuts("Welcome to the");
  lcdGotoxy(CENTER_X(10), SCREEN_HEIGHT/2 -4);
  lcdPuts("React Game");
  lcdGotoxy(CENTER_X(12), SCREEN_HEIGHT/2 + 12);
  lcdPuts("Press Center");

}

tU8 randomMove() {
    switch(rand() % 5) {
        case 0:
            whichArrow = KEY_UP;
            return KEY_UP;
        case 1:
            whichArrow = KEY_RIGHT;
            return KEY_RIGHT;
        case 2:
            whichArrow = KEY_DOWN;
            return KEY_DOWN;
        case 3:
            whichArrow = KEY_LEFT;
            return KEY_LEFT;
        case 4:
            whichArrow = KEY_CENTER;
            return KEY_CENTER;
    }
}

void showRandomArrow(whichArrow) {
    switch(whichArrow) {
        case KEY_UP:
            lcdColor(GAME_BKG_COLOR_OK,BOARD_GRID_COLOR);
            lcdClrscr();
            lcdRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GAME_BKG_COLOR_OK);
            lcdGotoxy(CENTER_X(2), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
            lcdPuts("UP");
            break;

        case KEY_RIGHT:
            lcdColor(GAME_BKG_COLOR_OK,BOARD_GRID_COLOR);
            lcdClrscr();
            lcdRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GAME_BKG_COLOR_OK);
            lcdGotoxy(CENTER_X(5), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
            lcdPuts("RIGHT");
            break;

        case KEY_DOWN:
            lcdColor(GAME_BKG_COLOR_OK,BOARD_GRID_COLOR);
            lcdClrscr();
            lcdRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GAME_BKG_COLOR_OK);
            lcdGotoxy(CENTER_X(4), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
            lcdPuts("DOWN");
            break;

        case KEY_LEFT:
            lcdColor(GAME_BKG_COLOR_OK,BOARD_GRID_COLOR);
            lcdClrscr();
            lcdRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GAME_BKG_COLOR_OK);
            lcdGotoxy(CENTER_X(4), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
            lcdPuts("LEFT");
            break;

        case KEY_CENTER:
            lcdColor(GAME_BKG_COLOR_OK,BOARD_GRID_COLOR);
            lcdClrscr();
            lcdRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GAME_BKG_COLOR_OK);
            lcdGotoxy(CENTER_X(6), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
            lcdPuts("CENTER");
            break;

        default:
            lcdColor(GAME_BKG_COLOR_OK,BOARD_GRID_COLOR);
            lcdClrscr();
            lcdRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GAME_BKG_COLOR_OK);
            lcdGotoxy(CENTER_X(2), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
            lcdPuts("UP");
            break;
    }
}

void useBuzzer(int MAX){
    int j;
    for (j = 0; j < MAX; j++) {
        setBuzzer(TRUE);
        osSleep(20);
        setBuzzer(FALSE);
        osSleep(20);
    }
}

void showScore() {

    tU8 str[13];

    str[0] = score / 100000 + '0';
    str[1] = (score / 10000) % 10 + '0';
    str[2] = (score / 1000) % 10 + '0';
    str[3] = (score / 100) % 10 + '0';
    str[4] = (score / 10) % 10 + '0';
    str[5] = score % 10 + '0';
    str[6] = 0;

    //remove leading zeroes
    if (str[0] == '0') {
        str[0] = ' ';
        if (str[1] == '0') {
            str[1] = ' ';
            if (str[2] == '0') {
                str[2] = ' ';
                if (str[3] == '0') {
                    str[3] = ' ';
                    if (str[4] == '0') {
                        str[4] = ' ';
                    }
                }
            }
        }
    }

    lcdGotoxy(CENTER_X(12), SCREEN_HEIGHT/2 -4);
    lcdPuts(str);
}


void playReactGame(void){
    // printf("\nPlay React-Game works!\n");
    showPreStartupScreen();

    arrow = TRUE;
    finish = TRUE;
    score = 0;
    char strScore[16];
    rounds = 6;
    currentRound = 0;

    eaInit();

    gameStatus = GAME_NOT_STARTED;

      while(gameStatus != GAME_END)
        {
          tU8 anyKey;

          anyKey = checkKey();

          switch(gameStatus){
              case GAME_NOT_STARTED:
                  if (anyKey == KEY_CENTER){
                      gameStatus = GAME_START;
                  }
                  break;

              case GAME_START:
                  if (currentRound == rounds){
                      gameStatus = GAME_SHOW_SCORE;
                      break;
                  }

                  if (arrow == TRUE){
                      randomMove();
                      showRandomArrow(whichArrow);
                      arrow = FALSE;
                  }
                  if( anyKey != KEY_NOTHING){
                      if (anyKey == whichArrow){
                          currentRound++;
                          gameStatus = GAME_SCORED;
                      }
                      else if (anyKey != whichArrow){
                          currentRound++;
                          gameStatus = GAME_NOT_SCORED;
                      }
                  }
                  break;

              case GAME_SCORED:
                  lcdColor(GAME_BKG_COLOR_OK,BOARD_GRID_COLOR);
                  lcdClrscr();
                  lcdRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GAME_BKG_COLOR_OK);
                  lcdGotoxy(CENTER_X(8), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
                  lcdPuts("SCORED !");
                  useBuzzer(1);

                  setLED(LED_GREEN,TRUE);
                  osSleep(200);
                  setLED(LED_GREEN,FALSE);

                  score++;
                  arrow = TRUE;
                  gameStatus = GAME_START;
                  break;

              case GAME_NOT_SCORED:
                  TIMER1_MCR = 0x04;

                  lcdColor(GAME_BKG_COLOR_OK,BOARD_GRID_COLOR);
                  lcdClrscr();
                  lcdRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GAME_BKG_COLOR_OK);
                  lcdGotoxy(CENTER_X(8), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
                  lcdPuts("FAILED !");
                  useBuzzer(2);

                  setLED(LED_RED,TRUE);
                  osSleep(200);
                  setLED(LED_RED,FALSE);

                  arrow = TRUE;
                  gameStatus = GAME_START;
                  break;

              case GAME_SHOW_SCORE:
                  lcdColor(GAME_BKG_COLOR_OK,BOARD_GRID_COLOR);
                  lcdClrscr();
                  lcdRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GAME_BKG_COLOR_OK);
                  lcdGotoxy(CENTER_X(13), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
                  lcdPuts("You scored: ");

                  lcdGotoxy(CENTER_X(12), SCREEN_HEIGHT/2 -4);

                  showScore();

                  osSleep(500);
                  if(score > 3){
                      gameStatus = GAME_WON;
                  }
                  else if (score <= 3){
                      gameStatus = GAME_LOST;
                  }
                  break;

              case GAME_WON:
                  lcdColor(GAME_BKG_COLOR_OK,BOARD_GRID_COLOR);
                  lcdClrscr();
                  lcdRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GAME_BKG_COLOR_OK);
                  lcdGotoxy(CENTER_X(8), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
                  lcdPuts("You won!");
             
                  saveScoreToEeprom("WIN");
                  gameStatus = GAME_OVER;
                  osSleep(500);
                  break;

              case GAME_LOST:
                  lcdColor(GAME_BKG_COLOR_OK,BOARD_GRID_COLOR);
                  lcdClrscr();
                  lcdRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GAME_BKG_COLOR_OK);
                  lcdGotoxy(CENTER_X(9), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
                  lcdPuts("You lost!");
                  saveScoreToEeprom("LOSE");
                  gameStatus = GAME_OVER;
                  osSleep(500);
                  break;

              case GAME_OVER:
                  if ( finish == TRUE ){
                      lcdColor(GAME_BKG_COLOR_OK,BOARD_GRID_COLOR);
                      lcdClrscr();
                      lcdRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GAME_BKG_COLOR_OK);
                      lcdGotoxy(CENTER_X(10), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
                      lcdPuts("Restart-Up");
                      lcdGotoxy(CENTER_X(12), SCREEN_HEIGHT/2 -4);
                      lcdPuts("EndGame-Down");
                      //TEST:
                      lcdGotoxy(CENTER_X(10), SCREEN_HEIGHT/2 + CHAR_HEIGHT);
                      readScoreFromEeprom();                   
                      //lcdPuts(eepromTest);
                      finish = FALSE;
                  }

                  if(anyKey == KEY_UP){
                          currentRound = 0 ;
                          score = 0;
                          finish= TRUE;
                          gameStatus = GAME_START;
                  }
                  else if (anyKey == KEY_DOWN){
                          gameStatus = GAME_END;
                  }
                  break;

              default:
                  gameStatus = GAME_END;
                  break;
            }
        }
    return;
}




void saveScoreToEeprom(tS8 score[]) {

    tS8 errorCode;
    errorCode = eepromWrite(0x0004, score, sizeof(score));
    if(!errorCode == I2C_CODE_OK){
        printf("EEPROM write error! \n");
    }

    return;


}

void readScoreFromEeprom(void){
    tS8 errorCode;
    tU8 testBuf[MAX_LENGTH];
    errorCode = eepromPageRead(0x0004, testBuf, MAX_LENGTH);
    if(!errorCode == I2C_CODE_OK){
     printf("\n%d,%d,%d,%d,%d %d,%d,%d,%d,%d %d",
             testBuf[0],testBuf[1],testBuf[2],testBuf[3],testBuf[4],testBuf[5],testBuf[6],testBuf[7],testBuf[8],testBuf[9],testBuf[10]);
      printf("\n%c%c%c%c%c%c%c%c%c%c%c",
             testBuf[0],testBuf[1],testBuf[2],testBuf[3],testBuf[4],testBuf[5],testBuf[6],testBuf[7],testBuf[8],testBuf[9],testBuf[10]);
    }
    lcdPuts(testBuf);
    // eepromTest = testBuf;
    return;
}


