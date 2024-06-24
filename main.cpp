#include "timerISR.h"
#include "periph.h"

int currRow = 4;
int newPiece = 1;
int currPiece = 0;
int state;
int time = 6;
int currTime = 0;
bool gameOver = true;
int score = 0;


#define NUM_TASKS 6 //TODO: Change to the number of tasks being used
//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;

const unsigned long GCD_PERIOD = 5;//TODO:Set the GCD Period

task tasks[NUM_TASKS]; // declared task array with 5 tasks

enum DisplayRows{Display_Row};
int TickFct_displayRow(int state);

enum DisplayColumns{Display_Column};
int TickFct_displayColumn(int state);

enum Constructor{MakePiece};
int TickFct_Make(int state);

enum LowerPieces{Fall};
int TickFct_lowerPiece(int state);

enum ShiftPieces{Wait, HoldLeft, HoldRight};
int TickFct_shiftPiece(int state);

enum DestroyPiece{Destroy};
int TickFct_destroyPiece(int state);



void TimerISR() {
	for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {                   // Iterate through each task in the task array
		if ( tasks[i].elapsedTime >= tasks[i].period ) {           // Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state); // Tick and set the next state for this task
			tasks[i].elapsedTime = 0;                          // Reset the elapsed time for the next tick
		}
		tasks[i].elapsedTime += GCD_PERIOD;                        // Increment the elapsed time by GCD_PERIOD
	}
}

int piecesOrder[60] = {
        6, 6, 6, 6, 8, 8, 11, 13, 5, 12,
        4, 17, 15, 18, 13, 20, 16, 5, 15, 7,
        14, 13, 3, 3, 5, 2, 7, 11, 12, 2,
        1, 3, 6, 20, 20, 2, 5, 1, 13, 16,
        16, 11, 14, 15, 6, 2, 3, 7, 2, 5,
        1, 1, 11, 4, 15, 19, 4, 15, 19, 2
    };



int gameBoard[12][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}  
};


unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
   return (b ?  (x | (0x01 << k))  :  (x & ~(0x01 << k)) );
              //   Set bit to 1           Set bit to 0
}

unsigned char GetBit(unsigned char x, unsigned char k) {
   return ((x & (0x01 << k)) != 0);
}

void updateMatrix(unsigned char val[8]) {
    PORTD = SetBit(PORTD, 4, 0); 
    for (int i = 0; i < 8; i++) {
        PORTD = SetBit(PORTD, 2, val[i]); 
        PORTD = SetBit(PORTD, 3, 0); 
        PORTD = SetBit(PORTD, 3, 1); 
    }
    PORTD = SetBit(PORTD, 4, 1);
}

void updateMatrix2(unsigned char val[8]) {
    PORTD = SetBit(PORTD, 7, 0); 
    for (int i = 0; i < 8; i++) {
        PORTD = SetBit(PORTD, 5, val[i]); 
        PORTD = SetBit(PORTD, 6, 0); 
        PORTD = SetBit(PORTD, 6, 1); 
    }
    PORTD = SetBit(PORTD, 7, 1);
}


int main(void) {
    
    DDRB = 0x00; PORTB = 0xFF;
    DDRD = 0xFF; PORTD = 0x00;

    unsigned char val[8] = {1, 1, 1, 1, 1, 1, 1, 0};
    tasks[0].period = 5;
    tasks[0].state = Display_Row;
    tasks[0].elapsedTime = 5;
    tasks[0].TickFct = &TickFct_displayRow;
    tasks[1].period = 5;
    tasks[1].state = Display_Column;
    tasks[1].elapsedTime = 5;
    tasks[1].TickFct = &TickFct_displayColumn;
    tasks[2].period = 500;
    tasks[2].state = MakePiece;
    tasks[2].elapsedTime = 500;
    tasks[2].TickFct = &TickFct_Make;
    tasks[3].period = 1000;
    tasks[3].state = Wait;
    tasks[3].elapsedTime = 1000;
    tasks[3].TickFct = &TickFct_shiftPiece;
    tasks[4].period = 500;
    tasks[4].state = Fall;
    tasks[4].elapsedTime = 500;
    tasks[4].TickFct = &TickFct_lowerPiece;
    tasks[5].TickFct = &TickFct_shiftPiece;
    tasks[5].period = 1000;
    tasks[5].state = Destroy;
    tasks[5].elapsedTime = 1000;
    tasks[5].TickFct = &TickFct_destroyPiece;
    TimerSet(GCD_PERIOD);
    TimerOn();

    while (1) {
        /*updateMatrix(val);
        _delay_ms(5000);*/
    }

    return 0;
}

