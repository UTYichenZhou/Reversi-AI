#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <sys/time.h>

typedef struct flipped_piece{
    int row, col;
    char origColour;
    struct flipped_piece* next;
}flippedPiece;

typedef struct flipped_list{
    flippedPiece* head;
}flippedList;

typedef struct past_moves{
    int row, col;
    flippedList flips;
}PastMoves;

void insertFlipped(int ro, int co, char colour, flippedList* list);
void printBoard(int n, char[][26]);
bool positionInBounds(int n, int row, int col);
bool myTurn(int n, char board[][26], int spotValue[][8], char myColour, char humanColour, PastMoves pastMoves[7], short deltaRows[8], short deltaCols[8], int* minRow, int* maxRow, int* minCol, int* maxCol, int* humanNum, int* myNum, char* winner);
bool isPlayerSpot(int n, char board[][26], char playerColour, int row, int col, short deltaRows[8], short deltaCols[8]);
bool canPlayerMove(int n, char board[][26], char playerColour, short deltaRows[8], short deltaCols[8], int minRow, int maxRow, int minCol, int maxCol);
void getHumanMove(int n, char board[][26], short deltaRows[8], short deltaCols[8], char humanColour, char myColour, int *minRow, int *maxRow, int *minCol, int *maxCol, char *enter, int* humanNum, int* myNum, char *winner);
int boardScore(int n, char board[][26], int spotValue[][8], char myColour, char humanColour, int myNum, int humanNum);
void countConstPieces(int* myConstPieces, int*humanConstPieces, int*myPosScore, int*humanPosScore, char myColour, char humanColour, char board[][26], int spotValue[][8], bool isVisited[][8]);
void mobilityAndPositionMinimax(int n, char board[][26], int* mySpot, int* humanSpot, int* mySpotScore, int* humanSpotScore, int spotValue[][8], char myColour, char humanColour, int* myPosScore, int* humanPosScore, bool isVisited[][8]);
int miniMax(int min, int max, int depth, bool maximize, bool timeOut, /*double* timeStart,*/ int* bestRow, int* bestCol, int* myNum, int* humanNum, int n, char board[][26], PastMoves *firstMove, int spotValue[][8], short deltaRows[8], short deltaCols[8], char myColour, char humanColour, int minRow, int minCol, int maxRow, int maxCol);
int flipPieceMiniMax(int n, char board[][26], short deltaRows[8], short deltaCols[8], char playerColour, char opponentColour, int row, int col, PastMoves* currentMove);
void reverseBoardConfig(char board[][26], PastMoves *currentMove, int numFlipped, int *playerNum, int *opponentNum);
int flipPieces(int n, char board[][26], short deltaRows[8], short deltaCols[8], char colour, int row, int col);
int piecesICanEat(int n, char board[][26], char myColour, int row, int col, short deltaRows[8], short deltaCols[8]);
void initializeBoard(int n, char board[][26]);
bool myTurnDumb(int n, char board[][26], char humanColour, char myColour, short deltaRows[8], short deltaCols[8], int* minRow, int* maxRow, int* minCol, int* maxCol, int* humanNum, int* myNum, char* winner);
char findWinner(int myNum, int humanNum);

void insertFlipped(int ro, int co, char colour, flippedList* list){
    if(list->head == NULL){
        list->head = (flippedPiece*)malloc(sizeof(flippedPiece));
        list->head->row = ro;
        list->head->col = co;
        list->head->origColour = colour;
        list->head->next = NULL;
    }
    else{
        flippedPiece *newHead = (flippedPiece *)malloc(sizeof(flippedPiece));
        newHead->row = ro;
        newHead->col = co;
        newHead->origColour = colour;
        newHead->next = list->head;
        list->head = newHead;
    }
}

void addNewPiece(int ro, int co, PastMoves* currMove){
    currMove->row = ro;
    currMove->col = co;
}
//CHANGE BACK IN THE FUTURE
void printBoard(int n, char board[][26]){
    printf("  ");
    for(char i = 'a'; i <'a'+n; i++)
        printf("%c",i);
    printf("\n");
    for(char i = 'a'; i < 'a'+n; i++){
        printf("%c ", i);
        for(char j = 'a'; j < 'a'+n; j++)
            printf("%c", board[i-'a'][j-'a']);
        printf("\n");
    }
}

bool positionInBounds(int n, int row, int col){
    return row >= 0 && row < n && col >= 0 && col < n;
}

