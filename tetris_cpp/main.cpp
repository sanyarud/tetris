#include <SDL.h>
#include <SDL_ttf.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>

// ── Кольори ──────────────────────────────────────────────
const SDL_Color COLORS[] = {
    {0,   220, 220},  // I — ціан
    {220, 220, 0},    // O — жовтий
    {160, 0,   220},  // T — фіолетовий
    {0,   220, 0},    // S — зелений
    {220, 0,   0},    // Z — червоний
    {0,   0,   220},  // J — синій
    {220, 150, 0},    // L — помаранчевий
};

const SDL_Color DARK_COLORS[] = {
    {0,   160, 160},
    {160, 160, 0},
    {100, 0,   160},
    {0,   160, 0},
    {160, 0,   0},
    {0,   0,   160},
    {160, 110, 0},
};

// ── Фігури (4 обертання) ─────────────────────────────────
const int SHAPES[7][4][4][2] = {
    // I
    {{{0,1},{1,1},{2,1},{3,1}}, {{2,0},{2,1},{2,2},{2,3}},
     {{0,2},{1,2},{2,2},{3,2}}, {{1,0},{1,1},{1,2},{1,3}}},
    // O
    {{{1,0},{2,0},{1,1},{2,1}}, {{1,0},{2,0},{1,1},{2,1}},
     {{1,0},{2,0},{1,1},{2,1}}, {{1,0},{2,0},{1,1},{2,1}}},
    // T
    {{{1,0},{0,1},{1,1},{2,1}}, {{1,0},{1,1},{2,1},{1,2}},
     {{0,1},{1,1},{2,1},{1,2}}, {{1,0},{0,1},{1,1},{1,2}}},
    // S
    {{{1,0},{2,0},{0,1},{1,1}}, {{1,0},{1,1},{2,1},{2,2}},
     {{1,1},{2,1},{0,2},{1,2}}, {{0,0},{0,1},{1,1},{1,2}}},
    // Z
    {{{0,0},{1,0},{1,1},{2,1}}, {{2,0},{1,1},{2,1},{1,2}},
     {{0,1},{1,1},{1,2},{2,2}}, {{1,0},{0,1},{1,1},{0,2}}},
    // J
    {{{0,0},{0,1},{1,1},{2,1}}, {{1,0},{2,0},{1,1},{1,2}},
     {{0,1},{1,1},{2,1},{2,2}}, {{1,0},{1,1},{0,2},{1,2}}},
    // L
    {{{2,0},{0,1},{1,1},{2,1}}, {{1,0},{1,1},{1,2},{2,2}},
     {{0,1},{1,1},{2,1},{0,2}}, {{0,0},{1,0},{1,1},{1,2}}},
};

// ── Константи ────────────────────────────────────────────
const int CELL  = 30;
const int COLS  = 10;
const int ROWS  = 20;
const int SIDE  = 180;
const int W     = CELL * COLS + SIDE;
const int H     = CELL * ROWS;

// ── Фігура ───────────────────────────────────────────────
struct Piece {
    int idx, rot, x, y;
    Piece(int i = -1) : idx(i), rot(0), x(COLS/2 - 2), y(0) {
        if (idx < 0) idx = rand() % 7;
    }
    void cells(int out[4][2], int ox=0, int oy=0, int r=-1) const {
        if (r < 0) r = rot;
        for (int i = 0; i < 4; i++) {
            out[i][0] = x + SHAPES[idx][r % 4][i][0] + ox;
            out[i][1] = y + SHAPES[idx][r % 4][i][1] + oy;
        }
    }
};

// ── Гра ──────────────────────────────────────────────────
class Tetris {
    int board[ROWS][COLS];  // -1 = порожньо, 0..6 = індекс кольору
    Piece cur, nxt;
    int score, lines, level;
    bool over, paused;
    Uint32 fall_timer, fall_speed;

    bool valid(const Piece& p, int ox=0, int oy=0, int rot=-1) {
        int c[4][2];
        p.cells(c, ox, oy, rot);
        for (int i = 0; i < 4; i++) {
            int cx = c[i][0], cy = c[i][1];
            if (cx < 0 || cx >= COLS || cy >= ROWS) return false;
            if (cy >= 0 && board[cy][cx] >= 0) return false;
        }
        return true;
    }

