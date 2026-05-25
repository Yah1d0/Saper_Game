#include "../ConsoleVersion/board.hpp"
#include "sfml.hpp"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <optional>
#include <SFML/Graphics.hpp>
#include <sstream>
#include <vector>

constexpr int tileSize = 32;
constexpr int baseTopBarHeight = 60;
constexpr int minMargin = 100;
constexpr float openSpeed = 4.0f;
constexpr float flagSpeed = 5.0f;
constexpr float default_mine_density = 0.156f;

sf::FloatRect getTextureRect(SpriteType type, bool isDark) {
	int x = 0;
	int y = 0;
	switch (type) {
		case None: x = 1; y = 4; break;
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
	return sf::FloatRect({ static_cast<float>(x * tileSize), static_cast<float>(y * tileSize) }, { static_cast<float>(tileSize), static_cast<float>(tileSize) });
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
	} else if (button == MouseButton::RCM) {
		board.Flag(row, col);
	}
	game.updateBoard();
}

UI::UI(Game& game) :
	game(game),
	window(sf::VideoMode::getDesktopMode(), "Saper game", sf::Style::Default),
	textTimer(font),
	textMines(font),
	AnimStateArr(std::make_unique<CellAnim[]>(game.getBoard().getRows()* game.getBoard().getCols())),
	CellStateArr(std::make_unique<CellState[]>(game.getBoard().getRows()* game.getBoard().getCols()))
{
	window.setFramerateLimit(60);
	loadResources();
	initLayout();
}

void UI::initLayout() {
	Board& board = game.getBoard();

	int rows = board.getRows();
	int cols = board.getCols();

	std::fill(AnimStateArr.get(), AnimStateArr.get() + rows * cols, CellAnim{});
	std::fill(CellStateArr.get(), CellStateArr.get() + rows * cols, CellState::Closed);

	sf::Vector2u winSize = window.getSize();

	float screenWidth = static_cast<float>(winSize.x);
	float screenHeight = static_cast<float>(winSize.y);

	float contentWidth = cols * tileSize;
	float contentHeight = (rows * tileSize) + baseTopBarHeight;

	float availableWidth = screenWidth - (minMargin * 2);
	float availableHeight = screenHeight - (minMargin * 2);

	float scaleX = availableWidth / contentWidth;
	float scaleY = availableHeight / contentHeight;

	cellScalePx = std::min(scaleX, scaleY);
	boardWidth = contentWidth * cellScalePx;

	topBarHeight = baseTopBarHeight * cellScalePx;
	float finalTotalHeight = topBarHeight + ((rows * tileSize) * cellScalePx);

	float offsetX = (screenWidth - boardWidth) / 2.0f;
	float startY = (screenHeight - finalTotalHeight) / 2.0f;

	gridStartPos.first = offsetX;
	gridStartPos.second = startY + topBarHeight;

	topBarRect.setSize({ screenWidth, topBarHeight });
	topBarRect.setPosition({ 0.0f, 0.0f });
	topBarRect.setFillColor(sf::Color({ 46, 46, 54 }));

	unsigned int fontSize = static_cast<unsigned int>(14 * cellScalePx);

	textTimer.setCharacterSize(fontSize);
	textMines.setCharacterSize(fontSize);

	backgroundVA.setPrimitiveType(sf::PrimitiveType::Triangles);
	overlayVA.setPrimitiveType(sf::PrimitiveType::Triangles);

	backgroundVA.resize(rows * cols * 6);
	overlayVA.resize(rows * cols * 6);

	for (int r = 0; r < rows; ++r) {
		for (int c = 0; c < cols; ++c) {
			sf::Vertex* backgroundVertex = &backgroundVA[(r * cols + c) * 6];
			sf::Vertex* overlayVertex = &overlayVA[(r * cols + c) * 6];

			float left = gridStartPos.first + (c * tileSize * cellScalePx);
			float right = gridStartPos.first + ((c + 1) * tileSize * cellScalePx);
			float top = gridStartPos.second + (r * tileSize * cellScalePx);
			float bottom = gridStartPos.second + ((r + 1) * tileSize * cellScalePx);

			backgroundVertex[0].position = { left, top };
			backgroundVertex[1].position = { right, top };
			backgroundVertex[2].position = { left, bottom };
			backgroundVertex[3].position = { right, top };
			backgroundVertex[4].position = { right, bottom };
			backgroundVertex[5].position = { left, bottom };

			overlayVertex[0].position = { left, top };
			overlayVertex[1].position = { right, top };
			overlayVertex[2].position = { left, bottom };
			overlayVertex[3].position = { right, top };
			overlayVertex[4].position = { right, bottom };
			overlayVertex[5].position = { left, bottom };

			bool isDark = (r + c) & 1;

			sf::FloatRect textureRect = getTextureRect(SpriteType::Closed, isDark);

			float txRectLeft = textureRect.position.x;
			float txRectRight = txRectLeft + textureRect.size.x;
			float txRectTop = textureRect.position.y;
			float txRectBottom = txRectTop + textureRect.size.y;

			backgroundVertex[0].texCoords = { txRectLeft, txRectTop };
			backgroundVertex[1].texCoords = { txRectRight, txRectTop };
			backgroundVertex[2].texCoords = { txRectLeft, txRectBottom };
			backgroundVertex[3].texCoords = { txRectRight, txRectTop };
			backgroundVertex[4].texCoords = { txRectRight, txRectBottom };
			backgroundVertex[5].texCoords = { txRectLeft, txRectBottom };

			overlayVertex[0].texCoords = { txRectLeft, txRectTop };
			overlayVertex[1].texCoords = { txRectRight, txRectTop };
			overlayVertex[2].texCoords = { txRectLeft, txRectBottom };
			overlayVertex[3].texCoords = { txRectRight, txRectTop };
			overlayVertex[4].texCoords = { txRectRight, txRectBottom };
			overlayVertex[5].texCoords = { txRectLeft, txRectBottom };
		}
	}
}

