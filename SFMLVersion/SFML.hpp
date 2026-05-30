#pragma once
#include "../ConsoleVersion/board.hpp"
#include <SFML/Graphics.hpp>
#include <optional>

class Game;

enum class MouseButton { LCM, RCM };

struct CellAnim {
	float openProgress = 0.0f;
	float flagProgress = 0.0f;
	bool removeFlag = false;
};

struct ClickHandler final {
	Game& game;
	int row;
	int col;
	explicit ClickHandler(Game& g, int r, int c);
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


class Game final {
private:
	int totalMines;
	Board board;
	GameState state;
public:
	explicit Game(int rows, int cols);
	void updateBoard();
	void startPlaying();
	void restart(int rows, int cols);
	Board& getBoard() { return board; }
	const Board& getBoard() const { return board; }
	GameState getGameState() const { return state; }
};

class UI final {
private:
	Game& game;
	float cellScalePx;
	std::pair<float, float> gridStartPos;
	float boardWidth;
	float topBarHeight;
	float finalTime = 0.0f;
	GameState lastState;
	std::unique_ptr<CellAnim[]> AnimStateArr;
	std::unique_ptr<CellState[]> CellStateArr;
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
	sf::RectangleShape overlayBackground;
	sf::Text overlayMainText;
	sf::RectangleShape restartButton;
	sf::Text restartButtonText;
	void calculateLayoutMetrics();
	void setupVertexArrays();
	void updateOverlayLayout();
	void updateCellAnimations(float dt);
	void updateGameStats();
	float getGameTime() const;
	void handleBoardClick(const sf::Vector2f& worldPos, sf::Mouse::Button button);
	void handleOverlayClick(const sf::Vector2f& worldPos, sf::Mouse::Button button);
public:
	explicit UI(Game& game);
	void initLayout();
	void updateCell(int row, int col, const Cell& cell, const CellAnim& anim, std::optional<std::pair<int, int>> explodedMine);
	SpriteType getBackgroundType(int row, int col, std::optional<std::pair<int, int>> explodedMine);
	SpriteType getOverlayType(int row, int col, const CellAnim& anim);
	bool loadResources();
	void handleInput(float mouseX, float mouseY, sf::Mouse::Button button);
	void processEvents();
	void update();
	void render();
	void run();
	CellAnim* getAnimState() { return AnimStateArr.get(); }
	const CellAnim* getAnimState() const { return AnimStateArr.get(); }
	CellState* getCellState() { return CellStateArr.get(); }
	const CellState* getCellState() const { return CellStateArr.get(); }
};