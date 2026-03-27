
![C++](https://img.shields.io/badge/C++-17%2F20-blue.svg)
![SFML](https://img.shields.io/badge/Library-SFML%203.x-green.svg)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)

# Saper Game (SFML)




## Features

- Smart mine placement
- Smooth animations
- Selfdrawn textures
- Chording mechanic
- PC adaptive layout


## Tech Stack

**Langauge:** C++ 17/20

**Library:** SFML 3.x.x


## Usage/Examples

**LMB:** Open cell/Chording(on opened cell)

**RMB:** Flag cell




## Structure

The project is divided into two main versions sharing the same core logic:

- ConsoleVersion/: A lightweight, CLI-based version of Minesweeper.

- SFMLVersion/: A modern GUI version with custom textures and animations.

- Core Logic: Handled by board.hpp/cpp (shared between both versions).

```Plaintext
├── ConsoleVersion/          # CLI implementation
│   ├── board.cpp/hpp        # Shared core game logic
│   └── console.cpp          # Console-specific entry point
├── SFMLVersion/             # Graphical implementation
│   ├── SFML.cpp             # GUI-specific entry point & rendering
│   ├── tiles.png            # Custom hand-drawn textures
│   └── font.ttf             # Game font
├── Saper_game.slnx          # Visual Studio Solution (modern format)
└── .gitignore               # Keeps your repo clean
```
## Run Locally

Clone the project

```bash
  git clone https://github.com/Yah1d0/Saper_Game
```

Go to the project directory

```bash
  cd Saper_Game
```

Install dependencies

- Download SFML 3.x.x version first

### Using Visual Studio (Recommended)

1. Open Saper_game.slnx.

2. Right-click on either ConsoleVersion or SFMLVersion in the Solution Explorer.
3. Select "Set as Startup Project".

4. Press F5 to compile and run.

### Manual Compilation (g++)

**Console Build:**
```Bash
g++ ConsoleVersion/console.cpp ConsoleVersion/board.cpp -o SaperConsole
```

**SFML Build:**

```Bash
g++ SFMLVersion/SFML.cpp ConsoleVersion/board.cpp -IConsoleVersion -o SaperGUI
```

*(Note: Ensure SFML 3.x headers and libraries are in your compiler's search path.)*

