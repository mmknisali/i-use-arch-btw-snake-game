#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <string.h>

#define WINDOW_SIZE 512
#define PIXEL_SCALE (WINDOW_SIZE / 16)

#define SAMPLE_RATE 48000
#define AMPLITUDE   28000
uint8_t current_note = 0;

void play_note(SDL_AudioDeviceID dev, uint8_t pitch) {
  // Clear any remaining audio
  SDL_ClearQueuedAudio(dev);

  double freq = 440.0 * pow(2.0, ((int)pitch - 69.0) / 12.0);
  int samples = SAMPLE_RATE / 6; // ~166ms duration
  int16_t *buf = malloc(samples * sizeof(int16_t));
  if (!buf) return;

  // Apply envelope to prevent clicks
  for (int i = 0; i < samples; i++) {
    double t = (double)i / SAMPLE_RATE;
    // Attack-decay envelope
    double envelope = 1.0;
    if (i < SAMPLE_RATE / 50) { // 20ms attack
      envelope = (double)i / (SAMPLE_RATE / 50);
    } else if (i > samples - SAMPLE_RATE / 50) { // 20ms release
      envelope = (double)(samples - i) / (SAMPLE_RATE / 50);
    }
    buf[i] = (int16_t)(AMPLITUDE * envelope * sin(2.0 * M_PI * freq * t));
  }

  SDL_QueueAudio(dev, buf, samples * sizeof(int16_t));
  free(buf);
}

uint16_t program[16777216] = {0};
uint8_t memory[30000] = {0};

size_t program_size = 0, cursor = 0;
int address = 0;

void run_program () {
  while (cursor < program_size) {
    switch (program[cursor++]) {
      case '>':
        address += program[cursor++];
        break;
      case '<':
        address -= program[cursor++];
        break;
      case '+': memory[address] += program[cursor++]; break;
      case '-': memory[address] -= program[cursor++]; break;
      case '[':
        if (!memory[address]) {
          cursor += program[cursor];
        }
        cursor++;
        break;
      case ']':
        if (memory[address]) {
          cursor -= program[cursor];
        }
        cursor++;
        break;
      case '.':
        cursor++;
        return;
      case ',':
        cursor++;
        uint8_t key = 0;
        const uint8_t *keystate = SDL_GetKeyboardState(NULL);
        if (keystate[SDL_SCANCODE_Z]) key |= 0x80;
        if (keystate[SDL_SCANCODE_X]) key |= 0x40;
        if (keystate[SDL_SCANCODE_RETURN]) key |= 0x20;
        if (keystate[SDL_SCANCODE_SPACE]) key |= 0x10;
        if (keystate[SDL_SCANCODE_UP]) key |= 0x08;
        if (keystate[SDL_SCANCODE_DOWN]) key |= 0x04;
        if (keystate[SDL_SCANCODE_LEFT]) key |= 0x02;
        if (keystate[SDL_SCANCODE_RIGHT]) key |= 0x01;
        memory[address] = key;
        break;
      default:
        printf("unexpected char '%c'\n", program[cursor - 1]);
        break;
    }
  }
}

int main (int argc, char *argv[]) {

  if (argc != 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return 1;
  }

  FILE *file = fopen(argv[1], "r");
  if (!file) {
    printf("Failed to open file: %s\n", argv[1]);
    return 1;
  }

  // Read the entire file
  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);
  char *code = malloc(file_size + 1);
  fread(code, 1, file_size, file);
  code[file_size] = '\0';
  fclose(file);

  // Remove comments
  char *new_code = malloc(file_size + 1);
  char *src = code;
  char *dst = new_code;
  while (*src) {
    if (*src == ';') {
      while (*src && *src != '\n') src++;
    } else {
      *dst++ = *src++;
    }
  }
  *dst = '\0';
  free(code);
  code = new_code;

  // Preprocess: replace keywords with BF chars
  char *replacements[][2] = {
    {"i ", ">"},
    {"use ", "<"},
    {"arch ", "+"},
    {"linux ", "-"},
    {"btw ", "."},
    {"by ", ","},
    {"the ", "["},
    {"way ", "]"},
    {NULL, NULL}
  };
  for (int i = 0; replacements[i][0]; i++) {
    char *pos = code;
    while ((pos = strstr(pos, replacements[i][0])) != NULL) {
      memcpy(pos, replacements[i][1], strlen(replacements[i][1]));
      memmove(pos + strlen(replacements[i][1]), pos + strlen(replacements[i][0]), strlen(pos + strlen(replacements[i][0])) + 1);
    }
  }

  // Parse as BF code
  char *ptr = code;
  char ch;
  while ((ch = *ptr++)) {
    switch (ch) {
      case '.':
      case ',':
      case '[':
        program[program_size] = ch;
        program_size += 2;
        break;
      case '>':
      case '<':
      case '+':
      case '-':
        program[program_size++] = ch;
        program[program_size] = 1;
        while (*ptr == ch) {
          program[program_size]++;
          ptr++;
        }
        program_size++;
        break;
      case ']':
        program[program_size] = ch;
        int depth = 1;
        int i = program_size;
        while (i > 0 && depth > 0) {
          i -= 2;
          if (program[i] == '[') depth--;
          if (program[i] == ']') depth++;
        }
        program[i + 1] = program_size - i;
        program[program_size + 1] = program_size - i;
        program_size += 2;
        break;
      default:
        break;
    }
  }

  free(code);

  // SDL initialization
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow("I use Arch btw", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_SIZE, WINDOW_SIZE, SDL_WINDOW_SHOWN);
  if (!window) {
    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  SDL_AudioDeviceID audio_dev = 0;
  SDL_AudioSpec want, have;
  SDL_zero(want);
  want.freq = SAMPLE_RATE;
  want.format = AUDIO_S16SYS;
  want.channels = 1;
  want.samples = 4096;
  audio_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
  if (!audio_dev) {
    printf("Failed to open audio device: %s\n", SDL_GetError());
  }

  // Main loop
  int quit = 0;
  SDL_Event e;
  while (!quit) {
    // Run the program until `.` or end
    run_program();

    // Render the framebuffer
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    for (int y = 0; y < 16; y++) {
      for (int x = 0; x < 16; x++) {
        uint8_t pixel = memory[y * 16 + x];
        uint8_t r = (pixel >> 5) & 0x7;
        uint8_t g = (pixel >> 2) & 0x7;
        uint8_t b = pixel & 0x3;
        r = (r << 5) | (r << 2) | (r >> 1);
        g = (g << 5) | (g << 2) | (g >> 1);
        b = (b << 6) | (b << 4) | (b << 2) | b;
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_Rect rect = {x * PIXEL_SCALE, y * PIXEL_SCALE, PIXEL_SCALE, PIXEL_SCALE};
        SDL_RenderFillRect(renderer, &rect);
      }
    }
    SDL_RenderPresent(renderer);

    // Handle events
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT) {
        quit = 1;
      }
    }

    // Small delay
    SDL_Delay(16); // ~60 FPS
  }

  SDL_CloseAudioDevice(audio_dev);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}