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

#define GAME_NOT_STARTED 0
#define GAME_RUNNING     1
#define GAME_OVER        2
#define GAME_END         3
#define GAME_START 		 4
#define GAME_SCORED 	 5
#define GAME_NOT_SCORED  6
#define GAME_SHOW_SCORE  7
#define GAME_WON		 8
#define GAME_LOST        9

#define MODE_MOVING      0
#define MODE_SELECTING   1

#define SCREEN_WIDTH  ((tU8)130)
#define SCREEN_HEIGHT ((tU8)130)
#define NUM_COLS ((tU8)4)
#define NUM_ROWS ((tU8)4)

#define BORDER ((tU8)1)
#define BOARD_X_OFFSET  ((tU8)4)
#define BOARD_Y_OFFSET  ((tU8)4)

#define CELL_WIDTH   ((tU8)(130-(BOARD_X_OFFSET*2)-(NUM_COLS+1)*BORDER)/NUM_COLS)
#define CELL_HEIGHT  ((tU8)(130-(BOARD_Y_OFFSET*2)-(NUM_ROWS+1)*BORDER)/NUM_ROWS)

#define BOARD_WIDTH  ((tU8)((NUM_COLS+1)*BORDER+NUM_COLS*CELL_WIDTH))
#define BOARD_HEIGHT ((tU8)((NUM_ROWS+1)*BORDER+NUM_ROWS*CELL_HEIGHT))

#define BOARD_GRID_COLOR  ((tU8)0)    // black
#define BOARD_BKG_COLOR   ((tU8)0xff) // white
#define GAME_BKG_COLOR_OK    ((tU8)0x0c) // green
#define GAME_BKG_COLOR_ERR   ((tU8)0xe0) // red

#define SELECTED_BKG_COLOR  ((tU8)0xfc) // yellow
#define FONT_FIXED_COLOR    ((tU8)0xe0) // black
#define FONT_UNFIXED_COLOR  ((tU8)0x02) // blue

#define CELL_X_OFFSET(col)  (BOARD_X_OFFSET + BORDER + (col*BORDER) + ((col)*CELL_WIDTH))
#define CELL_Y_OFFSET(row)  (BOARD_Y_OFFSET + BORDER + (row*BORDER) + ((row)*CELL_WIDTH))

#define CELL_PAD_X  ((tU8)3)
#define CELL_PAD_Y  ((tU8)3)

#define CHAR_HEIGHT 14
#define CHAR_WIDTH   8
#define CENTER_X(numchars) ((SCREEN_WIDTH - numchars*CHAR_WIDTH)/2)


#define IS_SELECTED(row,col) (row==currRow && col==currCol)

static tU8 gameStatus;
static tU8 gameMode;

static tU8 currRow;
static tU8 currCol;

static tU8 selectedNum;

#define BOARD_SIZE               4
tU8 matrix[BOARD_SIZE][BOARD_SIZE];

// ------------------------------------
#include <lpc2xxx.h>
#include <printf_P.h>
#include <ea_init.h>
#include "startup/config.h"



// TIMER DEFINES
#define tU8   unsigned char
#define tU16  unsigned short
#define tU32  unsigned int
#define tS8   char
#define tS16  short
#define tS32  int
#define tBool unsigned char;
#define TRUE  1
#define FALSE 0

// --------------------------------------

static tBool arrow;
static int score;
static tU8 rounds;
static tU8 currentRound;

#define LED_GREEN  1
#define LED_RED    2

// --------------------------------------

void showPreStartupScreen(void){
    // clear screen
    lcdColor(GAME_BKG_COLOR_OK,BOARD_GRID_COLOR);
    lcdClrscr();
    lcdRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GAME_BKG_COLOR_OK);
    printf("setup screen method");

    // print message
    lcdGotoxy(CENTER_X(12), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
    lcdPuts("Welcome to the");
    lcdGotoxy(CENTER_X(12), SCREEN_HEIGHT/2 -4);
    lcdPuts("React Game");
    lcdGotoxy(CENTER_X(12), SCREEN_HEIGHT/2 +12);
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
            lcdGotoxy(CENTER_X(12), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
            lcdPuts("KEY_UP");
            break;

        case KEY_RIGHT:
            lcdColor(GAME_BKG_COLOR_OK,BOARD_GRID_COLOR);
            lcdClrscr();
            lcdRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GAME_BKG_COLOR_OK);
            lcdGotoxy(CENTER_X(12), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
            lcdPuts("KEY_RIGHT");
            break;

        case KEY_DOWN:
            lcdColor(GAME_BKG_COLOR_OK,BOARD_GRID_COLOR);
            lcdClrscr();
            lcdRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GAME_BKG_COLOR_OK);
            lcdGotoxy(CENTER_X(12), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
            lcdPuts("KEY_DOWN");
            break;

        case KEY_LEFT:
            lcdColor(GAME_BKG_COLOR_OK,BOARD_GRID_COLOR);
            lcdClrscr();
            lcdRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GAME_BKG_COLOR_OK);
            lcdGotoxy(CENTER_X(12), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
            lcdPuts("KEY_LEFT");
            break;

        case KEY_CENTER:
            lcdColor(GAME_BKG_COLOR_OK,BOARD_GRID_COLOR);
            lcdClrscr();
            lcdRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GAME_BKG_COLOR_OK);
            lcdGotoxy(CENTER_X(12), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
            lcdPuts("KEY_CENTER");
            break;

        default:
            lcdColor(GAME_BKG_COLOR_OK,BOARD_GRID_COLOR);
            lcdClrscr();
            lcdRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GAME_BKG_COLOR_OK);
            lcdGotoxy(CENTER_X(12), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
            lcdPuts("KEY_UP");
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
    printf("\nPlay React-Game works!\n");

    showPreStartupScreen();

    arrow = TRUE;
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
                lcdGotoxy(CENTER_X(12), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
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
                lcdGotoxy(CENTER_X(12), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
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
                lcdGotoxy(CENTER_X(12), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
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
                lcdGotoxy(CENTER_X(12), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
                lcdPuts("You won!");
                osSleep(500);
                break;

            case GAME_LOST:
                lcdColor(GAME_BKG_COLOR_OK,BOARD_GRID_COLOR);
                lcdClrscr();
                lcdRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GAME_BKG_COLOR_OK);
                lcdGotoxy(CENTER_X(12), SCREEN_HEIGHT/2 - CHAR_HEIGHT-8);
                lcdPuts("You lost!");
                osSleep(500);
                break;
        }
    }
    return;
}


