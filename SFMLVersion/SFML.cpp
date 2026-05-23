#include "../ConsoleVersion/board.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <optional>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include "sfml.hpp"

const int tileSize = 32;
const int baseTopBarHeight = 60;
const int minMargin = 100;
const float openSpeed = 4.0f;
const float flagSpeed = 5.0f;

sf::FloatRect getTextureRect(SpriteType type, bool isDark) {
	int x = 0;
	int y = 0;
	switch (type) {
	case Closed: x = 0; y = 0; break;
	case Flagged: x = 5; y = 0; break;
	case WrongFlagged: x = 6; y = 0; break;
	case Mine: x = 1; y = 2; break;
	case Exploded: x = 1; y = 3; break;
	case Empty: x = 1; y = 1; break;
	case One: x = 2; y = 1; break;
	case Two: x = 2; y = 2; break;
	case Three: x = 2; y = 3; break;
	case Four: x = 2; y = 4; break;
	case Five: x = 3; y = 1; break;
	case Six: x = 3; y = 2; break;
	case Seven: x = 3; y = 3; break;
	case Eight: x = 3; y = 4; break;
	case OpenAnim_0: x = 0; y = 1; break;
	case OpenAnim_1: x = 0; y = 2; break;
	case OpenAnim_2: x = 0; y = 3; break;
	case OpenAnim_3: x = 0; y = 4; break;
	case FlagAnim_0: x = 1; y = 0; break;
	case FlagAnim_1: x = 2; y = 0; break;
	case FlagAnim_2: x = 3; y = 0; break;
	case FlagAnim_3: x = 4; y = 0; break;
	}
	if (isDark && (type == Mine || type == Empty || (type >= One && type <= Eight))) {
		x += 3;
	}
	return sf::FloatRect({ x * tileSize, y * tileSize }, { tileSize, tileSize });
}

SpriteType getOpenFrame(float progress) {
	if (progress < 0.25f) return OpenAnim_0;
	else if (progress < 0.5f) return OpenAnim_1;
	else if (progress < 0.75f) return OpenAnim_2;
	else return OpenAnim_3;
}

SpriteType getFlagFrame(float progress) {
	if (progress < 0.25f) return FlagAnim_0;
	else if (progress < 0.5f) return FlagAnim_1;
	else if (progress < 0.75f) return FlagAnim_2;
	else return FlagAnim_3;
}

void Lerp(float& progress, float dt, float speed) {
	if (progress > 0.0f && progress < 1.0f) {
		progress += dt * speed;
		if (progress > 1.0f)
			progress = 1.0f;
	}
}

ClickHandler::ClickHandler(Game& g, int r, int c) : game(g), row(r), col(c) {}

void ClickHandler::operator()(MouseButton& button) {
	Board& board = game.getBoard();
	if (button == MouseButton::LCM) {
		bool hitMine = board.openCell(row, col);
		if (!hitMine) {
			board.Chord(row, col);
		}
	}
	else if (button == MouseButton::RCM) {
		board.Flag(row, col);
	}
	game.updateBoard();
}

UI::UI(Game& game):
	game(game),
	window(sf::VideoMode({ 800, 600 }), "Saper", sf::Style::Default),
	textTimer(font),
	textMines(font) 
{
	window.setFramerateLimit(60);
	loadResources();
	initLayout();
}

void UI::initLayout() {
	Board& board = game.getBoard();
	int rows = board.getRows();
	int cols = board.getCols();
	cellsVA.setPrimitiveType(sf::PrimitiveType::Triangles);
	cellsVA.resize(rows * cols * 6);
	for (int r = 0; r < rows; ++r) {
		for (int c = 0; c < cols; ++c) {
			sf::Vertex* cellVertex = &cellsVA[(r * cols + c) * 6];
			float left = c * tileSize;
			float right = (c + 1) * tileSize;
			float top = r * tileSize;
			float bottom = (r + 1) * tileSize;
			cellVertex[0].position = { left, top };
			cellVertex[1].position = { right, top };
			cellVertex[2].position = { left, bottom };
			cellVertex[3].position = { right, top };
			cellVertex[4].position = { right, bottom };
			cellVertex[5].position = { left, bottom };

			bool isDark = (r + c) & 1;
			sf::FloatRect textureRect = getTextureRect(SpriteType::Closed, isDark);
			float txRectLeft = textureRect.position.x;
			float txRectRight = txRectLeft + textureRect.size.x;
			float txRectTop = textureRect.position.y;
			float txRectBottom = txRectTop + textureRect.size.y;
			cellVertex[0].texCoords = { txRectLeft, txRectTop };
			cellVertex[1].texCoords = { txRectRight, txRectTop };
			cellVertex[2].texCoords = { txRectLeft, txRectBottom };
			cellVertex[3].texCoords = { txRectRight, txRectTop };
			cellVertex[4].texCoords = { txRectRight, txRectBottom };
			cellVertex[5].texCoords = { txRectLeft, txRectBottom };
		}
	}
}

void UI::updateCell(int row, int col, const Cell& cell, const CellAnim& anim, std::pair<int, int> explodedMine) {
	int cols = game.getCols();
	int idx = (row * cols + col) * 6;
	bool isDark = (row + col) & 1;
	sf::FloatRect textureRect = getTextureRect(getCellType(row, col, anim, explodedMine), isDark);
	float txRectLeft = textureRect.position.x;
	float txRectRight = txRectLeft + textureRect.size.x;
	float txRectTop = textureRect.position.y;
	float txRectBottom = txRectTop + textureRect.size.y;
	cellsVA[idx + 0].texCoords = { txRectLeft, txRectTop };
	cellsVA[idx + 1].texCoords = { txRectRight, txRectTop };
	cellsVA[idx + 2].texCoords = { txRectLeft, txRectBottom };
	cellsVA[idx + 3].texCoords = { txRectRight, txRectTop };
	cellsVA[idx + 4].texCoords = { txRectRight, txRectBottom };
	cellsVA[idx + 5].texCoords = { txRectLeft, txRectBottom };
}