void UI::updateCell(int row, int col, const Cell& cell, const CellAnim& anim, std::pair<int, int> explodedMine) {
	int cols = game.getBoard().getCols();

	int idx = (row * cols + col) * 6;

	bool isDark = (row + col) & 1;

	sf::FloatRect backgroundRect = getTextureRect(getBackgroundType(row, col, explodedMine), isDark);
	sf::FloatRect overlayRect = getTextureRect(getOverlayType(row, col, anim), isDark);

	float bgRectLeft = backgroundRect.position.x;
	float bgRectRight = bgRectLeft + backgroundRect.size.x;
	float bgRectTop = backgroundRect.position.y;
	float bgRectBottom = bgRectTop + backgroundRect.size.y;

	float ovRectLeft = overlayRect.position.x;
	float ovRectRight = ovRectLeft + overlayRect.size.x;
	float ovRectTop = overlayRect.position.y;
	float ovRectBottom = ovRectTop + overlayRect.size.y;

	backgroundVA[idx + 0].texCoords = { bgRectLeft, bgRectTop };
	backgroundVA[idx + 1].texCoords = { bgRectRight,bgRectTop };
	backgroundVA[idx + 2].texCoords = { bgRectLeft, bgRectBottom };
	backgroundVA[idx + 3].texCoords = { bgRectRight,bgRectTop };
	backgroundVA[idx + 4].texCoords = { bgRectRight,bgRectBottom };
	backgroundVA[idx + 5].texCoords = { bgRectLeft, bgRectBottom };

	overlayVA[idx + 0].texCoords = { ovRectLeft, ovRectTop };
	overlayVA[idx + 1].texCoords = { ovRectRight,ovRectTop };
	overlayVA[idx + 2].texCoords = { ovRectLeft, ovRectBottom };
	overlayVA[idx + 3].texCoords = { ovRectRight,ovRectTop };
	overlayVA[idx + 4].texCoords = { ovRectRight,ovRectBottom };
	overlayVA[idx + 5].texCoords = { ovRectLeft, ovRectBottom };
}

SpriteType UI::getBackgroundType(int row, int col, std::pair<int, int> explodedMine) {
	Cell& cell = game.getBoard().getCell(row, col);
	if (cell.isMine) {
		return (explodedMine == std::make_pair(row, col)) ? Exploded : Mine;
	} else {
		return static_cast<SpriteType>(Empty + cell.neighborMines);
	}
}