    void lock() {
        int c[4][2]; cur.cells(c);
        for (int i = 0; i < 4; i++) {
            if (c[i][1] < 0) { over = true; return; }
            board[c[i][1]][c[i][0]] = cur.idx;
        }
        // Видалення заповнених ліній
        int cleared = 0;
        for (int r = ROWS - 1; r >= 0; r--) {
            bool full = true;
            for (int c = 0; c < COLS; c++) if (board[r][c] < 0) { full = false; break; }
            if (full) {
                for (int rr = r; rr > 0; rr--)
                    for (int c = 0; c < COLS; c++)
                        board[rr][c] = board[rr-1][c];
                for (int c = 0; c < COLS; c++) board[0][c] = -1;
                cleared++; r++; // перевірити цей ряд знову
            }
        }
        const int pts[] = {0, 100, 300, 500, 800};
        score += pts[cleared] * level;
        lines += cleared;
        level = lines / 10 + 1;
        fall_speed = (500 - (level - 1) * 40);
        if (fall_speed < 50) fall_speed = 50;

        cur = nxt;
        nxt = Piece();
        if (!valid(cur)) over = true;
    }

    int ghostY() {
        int oy = 0;
        while (valid(cur, 0, oy + 1)) oy++;
        return oy;
    }

    void hardDrop() {
        int oy = ghostY();
        score += oy * 2;
        cur.y += oy;
        lock();
    }

    bool tryRotate() {
        int nr = (cur.rot + 1) % 4;
        int kicks[] = {0, -1, 1, -2, 2};
        for (int kx : kicks) {
            if (valid(cur, kx, 0, nr)) {
                cur.x += kx; cur.rot = nr; return true;
            }
        }
        return false;
    }

public:
    Tetris() : cur(), nxt(), score(0), lines(0), level(1),
               over(false), paused(false), fall_timer(0), fall_speed(500) {
        memset(board, -1, sizeof(board));
    }

    bool isOver() const { return over; }
    bool isPaused() const { return paused; }
    void togglePause() { paused = !paused; }
    int getScore() const { return score; }
    int getLevel() const { return level; }
    int getLines() const { return lines; }

    void handleKey(SDL_Keycode key) {
        if (over || paused) return;
        switch (key) {
            case SDLK_LEFT:  if (valid(cur, -1, 0)) cur.x--; break;
            case SDLK_RIGHT: if (valid(cur, 1, 0))  cur.x++; break;
            case SDLK_DOWN:  if (valid(cur, 0, 1)) { cur.y++; score++; } break;
            case SDLK_UP:    tryRotate(); break;
            case SDLK_SPACE:  hardDrop(); break;
            default: break;
        }
    }

    void update(Uint32 dt) {
        if (over || paused) return;
        fall_timer += dt;
        if (fall_timer >= fall_speed) {
            fall_timer = 0;
            if (valid(cur, 0, 1)) cur.y++;
            else lock();
        }
    }