int boardScore(int n, char board[][26], int spotValue[][8], char myColour, char humanColour, int myNum, int humanNum){
    int myPosScore = 0, humanPosScore = 0, mySpots = 0, humanSpots = 0, mySpotScore = 0, humanSpotScore = 0, myConstPieces = 0, humanConstPieces = 0;
    bool isVisited[8][8] = {{0},{0},{0},{0},{0},{0},{0},{0}};
    countConstPieces(&myConstPieces, &humanConstPieces, &myPosScore, &humanPosScore, myColour, humanColour, board, spotValue, isVisited);
    if(myConstPieces >= 33)
        return 10000; //VALUE MAY NEED MODIFICATION
    else if(humanConstPieces >= 33)
        return -10000; //VALUES MAY NEED MODIFICATION
    mobilityAndPositionMinimax(n, board, &mySpots, &humanSpots, &mySpotScore, &humanSpotScore, spotValue, myColour, humanColour, &myPosScore, &humanPosScore, isVisited);
    return myPosScore - (int)(0.8*humanPosScore) + 10*(mySpots - humanSpots) + (int)(0.5*(mySpotScore - humanSpotScore));
}

void countConstPieces(int* myConstPieces, int*humanConstPieces, int*myPosScore, int*humanPosScore, char myColour, char humanColour, char board[][26], int spotValue[][8], bool isVisited[][8]){
    if(board[0][0] == myColour){
        int mRow = 7;
        (*myPosScore) += spotValue[0][0];
        (*myConstPieces)++;
        isVisited[0][0] = 1;
        for(int i = 1; i < mRow; i++){ //check first row
            if(board[0][i] != myColour){
                if(i == 1)
                    mRow = 1;
                else
                    mRow = i-1;
                break;
            }
            (*myConstPieces)++;
            (*myPosScore) += 10;
            isVisited[0][i] = 1;
        }
        if(mRow == 7)
            mRow--;
        for(int i = 1; i < 7 && board[i][0] == myColour; i++){ //check start of each row
            for(int j = 0; j < mRow; j++){
                if(board[i][j] != myColour){
                    mRow = j;
                    break;
                }
                (*myConstPieces)++;
                (*myPosScore) += 10;
                isVisited[i][j] = 1;
            }
            if(mRow > 1)
                mRow--;
        }
    }
    else if(board[0][0] == humanColour){
        int mRow = 7;
        (*humanPosScore) += spotValue[0][0];
        (*humanConstPieces)++;
        isVisited[0][0] = 1;
        for(int i = 1; i < mRow; i++){ //check first row
            if(board[0][i] != humanColour){
                if(i == 1)
                    mRow = 1;
                else
                    mRow = i-1;
                break;
            }
            (*humanPosScore) += 10;
            (*humanConstPieces)++;
            isVisited[0][i] = 1;
        }
        if(mRow == 7)
            mRow--;
        for(int i = 1; i < 7 && board[i][0] == humanColour; i++){ //check start of each row
            for(int j = 0; j < mRow; j++){
                if(board[i][j] != humanColour){
                    mRow = j;
                    break;
                }
                (*humanConstPieces)++;
                (*humanPosScore) += 10;
                isVisited[i][j] = 1;
            }
            if(mRow > 1)
                mRow--;
        }
    }
    //top right corner
    if(board[7][0] == myColour){
        int mRow = 1;
        isVisited[0][7] = 1;
        (*myConstPieces)++;
        (*myPosScore) += spotValue[7][0];
        for(int i = 6; i > mRow && !isVisited[0][i]; i--){
            if(board[0][i] != myColour){
                if(i == 6)
                    mRow = 6;
                else
                    mRow = i+1;
                break;
            }
            (*myConstPieces)++;
            (*myPosScore) += 10;
            isVisited[0][i] = 1;
        }
        if(mRow == 1)
            mRow++;
        for(int i = 1; i < 7 && board[i][7] == myColour; i++){
            for(int j = 7; j > mRow; j--){
                if(board[i][j] != myColour || isVisited[i][j]){
                    mRow = j;
                    break;
                }
                (*myPosScore) += 10;
                (*myConstPieces)++;
                isVisited[i][j] = 1;
            }
            mRow++;
        }
    }
    else if(board[7][0] == humanColour){
        int mRow = 1;
        (*humanPosScore) += spotValue[7][0];
        (*humanConstPieces)++;
        isVisited[0][7] = 1;
        for(int i = 6; i > mRow && !isVisited[0][i]; i--){
            if(board[0][i] != humanColour){
                if(i == 6)
                    mRow = 6;
                else
                    mRow = i+1;
                break;
            }
            (*humanPosScore) += 10;
            (*humanConstPieces)++;
            isVisited[0][i] = 1;
        }
        if(mRow == 1)
            mRow++;
        for(int i = 1; i < 7 && board[i][7] == humanColour; i++){
            for(int j = 7; j > mRow; j--){
                if(board[i][j] != humanColour || isVisited[i][j]){
                    mRow = j;
                    break;
                }
                (*humanPosScore) += 10;
                (*humanConstPieces)++;
                isVisited[i][j] = 1;
            }
            if(mRow < 6)
                mRow++;
        }
    }
    //bottom left corner
    if(board[7][0] == myColour){
        int mRow = 7;
        (*myPosScore) += spotValue[7][0];
        isVisited[7][0] = 1;
        for(int i = 1; i < mRow; i++){
            if(board[7][i] != myColour){
                if(i == 1)
                    mRow = 1;
                else
                    mRow = i-1;
                break;
            }
            (*myPosScore) += 10;
            (*myConstPieces)++;
            isVisited[7][i] = 1;
        }
        if(mRow == 7)
            mRow--;
        for(int i = 6; i > 1 && !isVisited[i][0] && board[i][0] == myColour; i--){
            for(int j = 0; j < mRow; j++){
                if(board[i][j] != myColour){
                    mRow = j;
                    break;
                }
                (*myPosScore) += 10;
                (*myConstPieces)++;
                isVisited[i][j] = 1;
            }
            if(mRow > 1)
                mRow--;
        }
    }
    else if(board[7][0] == humanColour){
        int mRow = 7;
        (*humanPosScore) += spotValue[7][0];
        (*humanConstPieces)++;
        isVisited[7][0] = 1;
        for(int i = 1; i < mRow; i++){
            if(board[7][i] != humanColour){
                if(i == 1)
                    mRow = 1;
                else
                    mRow = i-1;
                break;
            }
            (*humanPosScore) += 10;
            (*humanConstPieces)++;
            isVisited[7][i] = 1;
        }
        if(mRow == 7)
            mRow--;
        for(int i = 6; i > 1 && !isVisited[i][0] && board[i][0] == humanColour; i--){
            for(int j = 0; j < mRow; j++){
                if(board[i][j] != humanColour){
                    mRow = j;
                    break;
                }
                (*humanPosScore) += 10;
                (*humanConstPieces)++;
                isVisited[i][j] = 1;
            }
            if(mRow > 1)
                mRow--;
        }
    }
    //bottom right corner
    if(board[7][7] == myColour){
        *myPosScore += spotValue[7][7];
        int mRow = 1;
        (*myConstPieces)++;
        isVisited[7][7] = 1;
        for(int i = 6; i > mRow; i--){
            if(board[7][i] != myColour && !isVisited[7][i]){
                if(i == 6)
                    mRow = 6;
                else
                    mRow = i+1;
                break;
            }
            (*myPosScore) += 10;
            (*myConstPieces)++;
            isVisited[7][i] = 1;
        }
        if(mRow == 1)
            mRow++;
        for(int i = 6; i > 1 && !isVisited[i][7] && board[i][7] == myColour; i--){
            for(int j = 7; j > mRow; j--){
                if(board[i][j] != myColour || isVisited[i][j]){
                    mRow = j;
                    break;
                }
                (*myPosScore) += 10;
                (*myConstPieces)++;
                isVisited[i][j] = 1;
            }
            if(mRow < 6)
                mRow++;
        }
    }
    else if(board[7][7] == humanColour){
        (*humanPosScore) += spotValue[7][7];
        int mRow = 1;
        (*humanConstPieces)++;
        isVisited[7][7] = 1;
        for(int i = 6; i > mRow; i--){
            if(board[7][i] != humanColour && !isVisited[7][i]){
                if(i == 6)
                    mRow = 6;
                else
                    mRow = i+1;
                break;
            }
            (*humanPosScore) += 10;
            (*humanConstPieces)++;
            isVisited[7][i] = 1;
        }
        if(mRow == 1)
            mRow++;
        for(int i = 6; i > 1 && !isVisited[i][7] && board[i][7] == humanColour; i--){
            for(int j = 7; j > mRow; j--){
                if(board[i][j] != humanColour || isVisited[i][j]){
                    mRow = j;
                    break;
                }
                (*humanPosScore) += 10;
                (*humanConstPieces)++;
                isVisited[i][j] = 1;
            }
            if(mRow < 6)
                mRow++;
        }
    }
}