SpriteType UI::getOverlayType(int row, int col, const CellAnim& anim) {
	Cell& cell = game.getBoard().getCell(row, col);
	if (!cell.isRevealed) {
		if (anim.flagProgress > 0.0f && anim.flagProgress < 1.0f) {
			float progress = anim.flagProgress;
			if (anim.removeFlag && progress > 0.0f) progress = 1.0f - progress;
			return getFlagFrame(progress);
		}
		if (cell.isFlagged) {
			return Flagged;
		}
		return Closed;
	} else {
		if (anim.openProgress > 0.0f && anim.openProgress < 1.0f) {
			return getOpenFrame(anim.openProgress);
		}
		return None;
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
	GameState gstate = game.getGameState();
	sf::Vector2i pixelPos(static_cast<int>(mouseX), static_cast<int>(mouseY));
	sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);
	float localX = worldPos.x - gridStartPos.first;
	float localY = worldPos.y - gridStartPos.second;
	int pressedCol = static_cast<int>(localX / (tileSize * cellScalePx));
	int pressedRow = static_cast<int>(localY / (tileSize * cellScalePx));
	if (gstate == GameState::Defeat || gstate == GameState::Victory) return;
	if (localX >= 0 && localY >= 0 && board.isValidMove(pressedRow, pressedCol)) {
		if (game.getGameState() == GameState::Menu) {
			game.startPlaying();
			gameTimer.restart();
		}
		ClickHandler click(game, pressedRow, pressedCol);
		MouseButton btn = (button == sf::Mouse::Button::Left) ? MouseButton::LCM : MouseButton::RCM;
		click(btn);
	}
}

void UI::processEvents() {
	while (const std::optional event = window.pollEvent()) {
		if (event->is<sf::Event::Closed>()) {
			window.close();
		} else if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
			handleInput(static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y), mouseEvent->button);
		} else if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
			if (keyEvent->code == sf::Keyboard::Key::R) {
				int rows = game.getBoard().getRows();
				int cols = game.getBoard().getCols();
				game.restart(rows, cols);
				finalTime = 0.0f;
				gameTimer.restart();
				initLayout();
			}
		}
	}
}

void UI::update() {
	float dt = dtClock.restart().asSeconds();
	Board& board = game.getBoard();
	int rows = board.getRows();
	int cols = board.getCols();
	CellAnim* animState = getAnimState();
	CellState* cellState = getCellState();
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			int idx = i * cols + j;
			const Cell& cell = board.getCell(i, j);
			CellAnim& anim = animState[idx];
			CellState& prev = cellState[idx];
			if (cell.isRevealed && prev != CellState::Open) {
				anim.openProgress = 0.01f;
				prev = CellState::Open;
			}
			if (!cell.isRevealed && cell.isFlagged && prev != CellState::Flagged) {
				anim.flagProgress = 0.01f;
				anim.removeFlag = false;
				prev = CellState::Flagged;
			}
			if (!cell.isRevealed && !cell.isFlagged && prev == CellState::Flagged) {
				anim.flagProgress = 0.01f;
				anim.removeFlag = true;
				prev = CellState::Closed;
			}
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
	int totalMines = rows * cols * default_mine_density;
	textMines.setString("Mines left: " + std::to_string(totalMines - flagsPlaced));

	GameState gstate = game.getGameState();
	if ((gstate == GameState::Defeat || gstate == GameState::Victory) && finalTime == 0.0f) {
		finalTime = gameTimer.getElapsedTime().asSeconds();
	}
	float currentTime = 0.0f;
	if (gstate == GameState::Playing)
		currentTime = gameTimer.getElapsedTime().asSeconds();
	else if (gstate == GameState::Defeat || gstate == GameState::Victory)
		currentTime = finalTime;

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
	window.draw(backgroundVA, &tilesTexture);
	window.draw(overlayVA, &tilesTexture);
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
	: totalMines(static_cast<int>(rows* cols * default_mine_density)),
	board(rows, cols, totalMines) {
	state = GameState::Menu;
}

void Game::updateBoard() {
	if (board.isGameOver()) {
		state = GameState::Defeat;
		board.revealAll();
	} else if (board.isGameWon()) {
		state = GameState::Victory;
		board.revealAll();
	}
}

void Game::startPlaying() {
	if (state == GameState::Menu) state = GameState::Playing;
}

void Game::restart(int rows, int cols) {
	board = Board(rows, cols, totalMines);
	state = GameState::Menu;
}

int main() {
	int rows = 10;
	int cols = 10;
	float minesPercent = 0.21f;
	if (rows < 5) rows = 5;
	if (cols < 5) cols = 5;
	Game game(rows, cols);
	UI ui(game);
	ui.run();
	return 0;
}