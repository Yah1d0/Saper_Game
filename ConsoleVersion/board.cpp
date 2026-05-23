#include "board.hpp"
#include <queue>
#include <random>
#include <algorithm>
#include <iostream>
#include <string>
#include <ctime>

namespace {
    std::random_device rd;
    std::mt19937 gen(rd());
}

Board::Board(int rows, int cols, int mines) : rows(rows), cols(cols), mines(mines) {
    int maxMines = this->rows * this->cols - 9;
    this->mines = std::max(1, std::min(this->mines, maxMines));
    grid.resize(rows * cols);
}

int Board::getRows() const {
    return rows;
}

int Board::getCols() const {
    return cols;
}

std::pair<int, int> Board::getExplodedMine() const {
    return explodedMine;
}

const Cell &Board::getCell(int row, int col) const {
    return grid[getIndex(row, col)];
}

Cell& Board::getCell(int row, int col)
{
    return grid[getIndex(row, col)];
}

void Board::placeMines(int safeRow, int safeCol) {
    auto candidates = getCandidates(safeRow, safeCol);
    std::shuffle(candidates.begin(), candidates.end(), gen);
    int toPlace = std::min(mines, static_cast<int>(candidates.size()));
    for (int i = 0; i < toPlace; i++) {
        int r = candidates[i].first;
        int c = candidates[i].second;
        int index = getIndex(r, c);
        Cell &cell = grid[index];
        cell.isMine = true;
        for (const auto &dir : directions){
            int nr = r + dir.first;
            int nc = c + dir.second;
            if (isValidMove(nr, nc)) {
                int index = getIndex(nr, nc);
                grid[index].neighborMines++;
            }
        }
    }
}

std::vector<std::pair<int, int>> Board::getCandidates(int safeRow, int safeCol) const {
    std::vector<std::pair<int, int>> candidates;
    candidates.reserve(rows * cols - 9);
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (std::abs(r - safeRow) <= 1 && std::abs(c - safeCol) <= 1) {
                continue;
            }
            candidates.emplace_back(r, c);
        }
    }
    return candidates;
}

int Board::countNeighborFlags(int row, int col) const {
    int count = 0;
    for (const auto &dir : directions) {
        int r = row + dir.first;
        int c = col + dir.second;
        int index = getIndex(r, c);
        if (this->isValidMove(r, c) && this->grid[index].isFlagged) {
            count++;
        }
    }
    return count;
}

bool Board::isValidMove(int row, int col) const {
    return row >= 0 && row < rows && col >= 0 && col < cols;
}

bool Board::openCell(int row, int col) {
    if (!isValidMove(row, col)) return false;
    int index = getIndex(row, col);
    Cell &cell = grid[index];
    if (isFirstMove){
        isFirstMove = false;
        placeMines(row, col);
    }
    if (cell.isRevealed || cell.isFlagged) return false;
    if (cell.isMine) {
        cell.isRevealed = true;
        explodedMine = {row, col};
        return true;
    }
    std::queue<std::pair<int, int>> q;
    q.push({row, col});
    cell.isRevealed = true;
    while (!q.empty()) {
        std::pair<int, int> current = q.front();
        q.pop();
        int r = current.first;
        int c = current.second;
        int currentIndex = getIndex(r, c);
        Cell &currentCell = grid[currentIndex];
        if (currentCell.neighborMines > 0) continue;
        for (const auto &dir : directions) {
            int nr = r + dir.first;
            int nc = c + dir.second;
            if (isValidMove(nr, nc)) {
                int neighborIndex = getIndex(nr, nc);
                Cell &neighbor = grid[neighborIndex];
                if (!neighbor.isRevealed && !neighbor.isFlagged && !neighbor.isMine) {
                    neighbor.isRevealed = true;
                    q.push({nr, nc});
                }
            }
        }
    }
    return false;
}

bool Board::Flag(int row, int col) {
    if (!isValidMove(row, col)) {
        return false;
    }
    int index = getIndex(row, col);
    Cell &cell = grid[index];
    if (cell.isRevealed) {
        return false;
    }
    cell.isFlagged = !cell.isFlagged;
    return true;
}

bool Board::canBeChorded(int row, int col) const {
    int index = getIndex(row, col);
    const Cell& cell = grid[index];
    int flaggedNeighbors = countNeighborFlags(row, col);
    return flaggedNeighbors == cell.neighborMines;
}

void Board::Chord(int row, int col) {
    if (!this->canBeChorded(row, col)) return;
    for (const auto &dir : directions){
        int r = row + dir.first;
        int c = col + dir.second;
        if (isValidMove(r, c)){
            openCell(r, c);
        }
    }
}

void Board::revealAll() {
    for (auto& cell : grid) {
        cell.isRevealed = true;
        if (cell.isMine) {
            cell.isFlagged = false;
        }
    }
}

bool Board::isRevealed(int row, int col) const {
    int index = getIndex(row, col);
    return isValidMove(row, col) && grid[index].isRevealed;
}

bool Board::isGameOver() const {
    for (const auto& cell : grid) {
        if (cell.isMine && cell.isRevealed) {
            return true;
        }
    }
    return false;
}

bool Board::isGameWon() const {
    for (const auto& cell : grid) {
        if (!cell.isMine && !cell.isRevealed) {
            return false;
        }
    }
    return true;
}