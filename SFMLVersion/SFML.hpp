#include "../ConsoleVersion/board.hpp"
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

enum class GameState { Menu, Playing, Defeat, Victory };
enum class CellState { Open, Closed, Flagged };


class Game {
private:
	float minesFreq = 0.156f;
	int totalMines;
	Board board;
	GameState state;
	int rows, cols;
	float finishTimeMs = 0.0;
	std::unique_ptr<CellAnim[]> AnimStateArr;
	std::unique_ptr<CellState[]> CellStateArr;
public:
	Game(int rows, int cols);
	void getGameResult();
	void updateBoard();
	void handleInput();
	Board& getBoard() { return board; }
	const Board& getBoard() const { return board; }
	const CellAnim* getAnimState() const { return AnimStateArr.get(); }
	const CellState* getCellState() const { return CellStateArr.get(); }
	GameState getGameState() const { return state; }
};

class UI {
private:
	Game& game;
	float cellScalePx;
	std::pair<float, float> gridStartPos;
	float boardWidth;
	float topBarHeight;
	sf::VertexArray cellsVA;
	sf::RenderWindow& window;
	sf::Clock dtClock;
	sf::Clock gameTimer;
	sf::Texture tilesTexture;
	sf::Font font;
	sf::Text& textTimer;
	sf::Text& textMines;
	sf::RectangleShape topBarRect;
public:
	UI(Game& game);
	void initLayout();
	void updateCell();
	void loadResources();
	void handleInput(float mouseX, float mouseY, sf::Mouse::Button button);
	void processEvents();
	void update();
	void render();
	void drawCell();
	void run();
};