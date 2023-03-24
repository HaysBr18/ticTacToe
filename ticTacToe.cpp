//Bryant R. Hays
//01/31/2023
//CS 470 Lab02
#include <iostream>
#include <cstdlib>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include<limits>
#include <stddef.h>
#include<ios>


using namespace std;


void fillBoard(char *board, int boardSize);

void printBoard(char *board, int boardSize, int n);

void playGame(char *board, int size, int i);

void makeMove(int choice, char symbol, char *string1);

bool isValidMove(char *board, int n,  int boardSize, int row, int column);

bool gameOver(char *board, char symbol, int size, int i);

bool isTie(char *board, int size);

int main(int argc, char *argv[]) {

    //Variable for board size (Where the size will be n x n).
    int n = 0;

    //If no arguments are provided, then break the program.
    if (argc < 2){
        cout << "Error: a board size must be provided!" << endl;
        exit(0);
    }

    //Get the size of the board from the command line.
    n = atoi(argv[1]);


    //Check for erroneous board size. The size can be 3x3, 5x5, or 7x7.
    if (n < 3 || n > 7 || n % 2 != 1){
        cout << "\nError: Board size n must be an odd number greater than or equal to 3 (for 3 x 3) and less than or equal to 7 (for 7 x 7)." << endl;
        exit(0);
    }

    //Variable for total board size.
    int boardSize = n * n;


    //Establish the shared memory instance.
     int shmId = shmget(IPC_PRIVATE, sizeof(int) * boardSize, IPC_CREAT | 0666);

     //Failed to create process.
     if (shmId < 0){

         printf("\nError: Failed to create shared memory segment.");

         return 1;
     }

     //Attach shared memory instance to an array for the board.
     char* board = (char*)shmat(shmId, nullptr, 0);

     //Fill the board with empty space.
    fillBoard(board, boardSize);

    //Print the initial board.
    printBoard(board, boardSize, n);

    //Begin gameplay.
    playGame(board, n, boardSize);


     return 0;
}

//Function to monitor and orchestrate gameplay amongst a parent and child process using the fork() function.
void playGame(char *board, int n, int boardSize) {

    char players[2] = {'X', 'O'};
    int choice = 0;
    int row = 0;
    int column = 0;
    bool play = true;


    while (play == true) {

        //Flush the output stream so no extra output prints to the screen.
        fflush(stdout);

        //Fork the process.
        int pid = fork();


        //Child process (player 1).
        if (pid == 0) {

            //Check if game is already over.
            if (gameOver(board, players[1], boardSize, n) == true){

                cout << "\nChild process loses." << endl;
                break;
            }

            //Check if game is already tied.
            else if (isTie(board, boardSize) == true){

                cout << "\nThe result is a draw!";
                break;
            }

            //Begin the child process's turn.
            while (true) {

                cout << "\nChild process's turn ";

                do {

                    cout << "\nPlease make a row selection: ";
                    cin >> row;

                    cout << "\nPlease make a column selection: ";
                    cin >> column;

                } while (!isValidMove(board, n,  boardSize, row, column)); //Prompt process until selection is valid.

                //Map the row and column selection to an array element.
                choice = (n*row) + column;

                //Submit the player's selection.
                makeMove(choice, players[0], board);
                cout << "\n";

                //Check if game is over after this turn.
                if (gameOver(board, players[0], boardSize, n) == true){

                    cout << "\nChild process wins!";
                }

                //Check if game is tied after this turn.
                else if (isTie(board, boardSize) == true) {

                    cout << "\nThe result is a draw!";
                }

                //Display the resulting board.
                printBoard(board, boardSize, n);
                exit(0);

            }


        }

        //Parent process (player 2).
        else if (pid > 0) {

            //Wait for the child process to finish its execution.
            wait(NULL);

            //Check if the game is already over.
            if (gameOver(board, players[0], boardSize, n) == true){

                cout << "\nParent process loses." << endl;
                break;
            }

            //Check if the game is already tied.
            else if (isTie(board, boardSize) == true){

                cout << "\nThe result is a draw!" << endl;
                break;
            }

            //Begin the parent process's turn.
            while (true) {

                cout << "\nParent process's turn ";

                do {

                    cout << "\nPlease make a row selection: ";
                    cin >> row;

                    cout << "\nPlease make a column selection: ";
                    cin >> column;

                } while (!isValidMove(board, n,  boardSize, row, column)); //Prompt process until selection is valid.

                //Map the row and column selection to an array element.
                choice = (n*row) + column;

                //Submit the player's selection.
                makeMove(choice, players[1], board);

                //Check if game is over after this turn.
                if (gameOver(board, players[1], boardSize, n) == true){
                    cout << "\nParent process wins!";
                }

                //Check if game is tied after this turn.
                else if (isTie(board, boardSize) == true) {

                    cout << "\nThe result is a draw!" << endl;
                }

                //Display the resulting board.
                printBoard(board, boardSize, n);
                break;
            }

        } else {
            cout << "\nError forking process!"; //Something went wrong with forking the process.
        }

        //Check if the game is over again and terminate (parent process win).
        if (gameOver(board, players[1], boardSize, n) == true){

            cout << "\nChild process loses"  << endl;
            break;
        }

        //Check if the game is over again and terminate (child process win).
        else if (gameOver(board, players[0], boardSize, n) == true){

            cout << "\nParent process loses" << endl;
            break;
        }
    }

}

