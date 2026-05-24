#pragma once
#include "../ConsoleVersion/board.hpp"
#include <SFML/Graphics.hpp>
#include <variant>

class Game;

enum class MouseButton { LCM, RCM };

struct CellAnim {
	float openProgress = 0.0f;
	float flagProgress = 0.0f;
	bool removeFlag = false;
};

struct ClickHandler {
	Game& game;
	int row;
	int col;
	ClickHandler(Game& g, int r, int c);
	void operator()(MouseButton& button);
};

enum SpriteType {
	None,
	Closed,
	Flagged,
	WrongFlagged,
	Mine,
	Exploded,
	Empty, One, Two, Three, Four, Five, Six, Seven, Eight,
	OpenAnim_0, OpenAnim_1, OpenAnim_2, OpenAnim_3,
	FlagAnim_0, FlagAnim_1, FlagAnim_2, FlagAnim_3
};

enum class GameState { Menu, Playing, Defeat, Victory };
enum class CellState { Closed, Open, Flagged };


class Game {
private:
	float minesFreq = 0.156f;
	int totalMines;
	Board board;
	GameState state;
	float finishTimeMs = 0.0;
	std::unique_ptr<CellAnim[]> AnimStateArr;
	std::unique_ptr<CellState[]> CellStateArr;
public:
	Game(int rows, int cols);
	void updateBoard();
	void startPlaying();
	Board& getBoard() { return board; }
	const Board& getBoard() const { return board; }
	CellAnim* getAnimState() { return AnimStateArr.get(); }
	const CellAnim* getAnimState() const { return AnimStateArr.get(); }
	CellState* getCellState() { return CellStateArr.get(); }
	const CellState* getCellState() const { return CellStateArr.get(); }
	GameState getGameState() const { return state; }
};

class UI {
private:
	float finalTime = 0.0f;
	Game& game;
	float cellScalePx;
	std::pair<float, float> gridStartPos;
	float boardWidth;
	float topBarHeight;
	sf::VertexArray backgroundVA;
	sf::VertexArray	overlayVA;
	sf::RenderWindow window;
	sf::Clock dtClock;
	sf::Clock gameTimer;
	sf::Texture tilesTexture;
	sf::Font font;
	sf::Text textTimer;
	sf::Text textMines;
	sf::RectangleShape topBarRect;
public:
	UI(Game& game);
	void initLayout();
	void updateCell(int row, int col, const Cell& cell, const CellAnim& anim, std::pair<int, int> explodedMine);
	SpriteType getBackgroundType(int row, int col, std::pair<int, int> explodedMine);
	SpriteType getOverlayType(int row, int col, const CellAnim& anim);
	bool loadResources();
	void handleInput(float mouseX, float mouseY, sf::Mouse::Button button);
	void processEvents();
	void update();
	void render();
	void run();
};