    void draw(SDL_Renderer* ren, TTF_Font* font, TTF_Font* big_font, TTF_Font* small_font) {
        // Фон
        SDL_SetRenderDrawColor(ren, 20, 20, 30, 255);
        SDL_RenderClear(ren);

        // Сітка та поле
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                int x = c * CELL, y = r * CELL;
                SDL_SetRenderDrawColor(ren, 30, 30, 40, 255);
                SDL_Rect cell = {x, y, CELL, CELL};
                SDL_RenderFillRect(ren, &cell);
                SDL_SetRenderDrawColor(ren, 50, 50, 50, 255);
                SDL_RenderDrawRect(ren, &cell);

                if (board[r][c] >= 0) {
                    int ci = board[r][c];
                    drawBlock(ren, x, y, COLORS[ci], DARK_COLORS[ci]);
                }
            }
        }

        if (!over) {
            // Привид
            int gy = ghostY();
            int gc[4][2]; cur.cells(gc, 0, gy);
            for (int i = 0; i < 4; i++) {
                if (gc[i][1] >= 0) {
                    SDL_SetRenderDrawColor(ren, 80, 80, 80, 255);
                    SDL_Rect r = {gc[i][0]*CELL+1, gc[i][1]*CELL+1, CELL-2, CELL-2};
                    SDL_RenderDrawRect(ren, &r);
                }
            }
            // Поточна фігура
            int cc[4][2]; cur.cells(cc);
            for (int i = 0; i < 4; i++) {
                if (cc[i][1] >= 0) {
                    drawBlock(ren, cc[i][0]*CELL, cc[i][1]*CELL,
                              COLORS[cur.idx], DARK_COLORS[cur.idx]);
                }
            }
        }

        // Бічна панель
        int sx = CELL * COLS + 15;
        SDL_SetRenderDrawColor(ren, 50, 50, 50, 255);
        SDL_RenderDrawLine(ren, CELL*COLS, 0, CELL*COLS, H);

        // Наступна фігура
        drawText(ren, font, "НАСТУПНА", sx, 15, {255,255,255});
        for (int i = 0; i < 4; i++) {
            int bx = sx + SHAPES[nxt.idx][0][i][0] * 22;
            int by = 50 + SHAPES[nxt.idx][0][i][1] * 22;
            SDL_Rect r = {bx, by, 20, 20};
            SDL_SetRenderDrawColor(ren, COLORS[nxt.idx].r, COLORS[nxt.idx].g, COLORS[nxt.idx].b, 255);
            SDL_RenderFillRect(ren, &r);
        }

        // Рахунок
        int yoff = 170;
        const char* labels[] = {"РАХУНОК", "ЛІНІЇ", "РІВЕНЬ"};
        int values[] = {score, lines, level};
        for (int i = 0; i < 3; i++) {
            drawText(ren, small_font, labels[i], sx, yoff, {180,180,180});
            char buf[32]; snprintf(buf, sizeof(buf), "%d", values[i]);
            drawText(ren, font, buf, sx, yoff + 22, {255,255,255});
            yoff += 60;
        }

        // Керування
        yoff = H - 160;
        const char* controls[] = {
            "\xE2\x86\x90 \xE2\x86\x92  \xD1\x80\xD1\x83\xD1\x85",
            "\xE2\x86\x91     \xD0\xBE\xD0\xB1\xD0\xB5\xD1\x80\xD1\x82\xD0\xB0\xD0\xBD\xD0\xBD\xD1\x8F",
            "\xE2\x86\x93     \xD0\xBC'\xD1\x8F\xD0\xBA\xD0\xB5 \xD0\xBF\xD0\xB0\xD0\xB4\xD1\x96\xD0\xBD\xD0\xBD\xD1\x8F",
            "SPACE \xD1\x85\xD0\xB0\xD1\x80\xD0\xB4 \xD0\xB4\xD1\x80\xD0\xBE\xD0\xBF",
            "P     \xD0\xBF\xD0\xB0\xD1\x83\xD0\xB7\xD0\xB0",
        };
        for (auto line : controls) {
            drawText(ren, small_font, line, sx, yoff, {120,120,140});
            yoff += 24;
        }

        // Автор
        drawText(ren, small_font, "\xC2\xA9 sanyarud", sx, H - 28, {80,80,100});

        // Game Over
        if (over) {
            SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 150);
            SDL_Rect ov = {0, 0, CELL*COLS, H};
            SDL_RenderFillRect(ren, &ov);
            int cx = CELL*COLS/2;
            drawTextCentered(ren, big_font, "\xD0\x93\xD0\xA0\xD0\xA3 \xD0\x97\xD0\x90\xD0\x9A\xD0\x86\xD0\x9D\xD0\xA7\xD0\x95\xD0\x9D\xD0\x9E", cx, H/2-50, {255,60,60});
            char buf[64]; snprintf(buf, sizeof(buf), "\xD0\xA0\xD0\xB0\xD1\x85\xD1\x83\xD0\xBD\xD0\xBE\xD0\xBA: %d", score);
            drawTextCentered(ren, font, buf, cx, H/2+10, {255,255,255});
            drawTextCentered(ren, small_font, "R \xE2\x80\x94 \xD1\x80\xD0\xB5\xD1\x81\xD1\x82\xD0\xB0\xD1\x80\xD1\x82   ESC \xE2\x80\x94 \xD0\xB2\xD0\xB8\xD1\x85\xD1\x96\xD0\xB4", cx, H/2+50, {255,255,255});
        }

        // Пауза
        if (paused && !over) {
            SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 150);
            SDL_Rect ov = {0, 0, CELL*COLS, H};
            SDL_RenderFillRect(ren, &ov);
            drawTextCentered(ren, big_font, "\xD0\x9F\xD0\x90\xD0\xA3\xD0\x97\xD0\x90", CELL*COLS/2, H/2-20, {255,255,255});
        }

        SDL_RenderPresent(ren);
    }