void mobilityAndPositionMinimax(int n, char board[][26], int* mySpot, int* humanSpot, int* mySpotScore, int* humanSpotScore, int spotValue[][8], char myColour, char humanColour, int* myPosScore, int* humanPosScore, bool isVisited[][8]){
    short deltaRows[] = {-1,-1,-1,0,1,1,1,0}, deltaCols[] = {-1,0,1,1,1,0,-1,-1};
    bool isMine = 0, isHumans = 0;
    char colour = 'U';
    int cnt;
    for(int i = 0; i < 8; i++){
        for(int j = 0; j < 8; j++){
            if(board[i][j] == 'U'){
                for(int k = 0; k < 8; k++){
                    cnt = 1;
                    while(positionInBounds(n, i+cnt*deltaRows[k], j+cnt*deltaCols[k]) && (!isHumans || !isMine)){
                        if(board[i+cnt*deltaRows[k]][j+cnt*deltaCols[k]] == 'U' ||
                           (cnt == 1 && ((isHumans && board[i+cnt*deltaRows[k]][j+cnt*deltaCols[k]] == myColour) ||
                                         (isMine && board[i+cnt*deltaRows[k]][j+cnt*deltaCols[k]] == humanColour))))
                            break;
                        else if(colour == myColour && board[i+cnt*deltaRows[k]][j+cnt*deltaCols[k]] == humanColour){
                            (*humanSpot)++;
                            isHumans = 1;
                            (*humanSpotScore) += spotValue[i+cnt*deltaRows[k]][j + cnt*deltaCols[k]];
                            break;
                        }
                        else if(colour == humanColour && board[i+cnt*deltaRows[k]][j+cnt*deltaCols[k]] == myColour){
                            (*mySpot)++;
                            isMine = 1;
                            (*mySpotScore) += spotValue[i+cnt*deltaRows[k]][j + cnt*deltaCols[k]];
                            break;
                        }
                        else if(colour == 'U')
                            colour = board[i+cnt*deltaRows[k]][j+cnt*deltaCols[k]];
                        cnt++;
                    }
                }
            }
            else if(board[i][j] == myColour && !isVisited[i][j]){
                (*myPosScore) += spotValue[i][j];
            }
            else if(board[i][j] == humanColour && !isVisited[i][j]){
                (*humanPosScore) += spotValue[i][j];
            }
        }
    }
}