int TickFct_displayRow (int state)
{
    PORTD = SetBit(PORTD, 4, 0); 
    for (int i = 0; i < 8; i++) {
        if (i == currRow - 4)
        {
            PORTD = SetBit(PORTD, 2, 0); 
        }
        else
        {
            PORTD = SetBit(PORTD, 2, 1);
        }
        PORTD = SetBit(PORTD, 3, 0); 
        PORTD = SetBit(PORTD, 3, 1); 
    }
    PORTD = SetBit(PORTD, 4, 1);
    return state;
}

int TickFct_displayColumn (int state)
{
    PORTD = SetBit(PORTD, 7, 0); 
    for (int i = 0; i < 8; i++) {
        if (gameBoard[currRow][i] >= 1)
        {
            PORTD = SetBit(PORTD, 5, 1); 
        }
        else
        {
            PORTD = SetBit(PORTD, 5, 0);
        }
        PORTD = SetBit(PORTD, 6, 0); 
        PORTD = SetBit(PORTD, 6, 1); 
    }
    PORTD = SetBit(PORTD, 7, 1);
    if (currRow == 11)
    {
        currRow = 4;
    }
    else
    {
        currRow++;
    }
    return state;
}


int TickFct_Make (int state)
{
    if ((newPiece == 1) && !gameOver)
    {
        switch (piecesOrder[currPiece]) {
            case 1: 
                gameBoard[0][4] = 2; 
                gameBoard[1][4] = 3; 
                gameBoard[2][4] = 4; 
                gameBoard[3][4] = 5; 
                break;
            case 2: 
                gameBoard[1][5] = 2; 
                gameBoard[1][4] = 3; 
                gameBoard[2][4] = 4; 
                gameBoard[3][4] = 5; 
                break;
            case 3: 
                gameBoard[1][3] = 2; 
                gameBoard[1][4] = 3; 
                gameBoard[2][4] = 4; 
                gameBoard[3][4] = 5; 
                break;
            case 4: 
                gameBoard[0][3] = 2; 
                gameBoard[0][4] = 3; 
                gameBoard[1][4] = 4; 
                gameBoard[1][5] = 5; 
                break;
            case 5: 
                gameBoard[1][3] = 2; 
                gameBoard[1][4] = 3; 
                gameBoard[0][4] = 4; 
                gameBoard[0][5] = 5; 
                break;
            case 6: 
                gameBoard[0][4] = 2; 
                gameBoard[1][4] = 3; 
                gameBoard[0][3] = 4; 
                gameBoard[1][3] = 5; 
                break;
            case 7: 
                gameBoard[0][4] = 2; 
                gameBoard[1][3] = 3; 
                gameBoard[1][4] = 4; 
                gameBoard[1][5] = 5; 
                break;
            case 8:
                gameBoard[0][3]=2;
                gameBoard[0][4]=3;
                gameBoard[0][5]=4;
                gameBoard[0][6]=5;
                break;
            case 9:
                gameBoard[0][3]=2;
                gameBoard[0][4]=3;
                gameBoard[0][5]=4;
                gameBoard[1][4]=5;
                break;
            case 10:
                gameBoard[1][3]=2;
                gameBoard[1][4]=3;
                gameBoard[1][5]=4;
                gameBoard[0][3]=5;
                break;
            case 11:
                gameBoard[1][3]=2;
                gameBoard[1][4]=3;
                gameBoard[1][5]=4;
                gameBoard[0][5]=5;
                break;
            case 12:
                gameBoard[0][3]=2;
                gameBoard[1][3]=3;
                gameBoard[1][2]=4;
                gameBoard[2][2]=5;
                break;
            case 13:
                gameBoard[0][4]=2;
                gameBoard[1][4]=3;
                gameBoard[1][5]=4;
                gameBoard[2][5]=5;
                break;
            case 14:
                gameBoard[0][3]=2;
                gameBoard[1][3]=3;
                gameBoard[2][3]=4;
                gameBoard[1][4]=5;
                break;
            case 15:
                gameBoard[0][4]=2;
                gameBoard[1][4]=3;
                gameBoard[2][4]=4;
                gameBoard[1][3]=5;
                break;
            case 16:
                gameBoard[0][3]=2;
                gameBoard[0][4]=3;
                gameBoard[0][5]=4;
                gameBoard[1][4]=5;
                break;
            case 17:
                gameBoard[0][3]=2;
                gameBoard[0][4]=3;
                gameBoard[0][5]=4;
                gameBoard[1][3]=5;
                break;
            case 18:
                gameBoard[0][3]=2;
                gameBoard[0][4]=3;
                gameBoard[0][5]=4;
                gameBoard[1][5]=5;
                break;
            case 19:
                gameBoard[0][3]=2;
                gameBoard[1][3]=3;
                gameBoard[2][3]=4;
                break;
            case 20:
                gameBoard[0][3]=2;
                break;
            default:
                break;
        }
        if (currPiece == 58)
        {
            currPiece = 0;
        }
        else {
            currPiece ++;
        }
        state = 1;
    }
    newPiece = 0;
    return state;
}

