# I use Arch btw Visual Interpreter

This is a visual interpreter for the I use Arch btw esoteric programming language, based on Brainfuck.

## Usage

Compile the interpreter:

```bash
gcc visual_interpreter.c -o visual_interpreter -lSDL2 -lm
```

Run the snake game:

```bash
./visual_interpreter snake.archbtw
```

Use arrow keys to control the snake. The game runs in a 512x512 window with 16x16 pixel framebuffer.

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