int miniMax(int min, int max, int depth, bool maximize, bool timeOut, /*double* timeStart,*/ int* bestRow, int* bestCol, int* myNum, int* humanNum, int n, char board[][26], PastMoves *firstMove, int spotValue[][8], short deltaRows[8], short deltaCols[8], char myColour, char humanColour, int minRow, int minCol, int maxRow, int maxCol){
    int numFlipped = 0;
    bool canMove = 0;
    if(depth == 5) //return the heuristic value of the game board if it's the seventh move
        return boardScore(n, board, spotValue, myColour, humanColour, *myNum, *humanNum);
    else if(maximize){
        int currMax = -10000;
        for(int i = 0; i < 8; i++){
            for(int j = 0; j < 8; j++){

                //CHECK TIME

                if(board[i][j] == 'U' && isPlayerSpot(n, board, myColour, i, j, deltaRows, deltaCols)){ //if it is computer's spot
                    canMove = 1;
                    //put down the piece, update the board, and calculate the number of flips
                    numFlipped = flipPieceMiniMax(n, board,deltaRows, deltaCols, myColour, humanColour, i, j, firstMove+depth);
                    (*myNum) += (numFlipped+1); //modify the number of computer pieces
                    (*humanNum) -= numFlipped; //modify the number of human pieces
                    if(*humanNum == 0){//Checks for complete wipe-out, in which case the computer wins because there aren't any other human pieces left on the board
                        reverseBoardConfig(board, firstMove+depth, numFlipped, myNum, humanNum);
                        if(depth == 0){ //set the row and column for the move to i and j
                            *bestRow = i;
                            *bestCol = j;
                        }
                        return 10000;
                    }

                    //NEED TO PASS IN TIME
                    currMax = (int)fmax(currMax, miniMax(min, max, depth+1, 0, 0, bestRow, bestCol, myNum, humanNum, n, board, firstMove, spotValue, deltaRows, deltaCols, myColour, humanColour, minRow, minCol, maxRow, maxCol));
                    if(depth == 0 && currMax > max){ //update the best row and column for the next move
                        *bestRow = i;
                        *bestCol = j;
                        max = currMax;
                    }
                    else
                        max = (int)fmax(currMax, max); //update max for improved pruning in future call of the minimizer
                    reverseBoardConfig(board, firstMove+depth, numFlipped, myNum, humanNum); //change the board and numbers back

                    if(max >= min) //prune, if the current maximum found by maximizer is greater than the minimum on top
                        return currMax;
                }
            }
        }
        if(!canMove){//If computer cannot move
            if(depth == 0){
                *bestRow = -1;
                *bestCol = -1;
                return -10000;
            }
            if(!canPlayerMove(n, board, myColour, deltaRows, deltaCols, minRow, minCol, maxRow, maxCol)){ //If human cannot move either
                char winner = findWinner(*myNum, *humanNum);
                if(winner == 'H')
                    return -10000;
                else if(winner == 'M')
                    return 10000;
                else if(winner == 'D')
                    return 10;
            }
            else
                return miniMax(min, max, depth, 0, 0, bestRow, bestCol, myNum, humanNum, n, board, firstMove, spotValue, deltaRows, deltaCols, myColour, humanColour, minRow, minCol, maxRow, maxCol);
        }
        return currMax;
    }
    else{
        int currMin = 10000;
        for(int i = 0; i < 8; i++){
            for(int j = 0; j < 8; j++){
                if(board[i][j] == 'U' && isPlayerSpot(n, board, humanColour, i, j, deltaRows, deltaCols)){
                    canMove = 1;
                    numFlipped = flipPieceMiniMax(n, board,deltaRows, deltaCols, humanColour, myColour, i, j, firstMove+depth);
                    (*humanNum) += (numFlipped+1);
                    (*myNum) -= numFlipped;
                    if(*myNum == 0){
                        reverseBoardConfig(board, firstMove+depth, numFlipped, humanNum, myNum);
                        return -10000;
                    }
                    currMin = (int)fmin(currMin, miniMax(min, max, depth+1, 1, 0, bestRow, bestCol, myNum, humanNum, n, board, firstMove, spotValue, deltaRows, deltaCols, myColour, humanColour, minRow, minCol, maxRow, maxCol));
                    min = (int)fmin(currMin, min); //update min for improved pruning in future call of the maximizer
                    reverseBoardConfig(board, firstMove+depth, numFlipped, humanNum, myNum);
                    if(max >= min)
                        return currMin;
                }
            }
        }
        if(!canMove){
            if(!canPlayerMove(n, board, myColour, deltaRows, deltaCols, minRow, maxRow, minCol, maxCol)){
                char winner = findWinner(*myNum, *humanNum);
                if(winner == 'H')
                    return 10000;
                else if(winner == 'M')
                    return -10000;
                else if(winner == 'D')
                    return 10;
            }
            else
                return miniMax(min, max, depth, 1, 0, bestRow, bestCol, myNum, humanNum, n, board, firstMove, spotValue, deltaRows, deltaCols, myColour, humanColour, minRow, minCol, maxRow, maxCol);
        }
        return currMin;
    }
}

