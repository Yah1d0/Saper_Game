#include "board.hpp"
#include <iostream>
#include <limits>
#include <cstdlib>
#include <string>
#include <iomanip>
#include <cctype>

// colors
const std::string RESET = "\033[0m";
const std::string GREY = "\033[90m";    
const std::string RED = "\033[91m";
const std::string D_RED = "\033[31m";
const std::string GREEN = "\033[92m";  
const std::string WHITE = "\033[97m";   
const std::string BLUE = "\033[94m";    
const std::string D_GREEN = "\033[32m";  
const std::string D_BLUE = "\033[34m";   
const std::string BROWN = "\033[33m";  
const std::string CYAN = "\033[36m";    
const std::string YELLOW = "\033[93m";
const std::string PURPLE = "\033[35m";
const std::string WHITE_ON_RED = "\033[97;41m";

    
std::string getColorForNum(int n)
{
    switch (n) {
        case 1: return BLUE;
        case 2: return D_GREEN;
        case 3: return RED;
        case 4: return D_BLUE;
        case 5: return BROWN;
        case 6: return CYAN;
        case 7: return YELLOW;
        case 8: return PURPLE;
        default: return RESET;
    }
}

void clearConsole() {
    #ifdef _WIN32
        std::system("cls");
    #else
        std::system("clear");
    #endif
}

void renderBoard(const Board& board) {
    int rows = board.getRows();
    int cols = board.getCols();
    std::pair<int, int> explodedMine = board.getExplodedMine();
    std::cout << "   ";
    for (int c = 0; c < cols; c++) {
        std::cout << std::setw(2) << char('A' + c) << " ";
    }
    std::cout << std::endl;
    std::cout << "   " << std::string(cols * 3, '=') << std::endl;
    for (int r = 0; r < rows; r++) {
        std::cout << std::setw(2) << (r + 1) << "|";
        for (int c = 0; c < cols; c++) {
            const Cell &cell = board.getCell(r, c);
            std::string context = " ";
            std::string color = RESET;
            if (cell.isFlagged) {
                if (cell.isRevealed && !cell.isMine) {
                    context = "X";
                    color = WHITE_ON_RED;
                }
                else {
                    context = "F";
                    color = GREEN;
                }
            }
            else if (!cell.isRevealed) {
                context = "#";
                color = GREY;
            }
            else if (cell.isMine) {
                context = "*";
                if (explodedMine.first == r && explodedMine.second == c) {
                    color = WHITE_ON_RED;
                }
                else {
                    color = D_RED;
                }
            }
            else {
                if (cell.neighborMines == 0) {
                    context = " ";
                    color = WHITE;
                }
                else {
                    context = std::to_string(cell.neighborMines);
                    color = getColorForNum(cell.neighborMines);
                }
            }
            std::cout << "[" << color << context << RESET << "]";
        }
        std::cout << "|" << std::endl;
    }
    std::cout << "   " << std::string(cols * 3, '=') << std::endl;
}

int main() {
    clearConsole();

    bool gamerunning = true;
    int rows, cols;

    while (true) {
        std::cout << "- Rows (5-50): ";
        if (std::cin >> rows && rows >= 5 && rows <= 50) {
            break;
        }
        std::cout << "Invalid range: " << rows << " is not in (5-50) range." << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    while (true) {
        std::cout << "- Columns (5-26): ";
        if (std::cin >> cols && cols >= 5 && cols <= 26) {
            break;
        }
        std::cout << "Invalid range: " << cols << " is not in (5-26) range." << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    int mines = (rows * cols) / 6;
    Board board(rows, cols, mines);
    
    while (gamerunning) {
        clearConsole();
        renderBoard(board);

        if (board.isGameOver()) {
            gamerunning = false;
            clearConsole();
            std::cout << std::endl << RED << "You hit a mine! Game over." << RESET<< std::endl;
            continue;
        }

        if (board.isGameWon()) {
            gamerunning = false;
            clearConsole();
            std::cout << GREEN << std::endl << "Congratulations! You've won!" << RESET << std::endl;
            continue;
        }

        std::string input;
        std::cout << "Enter move ('A1' or 'B2 F'...): ";
        std::getline(std::cin, input);

        if (input.empty()) {
            continue;
        }

        char colChar = 0;
        int rowStart = -1;
        int rowEnd = -1;

        for (size_t i = 0; i < input.length(); i++) {
            if (std::isalpha(input[i])) {
                colChar = std::toupper(input[i]);
                break;
            }
        }

        for (size_t i = 0; i < input.length(); i++) {
            if (std::isdigit(input[i])) {
                rowStart = i;
                size_t j = i;
                while (j < input.length() && std::isdigit(input[j])) {
                    j++;
                }
                rowEnd = j;
                break;
            }
        }

        if (colChar == 0 || rowStart == -1 || rowEnd == -1) {
            std::cout << "Invalid input format." << std::endl;
            std::cout << "Press Enter...";
            std::cin.get();
            continue;
        }

        int r = std::stoi(input.substr(rowStart, rowEnd - rowStart));
        r--;

        int c = colChar - 'A';

        bool flagAction = false;
        std::string actionPart = input.substr(rowEnd);
        for (char c : actionPart) {
            if (std::toupper(c) == 'F') {
                flagAction = true;
                break;
            }
        }

        if (!board.isValidMove(r, c)) {
                std::cout << "Incorrect coordinates (Row: " << r + 1 << ", Col: " << char(c + 'A') << ")" << std::endl;
                std::cout << "Press Enter...";
                std::cin.get();
                std::cout << std::endl;
                continue;
        }

        if (flagAction) {
            board.Flag(r, c);
        }
        else {
            if (board.canBeChorded(r, c)) {
                if (!board.openCell(r, c)) {
                    board.Chord(r, c);
                }
            }
            else {
                board.openCell(r, c);
            }
        }
    }

    std::cout << "--- Final field ---" << std::endl;
    board.revealAll();
    renderBoard(board);
    std::cout << "The game is over." << std::endl;
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    return 0;
}