private:
    void drawBlock(SDL_Renderer* ren, int x, int y, SDL_Color col, SDL_Color dark) {
        SDL_Rect r = {x+1, y+1, CELL-2, CELL-2};
        SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, 255);
        SDL_RenderFillRect(ren, &r);
        // Блиск
        SDL_Rect hl = {x+3, y+3, CELL/3, CELL/3};
        SDL_SetRenderDrawColor(ren,
            std::min(255, col.r+60),
            std::min(255, col.g+60),
            std::min(255, col.b+60), 255);
        SDL_RenderFillRect(ren, &hl);
    }

    void drawText(SDL_Renderer* ren, TTF_Font* f, const char* text, int x, int y, SDL_Color col) {
        SDL_Surface* s = TTF_RenderUTF8_Blended(f, text, col);
        SDL_Texture* t = SDL_CreateTextureFromSurface(ren, s);
        SDL_Rect dst = {x, y, s->w, s->h};
        SDL_RenderCopy(ren, t, nullptr, &dst);
        SDL_DestroyTexture(t);
        SDL_FreeSurface(s);
    }

    void drawTextCentered(SDL_Renderer* ren, TTF_Font* f, const char* text, int cx, int y, SDL_Color col) {
        SDL_Surface* s = TTF_RenderUTF8_Blended(f, text, col);
        SDL_Texture* t = SDL_CreateTextureFromSurface(ren, s);
        SDL_Rect dst = {cx - s->w/2, y, s->w, s->h};
        SDL_RenderCopy(ren, t, nullptr, &dst);
        SDL_DestroyTexture(t);
        SDL_FreeSurface(s);
    }
};

// ── Main ─────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    srand(time(nullptr));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL Init: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() < 0) {
        fprintf(stderr, "TTF Init: %s\n", TTF_GetError());
        return 1;
    }

    SDL_Window* win = SDL_CreateWindow(
        "\xD0\xA2\xD0\xB5\xD1\x82\xD1\x80\xD1\x96\xD1\x81",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W, H, 0);
    if (!win) { fprintf(stderr, "Window: %s\n", SDL_GetError()); return 1; }

    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font* font      = TTF_OpenFont("/System/Library/Fonts/Helvetica.ttc", 22);
    TTF_Font* big_font  = TTF_OpenFont("/System/Library/Fonts/Helvetica.ttc", 36);
    TTF_Font* small_font= TTF_OpenFont("/System/Library/Fonts/Helvetica.ttc", 18);

    Tetris game;
    Uint32 last = SDL_GetTicks();

    while (true) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) goto quit;
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) goto quit;
                if (e.key.keysym.sym == SDLK_r && game.isOver()) {
                    game = Tetris(); last = SDL_GetTicks(); continue;
                }
                if (e.key.keysym.sym == SDLK_p && !game.isOver()) {
                    game.togglePause(); continue;
                }
                game.handleKey(e.key.keysym.sym);
            }
        }

        Uint32 now = SDL_GetTicks();
        Uint32 dt = now - last;
        last = now;
        game.update(dt);
        game.draw(ren, font, big_font, small_font);
        SDL_Delay(1000 / 60);
    }

quit:
    TTF_CloseFont(font);
    TTF_CloseFont(big_font);
    TTF_CloseFont(small_font);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    return 0;
}