int flipPieceMiniMax(int n, char board[][26], short deltaRows[8], short deltaCols[8], char playerColour, char opponentColour, int row, int col, PastMoves* currentMove){
    int cnt;
    int numFlipped = 0;
    board[row][col] = playerColour;
    addNewPiece(row, col, currentMove);
    for(int i = 0; i < 8; i++){
        cnt = 1;
        while(positionInBounds(n, row+cnt*deltaRows[i], col+cnt*deltaCols[i])){
            if(board[row+cnt*deltaRows[i]][col+cnt*deltaCols[i]] == 'U')
                break;
            else if(board[row+cnt*deltaRows[i]][col+cnt*deltaCols[i]] == playerColour){
                if(cnt > 1){
                    for(int j = 1; j < cnt; j++){
                        board[row+j*deltaRows[i]][col+j*deltaCols[i]] = playerColour;
                        insertFlipped(row+j*deltaRows[i], col+j*deltaCols[i], opponentColour, &(currentMove->flips));
                        numFlipped++;
                    }
                }
                break;
            }
            else
                cnt++;
        }
    }
    return numFlipped;
}

void reverseBoardConfig(char board[][26], PastMoves *currentMove, int numFlipped, int *playerNum, int *opponentNum){
    board[currentMove->row][currentMove->col] = 'U';
    (*playerNum) -= (numFlipped+1);//Change the number of pieces for each player back to before this move was made
    (*opponentNum) += numFlipped;
    currentMove->row = currentMove->col = -1;
    flippedPiece* current;
    while((currentMove->flips).head != NULL){//Remove items from flippedList flips of currentMove
        board[(currentMove->flips).head->row][(currentMove->flips).head->col] = (currentMove->flips.head)->origColour;//Change the colours back
        current = ((currentMove->flips).head)->next;
        free((currentMove->flips).head);
        (currentMove->flips).head = current;
    }
}