int TickFct_lowerPiece (int state)
{
    if (currTime >= time) {
    bool block = false;
    for (int i = 0; i < 12; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if ((gameBoard[i][j] > 1) && ((gameBoard[i + 1][j] == 1) || ((i+1)>11)))
            {
                block = true;
            } 
        }
    }
    if (block)
    {
        for (int i = 0; i < 12; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                if (gameBoard[i][j] >= 2)
                {
                    gameBoard[i][j] = 1;
                } 
            }
        }
        newPiece = 1;
    }
    else
    {
        int temp;
        for (int i = 11; i >= 0; i--)
        {
            for (int j = 0; j < 8; j++)
            {
                if ((gameBoard[i][j] > 1))
                {
                    temp = gameBoard[i][j];
                    gameBoard[i][j] = 0;
                    gameBoard[i+1][j] = temp;
                } 
            }
        }
    }
    for (int i = 0; i < 12; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if ((gameBoard[i][j] == 1) && (i<4))
            {
                gameOver = true;
            } 
        }
    }

    for (int i = 11; i >=0 ; i--)
    {
        bool destroy = true;
        for (int j = 0; j < 8; j++)
        {
            if ((gameBoard[i][j] != 1))
            {
                destroy = false;
            }
        }
        if (destroy)
        {
            score++;
            for (int k = i; k < 12; k++)
            {
                for (int l = 0; l < 8; l++)
                {
                    gameBoard[k][l] = gameBoard[k-1][l];
                }
            }
        }
    }
    if (score >= 10)
    {
        time = 2;
    }
    else if (score >=2)
    {
        time = 3;
    }
    else
    {
        time = 4;
    }
    currTime = 0;
    }
    else
    {
        currTime++;
    }
    return state;
}



int TickFct_shiftPiece (int state)
{
    switch (state){
        case Wait:
        bool noMove = false;
        if (GetBit(PINB, 0))
        {
            for (int i = 0; i < 8; i++)
            {
                for (int j = 0; j < 12; j++)
                {
                    if ((gameBoard[j][i] > 1) && ((gameBoard[j][i-1] == 1) || (i-1 < 0)))
                    {
                        noMove = true;
                    }
                }
            }
            if (noMove == false)
            {
                int temp;
                for (int k = 11; k >= 0; k--)
                {
                    for (int l = 0; l < 8; l++)
                    {
                        if (gameBoard[k][l] > 1)
                        {
                            temp = gameBoard[k][l];
                            gameBoard[k][l] = 0;
                            gameBoard[k][l-1] = temp;
                        }
                    }
                }
            }
        }
        else if (GetBit(PINB, 1))
        {
            for (int i = 7; i >=0; i--)
            {
                for (int j = 0; j < 12; j++)
                {
                    if ((gameBoard[j][i] > 1) && ((gameBoard[j][i+1] == 1) || (i+1>7)))
                    {
                        noMove = true;
                    }
                }
            }
            if (noMove == false)
            {
                int temp;
                for (int k = 0; k < 12; k++)
                {
                    for (int l = 7; l >= 0; l--)
                    {
                        if (gameBoard[k][l] > 1)
                        {
                            temp = gameBoard[k][l];
                            gameBoard[k][l] = 0;
                            gameBoard[k][l+1] = temp;
                        }
                    }
                }
            }
        }
        break;

    }
    return state;
}

int TickFct_destroyPiece(int state)
{
    if (GetBit(PINB, 2))
    {
        for (int i = 0; i < 12; i++)
        {
            for (int k = 0; k < 8; k++)
            {
                if (gameBoard[i][k] > 1)
                {
                    gameBoard[i][k] = 0;
                }
            }
        }
        newPiece = 1;
    }
    if (GetBit(PINB, 2) && gameOver)
    {
        score = 0;
        for (int i = 0; i < 12; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                gameBoard[i][j] = 0;
            }
        }
        gameOver = false;
        time = 6;
        newPiece = 1;
    }
    return state;
}


/*int TickFct_rotatePiece(int state)
{
    if (GetBit(PINC, 2))
    {
        switch (piecesOrder[currRow]){
            case 1:

            for (int i = 0; i < 12; i++)
            {
                for (int k = 0; k < 8; k++)
                {
                    if 
                }
            }
        }
    }
}*/