#pragma once 
#include <vector>
#include <array>
#include <utility>
#include <optional>

struct Cell {
	bool isFlagged{ false };
	bool isMine{ false };
	bool isRevealed{ false };
	int neighborMines{ 0 };
};

class Board {
private:
	int rows, cols, mines;
	int revealedCount = 0;
	bool isFirstMove{ true };
	std::optional<std::pair<int, int>> explodedMine;
	std::vector<Cell> grid;
	inline int getIndex(int row, int col) const {
		return row * cols + col;
	}
	static constexpr std::array<std::pair<int, int>, 8> directions = { {
			{-1, -1}, {-1, 0}, {-1, 1},
			{0, -1},           {0, 1},
			{1, -1},  {1, 0},  {1, 1}
	} };
	bool canBeChorded(int row, int col) const;

public:
	Board(int rows, int cols, int mines);
	std::vector<std::pair<int, int>> getCandidates(int safeRow, int safeCol) const;
	void placeMines(int safeRow, int safeCol);
	const Cell& getCell(int row, int col) const;
	Cell& getCell(int row, int col);
	int  getRows() const;
	int  getCols() const;
	std::optional<std::pair<int, int>> getExplodedMine() const;
	int  countNeighborFlags(int row, int col) const;
	bool isValidMove(int row, int col) const;
	bool openCell(int row, int col);
	bool Flag(int row, int col);
	void Chord(int row, int col);
	void revealAll();
	bool isRevealed(int row, int col) const;
	bool isGameOver() const;
	bool isGameWon() const;
};