bool myTurn(int n, char board[][26], int spotValue[][8], char myColour, char humanColour, PastMoves pastMoves[7], short deltaRows[8], short deltaCols[8], int* minRow, int* maxRow, int* minCol, int* maxCol, int* humanNum, int* myNum, char* winner){
    int bestRow = 0, bestCol = 0;
    miniMax(9999, -9999, 0, 1, 0, &bestRow, &bestCol, myNum, humanNum, n, board, &pastMoves[0], spotValue, deltaRows, deltaCols, myColour, humanColour, *minRow, *minCol, *maxRow, *maxCol);
    if(bestRow == -1)
        return 0;
    else{
        int numFlipped = flipPieces(n, board, deltaRows, deltaCols, myColour, bestRow, bestCol);
        printf("Computer places %c at %c%c.\n", myColour, 'a'+bestRow, 'a'+bestCol);
        printBoard(n, board);
        *humanNum -= numFlipped;
        if(*humanNum == 0)
            *winner = myColour;
        else{
            *myNum += (numFlipped+1);
            if(bestRow < *minRow && *minRow > 1){
                if (bestRow <= 1)
                    *minRow = 1;
                else
                    *minRow = bestRow;
            }
            if(bestRow > *maxRow && *maxRow < n-2){
                if (bestRow >= n - 2)
                    *maxRow = n - 2;
                else
                    *maxRow = bestRow;
            }
            if(bestCol < *minCol && *minCol > 1){
                if (bestCol <= 1)
                    *minCol = 1;
                else
                    *minCol = bestCol;
            }
            if(bestCol > *maxCol && *maxCol < n-2){
                if (bestRow >= n - 2)
                    *maxCol = n - 2;
                else
                    *maxCol = bestCol;
            }
        }
        return 1;
    }
}

bool isPlayerSpot(int n, char board[][26], char playerColour, int row, int col, short deltaRows[8], short deltaCols[8]){
    int cnt = 1;
    for(int i = 0; i < 8; i++){//Checks the 8 directions from the current position
        while(positionInBounds(n, row+cnt*deltaRows[i], col+cnt*deltaCols[i])){
            if (board[row + cnt * deltaRows[i]][col + cnt * deltaCols[i]] == 'U')
                break;
            else if (board[row + cnt * deltaRows[i]][col + cnt * deltaCols[i]] == playerColour)
                if (cnt == 1)
                    break;
                else
                    return 1;
            else
                cnt++;
        }
        cnt = 1;
    }
    return 0;
}

bool canPlayerMove(int n, char board[][26], char playerColour, short deltaRows[8], short deltaCols[8], int minRow, int maxRow, int minCol, int maxCol){
    for(int i = minRow-1; i <= maxRow+1; i++)
        for(int j = minCol-1; j <= maxCol+1; j++)
            if(board[i][j] == 'U' && isPlayerSpot(n, board, playerColour, i, j, deltaRows, deltaCols))
                return 1;
    return 0;
}

void getHumanMove(int n, char board[][26], short deltaRows[8], short deltaCols[8], char humanColour, char myColour, int *minRow, int *maxRow, int *minCol, int *maxCol, char *enter, int* humanNum, int* myNum, char *winner){
    int numFlipped;
    char enterRow, enterCol;
    int row, col;
    printf("Enter move for colour %c (RowCol): ", humanColour);
    scanf("%c%c%c", &enterRow, &enterCol, enter);
    row = enterRow - 'a';
    col = enterCol - 'a';
    //findSmartestMove(board, n, humanColour, &row, &col);
    //printf("Testing AI move (row, col): %c%c\n", row + 'a', col + 'a');
    if(!positionInBounds(n, row, col) || board[row][col] != 'U'){
        printf("Invalid move.\n");
        *winner = 'M';
    }
    else{
        numFlipped = flipPieces(n, board, deltaRows, deltaCols, humanColour, row, col);
        if(numFlipped == 0){
            printf("Invalid move.\n");
            *winner = 'M';
        }
        else{
            printBoard(n, board);
            *myNum -= numFlipped;
            if(*myNum == 0)
                *winner = humanColour;
            else{
                *humanNum += (numFlipped+1);
                if(row < *minRow && *minRow > 1) {
                    if (row <= 1)
                        *minRow = 1;
                    else
                        *minRow = row;
                }
                if(row > *maxRow && *maxRow < n-2){
                    if (row >= n - 2)
                        *maxRow = n - 2;
                    else
                        *maxRow = row;
                }
                if(col < *minCol && *minCol > 1){
                    if (col <= 1)
                        *minCol = 1;
                    else
                        *minCol = col;
                }
                if(col > *maxCol && *maxCol < n-2){
                    if (col >= n - 2)
                        *maxCol = n - 2;
                    else
                        *maxCol = col;
                }
            }
        }
    }
}
//flip the pieces on the board for an actual move, and return the number of pieces flipped
int flipPieces(int n, char board[][26], short deltaRows[8], short deltaCols[8], char colour, int row, int col){
    int cnt;
    int numFlipped = 0;
    board[row][col] = colour;
    for(int i = 0; i < 8; i++){
        cnt = 1;
        while(positionInBounds(n, row+cnt*deltaRows[i], col+cnt*deltaCols[i])){
            if(board[row+cnt*deltaRows[i]][col+cnt*deltaCols[i]] == 'U')
                break;
            else if(board[row+cnt*deltaRows[i]][col+cnt*deltaCols[i]] == colour){
                if(cnt > 1){
                    for(int j = 1; j < cnt; j++){
                        board[row+j*deltaRows[i]][col+j*deltaCols[i]] = colour;
                        numFlipped++;
                    }
                }
                break;
            }
            else
                cnt++;
        }
    }
    return numFlipped;
}