SpriteType UI::getCellType(int row, int col, const CellAnim& anim, std::pair<int, int> explodedMine) {
	Cell& cell = game.getBoard().getCell(row, col);
	if (!cell.isRevealed) {
		if (anim.flagProgress > 0.0f && anim.flagProgress < 1.0f) {
			float progress = anim.flagProgress;
			if (anim.flagProgress && progress > 0.0f) progress = 1.0f - progress;
			return getFlagFrame(progress);
		}
		if (cell.isFlagged) {
			return Flagged;
		}
		return Closed;
	}
	else {
		if (anim.openProgress > 0.0f && anim.openProgress < 1.0f) {
			return getOpenFrame(anim.openProgress);
		}
		if (cell.isMine) {
			return (explodedMine == std::make_pair(row, col)) ? Exploded : Mine;
		}
		else {
			return static_cast<SpriteType>(Empty + cell.neighborMines);
		}
	}
}

bool UI::loadResources() {
	if (!tilesTexture.loadFromFile("tiles.png") || !font.openFromFile("font.ttf")) {
		return false;
	}
	tilesTexture.setSmooth(false);
	textTimer.setFont(font);
	textMines.setFont(font);
	textTimer.setFillColor(sf::Color::White);
	textTimer.setStyle(sf::Text::Bold);
	textMines.setFillColor(sf::Color::White);
	textMines.setStyle(sf::Text::Bold);
	return true;
}

void UI::handleInput(float mouseX, float mouseY, sf::Mouse::Button button) {
	Board& board = game.getBoard();
	float localX = mouseX - gridStartPos.first;
	float localY = mouseY - gridStartPos.second;
	int pressedCol = static_cast<int>(localX / cellScalePx);
	int pressedRow = static_cast<int>(localY / cellScalePx);
	if (board.isValidMove(pressedRow, pressedCol)) {
		ClickHandler click(this->game, pressedRow, pressedCol);
		MouseButton btn = (button == sf::Mouse::Button::Left) ? MouseButton::LCM : MouseButton::RCM;
		click(btn);
	}
}

void UI::processEvents() {
	while (const std::optional event = window.pollEvent()) {
		if (event->is<sf::Event::Closed>()) {
			window.close();
		}
		else if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
			handleInput(static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y), mouseEvent->button);
		}
	}
}

void UI::update() {
	float dt = dtClock.restart().asSeconds();
	Board& board = game.getBoard();
	int rows = board.getRows();
	int cols = board.getCols();
	CellAnim* animState = game.getAnimState();
	CellState* cellState = game.getCellState();
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			int idx = i * cols + j;
			const Cell& cell = board.getCell(i, j);
			CellAnim& anim = animState[idx];
			Lerp(anim.openProgress, dt, openSpeed);
			Lerp(anim.flagProgress, dt, flagSpeed);
			updateCell(i, j, cell, anim, board.getExplodedMine());
		}
	}
	int flagsPlaced = 0;
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (board.getCell(i, j).isFlagged) flagsPlaced++;
		}
	}
	int totalMines = rows * cols * 0.156f;
	textMines.setString("Mines left: " + std::to_string(totalMines - flagsPlaced));
	bool isGameWon = board.isGameWon();
	bool isGameOver = board.isGameOver();
	float currentTime = gameTimer.getElapsedTime().asSeconds();
	if (isGameOver || isGameWon) {
		currentTime = 0.0f;
	}

	std::stringstream ss;
	ss << std::fixed << std::setprecision(1) << currentTime;
	textTimer.setString(ss.str());
	float textY = (topBarHeight - textMines.getGlobalBounds().size.y) / 2.0f - (5 * cellScalePx);
	textMines.setPosition({ gridStartPos.first, textY });
	textTimer.setPosition({ gridStartPos.first + boardWidth - textTimer.getGlobalBounds().size.x, textY });
}

void UI::render() {
	window.clear(sf::Color({ 111, 111, 131 }));
	window.draw(topBarRect);
	window.draw(textMines);
	window.draw(textTimer);
	window.draw(cellsVA, &tilesTexture);
	window.display();
}

void UI::run() {
	while (window.isOpen()) {
		processEvents();
		update();
		render();
	}
}

Game::Game(int rows, int cols)
	: totalMines(static_cast<int>(rows* cols * 0.156)),
	board(rows, cols, totalMines) {
	this->AnimStateArr = std::make_unique<CellAnim[]>(rows * cols);
	this->CellStateArr = std::make_unique<CellState[]>(rows * cols);
	this->state = GameState::Menu;
}

int main() {
	int rows = 10;
	int cols = 10;
	float minesPercent = 0.21f;
	if (rows < 5) rows = 5;
	if (cols < 5) cols = 5;

	sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Saper", sf::Style::Default);
	
	sf::Texture tempTexture;
	sf::Sprite sprite(tempTexture);
	
	sf::Font tempFont;
	sf::Text textTimer(tempFont);
	sf::Text textMines(tempFont);

	Game game(rows, cols);
	UI ui(game);
	ui.run();
	return 0;
}