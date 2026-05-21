#include "../ConsoleVersion/board.hpp"
#include <variant>

enum class MouseButton { LCM, RCM };

struct CellAnim {
	float openProgress = 0.0f;
	float flagProgress = 0.0f;
	bool removeFlag = false;
};

struct ClickHandler {
	Board& board;
	int row;
	int col;
	ClickHandler(Board& b, int r, int c);
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
	const CellAnim* getAnimState() const;
};

class UI {
private:
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