char findWinner(int myNum, int humanNum){
    if(myNum > humanNum)
        return 'M';
    else if(myNum < humanNum)
        return 'H';
    else
        return 'D';
}

int piecesICanEat(int n, char board[][26], char myColour, int row, int col, short deltaRows[8], short deltaCols[8]){
    int totalPieces = 0, cnt;
    for(int i = 0; i < 8; i++){
        cnt = 1;
        while(positionInBounds(n, row+cnt*deltaRows[i], col+cnt*deltaCols[i])){
            if(board[row+cnt*deltaRows[i]][col+cnt*deltaCols[i]] == 'U')
                break;
            else if(board[row+cnt*deltaRows[i]][col+cnt*deltaCols[i]] ==  myColour){
                if(cnt == 1)
                    break;
                else{
                    totalPieces += cnt-1;
                    break;
                }
            }
            else
                cnt++;
        }
    }
    return totalPieces;
}

bool myTurnDumb(int n, char board[][26], char humanColour, char myColour, short deltaRows[8], short deltaCols[8], int* minRow, int* maxRow, int* minCol, int* maxCol, int* humanNum, int* myNum, char* winner){
    int bestRow = 0, bestCol = 0, maxFlips = 0, curr;
    for(int i = *minRow-1; i <= *maxRow+1; i++){
        for(int j = *minCol-1; j <= *maxCol+1; j++){
            if(board[i][j] == 'U'){
                curr = piecesICanEat(n, board, myColour, i, j, deltaRows, deltaCols);
                if(curr > maxFlips){
                    bestRow = i;
                    bestCol = j;
                    maxFlips = curr;
                }
            }
        }
    }
    if(maxFlips == 0)
        return 0;
    else{
        flipPieces(n, board, deltaRows, deltaCols, myColour, bestRow, bestCol);
        printf("Computer places %c at %c%c.\n", myColour, 'a'+bestRow, 'a'+bestCol);
        printBoard(n, board);
        *humanNum -= maxFlips;
        if(*humanNum == 0)
            *winner = myColour;
        else{
            *myNum += (maxFlips+1);
            if(bestRow < *minRow && *minRow > 1){
                if(bestRow <= 1)
                    *minRow = 1;
                else
                    *minRow = bestRow;
            }
            if(bestRow > *maxRow && *maxRow < n-2){
                if(bestRow >= n-2)
                    *maxRow = n-2;
                else
                    *maxRow = bestRow;
            }
            if(bestCol < *minCol && *minCol > 1){
                if(bestCol <= 1)
                    *minCol = 1;
                else
                    *minCol = bestCol;
            }
            if(bestCol > *maxCol && *maxCol < n-2){
                if(bestRow >= n-2)
                    *maxCol = n-2;
                else
                    *maxCol = bestCol;
            }
        }
        return 1;
    }
}

void initializeBoard(int n, char board[][26]){
    for(int i = 0; i < n; i++)
        for(int j = 0; j < n; j++)
            board[i][j] = 'U';
    board[(n-1)/2][(n-1)/2] = board[n/2][n/2] = 'W';
    board[n/2][(n-1)/2] = board[(n-1)/2][n/2] = 'B';
}