//Check if the game results in a tie.
bool isTie(char *board, int boardSize) {

    for (int i = 0; i < boardSize; i++){

        if (board[i] != 'X' && board[i] != 'O'){

            return false;
        }

    }

    return true;
}

//Check if the game is over.
bool gameOver(char *board, char symbol, int boardSize, int n) {

    //Variable for checking the wins. If this variable is equal to n-1, then the current process wins the game.
    int winCheck = 0;

    //Check rows.
    for (int i = 0; i < n; i++){

        winCheck = 0;

        for (int j = 0; j < n; j++){

            if (board[i * n + j] == symbol){

                if (winCheck == (n-1)){
                    return true;
                }
                winCheck++;
            }
        }
    }


    //Check columns.
    for (int i = 0; i < n; i++){
        winCheck = 0;

        for (int j = 0; j < n; j++){

            if (board[j * n + i] == symbol){

                if (winCheck == (n - 1)){

                    return true;
                }

                winCheck++;
            }
        }
    }

    //Check diagonals.

    //Forward diagonal
    winCheck = 0;
    for (int i = 0; i < n; i++){

        if (board[i * n + (n - 1 - i)] == symbol){
            if (winCheck == (n - 1)){

                return true;
            }

            winCheck++;
        }
    }


    //Backward diagonal.
    winCheck = 0;
    for (int i = 0; i < n; i++){

            if (board[i * n + i] == symbol) {

                if (winCheck == (n - 1)){

                    return true;
                }

                winCheck++;

            }
        }

    return false;

}

//Function to check if the move is valid.
bool isValidMove(char *board, int n,  int boardSize, int row, int column) {

    //Invalid input assignment to integer value. clear the input buffer and ask user to make a new selection.
    if (cin.fail()){

        cout << "\nError: Selection must be a valid integer value!";
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return false;

    }

    //Check if the row and column size is valid.
    if (row >= n || row < 0 || column >= n || column < 0){

        cout << "\nError: row and column selection must be a valid integer (0 to n-1)!";
        return false;
    }

    //Map the row and column selection to an array element.
    int choice  = (n*row) + column;

    //If this spot is already taken, then prompt the user to make another selection.
    if (board[choice] == 'X' || board[choice] == 'O'){
        cout << "\nError: This spot is already taken! Please make another selection.";
        return false;
    }

    return true;
}

//Submit the player's selection.
void makeMove(int choice, char symbol, char *board) {

    board[choice] = symbol;
}



//Method to print the board.
void printBoard(char *board, int boardSize, int n) {

    cout << "\n";


    int i = 0;
    int j = 0;

    while (i < boardSize){

        while (j < n){

            cout <<  board[i]  << " ";
            i++;
            j++;
        }

        cout << "\n";
        j = 0;
    }

}

//Fill the empty board.
void fillBoard(char *board, int boardSize) {

    for (int i = 0; i < boardSize; i++){

        board[i] = '-';
    }
}