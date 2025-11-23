# I use Arch btw Visual Interpreter

This is a visual interpreter for the I use Arch btw esoteric programming language, based on Brainfuck.

## Installation and Usage

### Step 1: Install Dependencies

Ensure you have GCC and SDL2 installed.

On Ubuntu/Debian:
```bash
sudo apt update
sudo apt install gcc libsdl2-dev
```

On Arch Linux:
```bash
sudo pacman -S gcc sdl2
```

On other systems, install SDL2 development libraries accordingly.

### Step 2: Compile the Interpreter

In the project directory, compile the source code:

```bash
gcc visual_interpreter.c -o visual_interpreter -lSDL2 -lm
```

This creates the `visual_interpreter` executable.

### Step 3: Run the Snake Game

Execute the interpreter with the snake game file:

```bash
./visual_interpreter snake.archbtw
```

A 512x512 window will open displaying the game. Use arrow keys to control the snake. Close the window to exit.

### Troubleshooting

- If compilation fails, ensure SDL2 is installed.
- If the window doesn't open, check your display environment (e.g., X11 on Linux).
- The game requires a graphical desktop.

## Requirements

- GCC
- SDL2 development libraries
- Linux or compatible system with graphical display

## Language

I use Arch btw is based on Brainfuck with keyword commands:

- i: >
- use: <
- arch: +
- linux: -
- btw: .
- by: ,
- the: [
- way: ]

Programs are text files with keywords separated by whitespace, comments start with ;.

## License

This project is licensed under the GNU General Public License v3.0 (GPL-3.0). See the LICENSE file for details.

## Credits

Based on bf16 by p2r3, which is also GPL-3.0 licensed.