int main(){
    int n, minRow, maxRow, minCol, maxCol, plays = 0, turn, myNum = 2, humanNum = 2;
    bool humanMovable = 0;
    char board[26][26] = {{'U','U','U','U','U','U','U','U'},
                          {'U','U','U','U','U','U','U','U'},
                          {'U','U','U','U','U','U','U','U'},
                          {'U','U','U','W','B','U','U','U'},
                          {'U','U','U','B','W','U','U','U'},
                          {'U','U','U','U','U','U','U','U'},
                          {'U','U','U','U','U','U','U','U'},
                          {'U','U','U','U','U','U','U','U'}};
    int spotValue[8][8] = {{1500, -100, 15, 5, 5, 15, -100, 1500},
                           {-100, -200, 1,  1, 1, 1,  -200, -100},
                           {15,   1,    10, 2, 2, 10, 1,    15},
                           {5,    1,    2,  1, 1, 2,  1,    5},
                           {5,    1,    2,  1, 1, 2,  1,    5},
                           {15,   1,    10, 2, 2, 10, 1,    15},
                           {-100, -200, 1,  1, 1, 1,  -200, -100},
                           {1500, -100, 15, 5, 5, 15, -100, 1500}};
    short deltaRows[] = {-1,-1,-1,0,1,1,1,0}, deltaCols[] = {-1,0,1,1,1,0,-1,-1};
    char myColour, humanColour, enter, winner = 'N';
    PastMoves pastMoves[7];
    for(int i = 0; i < 7; i++)
        pastMoves[i].flips.head = NULL;
    printf("Enter the board dimension: ");
    scanf("%d%c", &n, &enter);
    printf("Computer plays (B/W): ");
    scanf("%c%c", &myColour, &enter);
    minRow = minCol = (n-1)/2;
    maxRow = maxCol = minRow+1;
    if(myColour == 'B'){
        humanColour = 'W';
        turn = 1;
    }
    else{
        humanColour = 'B';
        turn = 0;
    }
    if(n == 8){
        printBoard(n, board);
        do{
            if (turn == 1){
                if(myTurn(n, board, spotValue, myColour, humanColour, pastMoves, deltaRows, deltaCols, &minRow, &maxRow, &minCol, &maxCol, &humanNum, &myNum, &winner))
                    plays++;
                else{
                    if(!canPlayerMove(n, board, humanColour, deltaRows, deltaCols, minRow, maxRow, minCol, maxCol))
                        winner = findWinner(myNum, humanNum);
                    else{
                        printf("%c player has no valid move.\n", myColour);
                        humanMovable = 1;
                    }
                }
                turn = 0;
            }
            else{
                if(!humanMovable && !canPlayerMove(n, board, humanColour, deltaRows, deltaCols, minRow, maxRow, minCol, maxCol)){
                    if(!canPlayerMove(n, board, myColour, deltaRows, deltaCols, minRow, maxRow, minCol, maxCol))
                        winner = findWinner(myNum, humanNum);
                    else
                        printf("%c player has no valid move.\n", humanColour);
                }
                else{
                    getHumanMove(n, board, deltaRows, deltaCols, humanColour, myColour, &minRow, &maxRow, &minCol, &maxCol, &enter, &humanNum, &myNum, &winner);
                    plays++;
                    humanMovable = 0;
                }
                turn = 1;
            }
        }while(winner == 'N' && plays < n*n - 4);
    }
    else{
        initializeBoard(n, board);
        printBoard(n, board);
        do{
            if (turn == 1){
                if(myTurnDumb(n, board, humanColour, myColour, deltaRows, deltaCols, &minRow, &maxRow, &minCol, &maxCol, &humanNum, &myNum, &winner))
                    plays++;
                else{
                    if(!canPlayerMove(n, board, humanColour, deltaRows, deltaCols, minRow, maxRow, minCol, maxCol))
                        winner = findWinner(myNum, humanNum);
                    else{
                        printf("%c player has no valid move.\n", myColour);
                        humanMovable = 1;
                    }
                }
                turn = 0;
            }
            else{
                if(!humanMovable && !canPlayerMove(n, board, humanColour, deltaRows, deltaCols, minRow, maxRow, minCol, maxCol)){
                    if(!canPlayerMove(n, board, myColour, deltaRows, deltaCols, minRow, maxRow, minCol, maxCol))
                        winner = findWinner(myNum, humanNum);
                    else
                        printf("%c player has no valid move.\n", humanColour);
                }
                else{
                    getHumanMove(n, board, deltaRows, deltaCols, humanColour, myColour, &minRow, &maxRow, &minCol, &maxCol, &enter, &humanNum, &myNum, &winner);
                    plays++;
                    humanMovable = 0;
                }
                turn = 1;
            }
        }while(winner == 'N' && plays < n*n - 4);
    }
    if(winner == 'N')
        winner = findWinner(myNum, humanNum);
    if(winner != 'D')
        if(winner == 'M')
            printf("%c player wins.\n", myColour);
        else
            printf("%c player wins.\n", humanColour);
    else
        printf("Draw!\n");
    return 0;
}