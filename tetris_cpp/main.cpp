#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>

// ── Звукова система (генерація програмно) ────────────────
struct SoundGen {
    static Mix_Chunk* makeTone(int freq, int duration_ms, int vol = 80, float decay = 0.5f) {
        int samples = 44100 * duration_ms / 1000;
        int len = samples * 2; // 16-bit mono
        Uint8* buf = new Uint8[len];
        Sint16* ptr = (Sint16*)buf;
        for (int i = 0; i < samples; i++) {
            float t = (float)i / 44100.0f;
            float env = powf(1.0f - (float)i / samples, decay);
            float val = sinf(2.0f * M_PI * freq * t) * env * vol * 327;
            ptr[i] = (Sint16)std::max(-32767.0f, std::min(32767.0f, val));
        }
        Mix_Chunk* chunk = new Mix_Chunk();
        chunk->allocated = 1;
        chunk->abuf = buf;
        chunk->alen = len;
        chunk->volume = 128;
        return chunk;
    }

    static Mix_Chunk* makeNoise(int duration_ms, int vol = 60, float decay = 1.0f) {
        int samples = 44100 * duration_ms / 1000;
        int len = samples * 2;
        Uint8* buf = new Uint8[len];
        Sint16* ptr = (Sint16*)buf;
        for (int i = 0; i < samples; i++) {
            float env = powf(1.0f - (float)i / samples, decay);
            float val = ((rand() % 65536) - 32768) * env * vol / 100.0f;
            ptr[i] = (Sint16)std::max(-32767.0f, std::min(32767.0f, val));
        }
        Mix_Chunk* chunk = new Mix_Chunk();
        chunk->allocated = 1;
        chunk->abuf = buf;
        chunk->alen = len;
        chunk->volume = 128;
        return chunk;
    }

    // Ефект лінії — висхідний свіп
    static Mix_Chunk* makeSweep(int freqStart, int freqEnd, int duration_ms, int vol = 70) {
        int samples = 44100 * duration_ms / 1000;
        int len = samples * 2;
        Uint8* buf = new Uint8[len];
        Sint16* ptr = (Sint16*)buf;
        float phase = 0;
        for (int i = 0; i < samples; i++) {
            float t = (float)i / samples;
            float freq = freqStart + (freqEnd - freqStart) * t;
            float env = 1.0f - t * 0.3f;
            phase += 2.0f * M_PI * freq / 44100.0f;
            float val = sinf(phase) * env * vol * 327;
            ptr[i] = (Sint16)std::max(-32767.0f, std::min(32767.0f, val));
        }
        Mix_Chunk* chunk = new Mix_Chunk();
        chunk->allocated = 1;
        chunk->abuf = buf;
        chunk->alen = len;
        chunk->volume = 128;
        return chunk;
    }

    // Ефект рівня — акорд
    static Mix_Chunk* makeChord(int freq1, int freq2, int freq3, int duration_ms, int vol = 60) {
        int samples = 44100 * duration_ms / 1000;
        int len = samples * 2;
        Uint8* buf = new Uint8[len];
        Sint16* ptr = (Sint16*)buf;
        for (int i = 0; i < samples; i++) {
            float t = (float)i / 44100.0f;
            float env = powf(1.0f - (float)i / samples, 0.3f);
            float val = (sinf(2*M_PI*freq1*t) + sinf(2*M_PI*freq2*t) + sinf(2*M_PI*freq3*t))
                        / 3.0f * env * vol * 327;
            ptr[i] = (Sint16)std::max(-32767.0f, std::min(32767.0f, val));
        }
        Mix_Chunk* chunk = new Mix_Chunk();
        chunk->allocated = 1;
        chunk->abuf = buf;
        chunk->alen = len;
        chunk->volume = 128;
        return chunk;
    }
};

// ── Система частинок ────────────────────────────────────
struct Particle {
    float x, y, vx, vy;
    float life, maxLife;
    SDL_Color color;
    float size;
};

class ParticleSystem {
    std::vector<Particle> particles;
public:
    void emit(float x, float y, SDL_Color col, int count, float speed = 3.0f, float sz = 4.0f) {
        for (int i = 0; i < count; i++) {
            Particle p;
            p.x = x; p.y = y;
            float angle = (float)(rand() % 360) * M_PI / 180.0f;
            float spd = speed * (0.5f + (rand() % 100) / 100.0f);
            p.vx = cosf(angle) * spd;
            p.vy = sinf(angle) * spd - 1.5f; // трохи вгору
            p.life = p.maxLife = 0.5f + (rand() % 50) / 100.0f;
            p.color = col;
            p.size = sz * (0.5f + (rand() % 100) / 100.0f);
            particles.push_back(p);
        }
    }

    void emitLine(float y, float width, SDL_Color col, int count) {
        for (int i = 0; i < count; i++) {
            Particle p;
            p.x = (float)(rand() % (int)width);
            p.y = y;
            p.vx = ((rand() % 200) - 100) / 20.0f;
            p.vy = -((rand() % 100) / 15.0f) - 2.0f;
            p.life = p.maxLife = 0.6f + (rand() % 60) / 100.0f;
            p.color = col;
            p.size = 3.0f + (rand() % 40) / 10.0f;
            particles.push_back(p);
        }
    }

    // Вогняні іскри вниз
    void emitSparks(float x, float y, SDL_Color col, int count) {
        for (int i = 0; i < count; i++) {
            Particle p;
            p.x = x + (rand() % 20) - 10;
            p.y = y;
            p.vx = ((rand() % 100) - 50) / 30.0f;
            p.vy = (rand() % 100) / 20.0f + 1.0f;
            p.life = p.maxLife = 0.3f + (rand() % 30) / 100.0f;
            p.color = col;
            p.size = 2.0f + (rand() % 20) / 10.0f;
            particles.push_back(p);
        }
    }

    void update(float dt) {
        for (auto& p : particles) {
            p.x += p.vx;
            p.y += p.vy;
            p.vy += 4.0f * dt; // гравітація
            p.life -= dt;
        }
        particles.erase(
            std::remove_if(particles.begin(), particles.end(),
                [](const Particle& p) { return p.life <= 0; }),
            particles.end());
    }

    void draw(SDL_Renderer* ren) {
        SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
        for (auto& p : particles) {
            float alpha = (p.life / p.maxLife);
            SDL_SetRenderDrawColor(ren, p.color.r, p.color.g, p.color.b,
                                   (Uint8)(alpha * 255));
            int s = (int)(p.size * alpha);
            if (s < 1) s = 1;
            SDL_Rect r = {(int)p.x - s/2, (int)p.y - s/2, s, s};
            SDL_RenderFillRect(ren, &r);
        }
    }

    bool active() const { return !particles.empty(); }
};

// ── Кольори ──────────────────────────────────────────────
const SDL_Color COLORS[] = {
    {0,   220, 220, 255},  // I — ціан
    {220, 220, 0,   255},  // O — жовтий
    {160, 0,   220, 255},  // T — фіолетовий
    {0,   220, 0,   255},  // S — зелений
    {220, 0,   0,   255},  // Z — червоний
    {0,   0,   220, 255},  // J — синій
    {220, 150, 0,   255},  // L — помаранчевий
};

const SDL_Color GLOW_COLORS[] = {
    {100, 255, 255, 255},
    {255, 255, 100, 255},
    {220, 100, 255, 255},
    {100, 255, 100, 255},
    {255, 100, 100, 255},
    {100, 100, 255, 255},
    {255, 200, 100, 255},
};

const SDL_Color DARK_COLORS[] = {
    {0,   160, 160, 255},
    {160, 160, 0,   255},
    {100, 0,   160, 255},
    {0,   160, 0,   255},
    {160, 0,   0,   255},
    {0,   0,   160, 255},
    {160, 110, 0,   255},
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
const int SIDE  = 200;
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
    int board[ROWS][COLS];
    Piece cur, nxt, hold_piece;
    bool has_hold, hold_used;
    int score, lines, level;
    bool over, paused;
    Uint32 fall_timer, fall_speed;

    // Ефекти
    int combo;
    int max_combo;
    int total_tetris;  // кількість 4-рядкових очищень
    float shake_x, shake_y;
    float shake_timer;
    float flash_timer;      // Білий спалах при очищенні
    int flash_rows[4];
    int flash_count;
    float combo_display_timer;
    float level_up_timer;
    int prev_level;
    float danger_pulse;     // Пульсація небезпеки
    float lock_flash_timer; // Спалах при фіксації
    int lock_flash_cells[4][2];
    bool last_was_tetris;   // Back-to-back Tetris

    // Звуки
    Mix_Chunk* snd_move;
    Mix_Chunk* snd_rotate;
    Mix_Chunk* snd_drop;
    Mix_Chunk* snd_lock;
    Mix_Chunk* snd_clear1;
    Mix_Chunk* snd_clear2;
    Mix_Chunk* snd_clear3;
    Mix_Chunk* snd_clear4;  // TETRIS!
    Mix_Chunk* snd_levelup;
    Mix_Chunk* snd_gameover;
    Mix_Chunk* snd_combo;
    Mix_Chunk* snd_hold;
    Mix_Chunk* snd_danger;

    ParticleSystem particles;

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

    int highestRow() {
        for (int r = 0; r < ROWS; r++)
            for (int c = 0; c < COLS; c++)
                if (board[r][c] >= 0) return r;
        return ROWS;
    }

    void lock() {
        int c[4][2]; cur.cells(c);
        for (int i = 0; i < 4; i++) {
            if (c[i][1] < 0) { over = true; Mix_PlayChannel(-1, snd_gameover, 0); return; }
            board[c[i][1]][c[i][0]] = cur.idx;
            lock_flash_cells[i][0] = c[i][0];
            lock_flash_cells[i][1] = c[i][1];
        }
        lock_flash_timer = 0.15f;

        // Іскри при фіксації
        for (int i = 0; i < 4; i++) {
            particles.emitSparks(c[i][0] * CELL + CELL/2, c[i][1] * CELL + CELL,
                                 GLOW_COLORS[cur.idx], 3);
        }

        Mix_PlayChannel(-1, snd_lock, 0);

        // Видалення заповнених ліній
        int cleared = 0;
        flash_count = 0;
        for (int r = ROWS - 1; r >= 0; r--) {
            bool full = true;
            for (int cc = 0; cc < COLS; cc++) if (board[r][cc] < 0) { full = false; break; }
            if (full) {
                if (flash_count < 4) flash_rows[flash_count] = r;

                // Частинки з кожної клітинки лінії
                for (int cc = 0; cc < COLS; cc++) {
                    int ci = board[r][cc];
                    if (ci >= 0) {
                        particles.emit(cc * CELL + CELL/2, r * CELL + CELL/2,
                                       GLOW_COLORS[ci], 6, 4.0f, 5.0f);
                    }
                }
                // Хвиля частинок по всій лінії
                particles.emitLine(r * CELL + CELL/2, CELL * COLS,
                                   {255, 255, 200, 255}, 20);

                for (int rr = r; rr > 0; rr--)
                    for (int cc = 0; cc < COLS; cc++)
                        board[rr][cc] = board[rr-1][cc];
                for (int cc = 0; cc < COLS; cc++) board[0][cc] = -1;
                cleared++; flash_count++; r++;
            }
        }

        if (cleared > 0) {
            combo++;
            if (combo > max_combo) max_combo = combo;
            combo_display_timer = 2.0f;

            // Shake залежно від кількості ліній
            shake_timer = 0.15f + cleared * 0.05f;
            flash_timer = 0.2f;

            // Звук залежно від кількості ліній
            switch (cleared) {
                case 1: Mix_PlayChannel(-1, snd_clear1, 0); break;
                case 2: Mix_PlayChannel(-1, snd_clear2, 0); break;
                case 3: Mix_PlayChannel(-1, snd_clear3, 0); break;
                case 4:
                    Mix_PlayChannel(-1, snd_clear4, 0);
                    total_tetris++;
                    shake_timer = 0.4f;
                    // Мега-вибух частинок для TETRIS!
                    for (int i = 0; i < 4; i++) {
                        particles.emitLine(flash_rows[i] * CELL + CELL/2,
                                           CELL * COLS, {255, 220, 50, 255}, 30);
                    }
                    break;
            }

            // Бонус за комбо
            if (combo > 1) {
                Mix_PlayChannel(-1, snd_combo, 0);
            }

            // Back-to-back Tetris бонус
            bool b2b = (cleared == 4 && last_was_tetris);
            last_was_tetris = (cleared == 4);

            const int pts[] = {0, 100, 300, 500, 800};
            int base = pts[cleared] * level;
            int combo_bonus = (combo > 1) ? 50 * (combo - 1) * level : 0;
            int b2b_bonus = b2b ? base / 2 : 0;
            score += base + combo_bonus + b2b_bonus;
        } else {
            combo = 0;
            last_was_tetris = false;
        }

        lines += cleared;
        int new_level = lines / 10 + 1;
        if (new_level > level) {
            level = new_level;
            level_up_timer = 2.0f;
            Mix_PlayChannel(-1, snd_levelup, 0);
            // Святкові частинки
            for (int i = 0; i < 50; i++) {
                SDL_Color col = GLOW_COLORS[rand() % 7];
                particles.emit(rand() % (CELL * COLS), rand() % H, col, 1, 5.0f, 6.0f);
            }
        }
        level = new_level;
        fall_speed = (500 - (level - 1) * 40);
        if (fall_speed < 50) fall_speed = 50;

        // Звук небезпеки коли стовпчик високий
        if (highestRow() <= 4 && !over) {
            Mix_PlayChannel(7, snd_danger, 0);
        }

        hold_used = false;
        cur = nxt;
        nxt = Piece();
        if (!valid(cur)) { over = true; Mix_PlayChannel(-1, snd_gameover, 0); }
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
        shake_timer = 0.1f;
        Mix_PlayChannel(-1, snd_drop, 0);
        lock();
    }

    bool tryRotate() {
        int nr = (cur.rot + 1) % 4;
        int kicks[] = {0, -1, 1, -2, 2};
        for (int kx : kicks) {
            if (valid(cur, kx, 0, nr)) {
                cur.x += kx; cur.rot = nr;
                Mix_PlayChannel(-1, snd_rotate, 0);
                return true;
            }
        }
        return false;
    }

    void doHold() {
        if (hold_used) return;
        Mix_PlayChannel(-1, snd_hold, 0);
        hold_used = true;
        if (!has_hold) {
            has_hold = true;
            hold_piece = Piece(cur.idx);
            cur = nxt;
            nxt = Piece();
        } else {
            int tmp = cur.idx;
            cur = Piece(hold_piece.idx);
            hold_piece = Piece(tmp);
        }
    }

    void initSounds() {
        snd_move    = SoundGen::makeTone(200, 50, 30, 2.0f);
        snd_rotate  = SoundGen::makeTone(400, 80, 40, 1.5f);
        snd_drop    = SoundGen::makeNoise(100, 50, 2.0f);
        snd_lock    = SoundGen::makeTone(150, 60, 35, 2.0f);
        snd_clear1  = SoundGen::makeSweep(300, 600, 200, 60);
        snd_clear2  = SoundGen::makeSweep(300, 800, 250, 65);
        snd_clear3  = SoundGen::makeSweep(300, 1000, 300, 70);
        snd_clear4  = SoundGen::makeSweep(200, 1200, 500, 80); // TETRIS!
        snd_levelup = SoundGen::makeChord(523, 659, 784, 600, 70); // C-E-G
        snd_gameover= SoundGen::makeSweep(400, 80, 800, 70);
        snd_combo   = SoundGen::makeTone(800, 100, 40, 1.0f);
        snd_hold    = SoundGen::makeTone(500, 60, 30, 2.0f);
        snd_danger  = SoundGen::makeTone(100, 300, 25, 0.5f);
    }

    void freeSounds() {
        auto freeChunk = [](Mix_Chunk* c) {
            if (c) { delete[] c->abuf; delete c; }
        };
        freeChunk(snd_move); freeChunk(snd_rotate); freeChunk(snd_drop);
        freeChunk(snd_lock); freeChunk(snd_clear1); freeChunk(snd_clear2);
        freeChunk(snd_clear3); freeChunk(snd_clear4); freeChunk(snd_levelup);
        freeChunk(snd_gameover); freeChunk(snd_combo); freeChunk(snd_hold);
        freeChunk(snd_danger);
    }

public:
    Tetris() : cur(), nxt(), hold_piece(-1), has_hold(false), hold_used(false),
               score(0), lines(0), level(1),
               over(false), paused(false), fall_timer(0), fall_speed(500),
               combo(0), max_combo(0), total_tetris(0),
               shake_x(0), shake_y(0), shake_timer(0),
               flash_timer(0), flash_count(0),
               combo_display_timer(0), level_up_timer(0), prev_level(1),
               danger_pulse(0), lock_flash_timer(0), last_was_tetris(false) {
        memset(board, -1, sizeof(board));
        memset(flash_rows, 0, sizeof(flash_rows));
        memset(lock_flash_cells, 0, sizeof(lock_flash_cells));
        initSounds();
    }

    ~Tetris() { freeSounds(); }

    bool isOver() const { return over; }
    bool isPaused() const { return paused; }
    void togglePause() { paused = !paused; }
    int getScore() const { return score; }
    int getLevel() const { return level; }
    int getLines() const { return lines; }

    void reset() {
        freeSounds();
        memset(board, -1, sizeof(board));
        cur = Piece(); nxt = Piece();
        hold_piece = Piece(-1); has_hold = false; hold_used = false;
        score = 0; lines = 0; level = 1;
        over = false; paused = false;
        fall_timer = 0; fall_speed = 500;
        combo = 0; max_combo = 0; total_tetris = 0;
        shake_x = 0; shake_y = 0; shake_timer = 0;
        flash_timer = 0; flash_count = 0;
        combo_display_timer = 0; level_up_timer = 0; prev_level = 1;
        danger_pulse = 0; lock_flash_timer = 0; last_was_tetris = false;
        initSounds();
    }

    void handleKey(SDL_Keycode key) {
        if (over || paused) return;
        switch (key) {
            case SDLK_LEFT:
                if (valid(cur, -1, 0)) { cur.x--; Mix_PlayChannel(-1, snd_move, 0); }
                break;
            case SDLK_RIGHT:
                if (valid(cur, 1, 0)) { cur.x++; Mix_PlayChannel(-1, snd_move, 0); }
                break;
            case SDLK_DOWN:
                if (valid(cur, 0, 1)) { cur.y++; score++; Mix_PlayChannel(-1, snd_move, 0); }
                break;
            case SDLK_UP:    tryRotate(); break;
            case SDLK_SPACE: hardDrop(); break;
            case SDLK_c:     doHold(); break;
            default: break;
        }
    }

    void update(Uint32 dt) {
        float dtf = dt / 1000.0f;

        // Оновлення частинок завжди
        particles.update(dtf);

        // Shake
        if (shake_timer > 0) {
            shake_timer -= dtf;
            shake_x = (rand() % 7 - 3) * (shake_timer * 10);
            shake_y = (rand() % 7 - 3) * (shake_timer * 10);
        } else {
            shake_x = shake_y = 0;
        }

        // Таймери ефектів
        if (flash_timer > 0) flash_timer -= dtf;
        if (combo_display_timer > 0) combo_display_timer -= dtf;
        if (level_up_timer > 0) level_up_timer -= dtf;
        if (lock_flash_timer > 0) lock_flash_timer -= dtf;

        // Пульсація небезпеки
        int hr = highestRow();
        if (hr <= 5) {
            danger_pulse += dtf * 4.0f;
        } else {
            danger_pulse *= 0.95f;
        }

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

        // Зсув від тряски
        int ox = (int)shake_x, oy = (int)shake_y;

        // Пульсація небезпеки — червоне свічення зверху
        if (danger_pulse > 0.1f) {
            SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
            float pulse = (sinf(danger_pulse) + 1.0f) / 2.0f;
            int hr = highestRow();
            float intensity = std::min(1.0f, (6.0f - hr) / 6.0f);
            for (int r = 0; r < std::min(6, ROWS); r++) {
                Uint8 alpha = (Uint8)(pulse * intensity * 40 * (6 - r) / 6);
                SDL_SetRenderDrawColor(ren, 255, 0, 0, alpha);
                SDL_Rect dr = {ox, r * CELL + oy, CELL * COLS, CELL};
                SDL_RenderFillRect(ren, &dr);
            }
        }

        // Сітка та поле
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                int x = c * CELL + ox, y = r * CELL + oy;
                SDL_SetRenderDrawColor(ren, 30, 30, 40, 255);
                SDL_Rect cell = {x, y, CELL, CELL};
                SDL_RenderFillRect(ren, &cell);
                SDL_SetRenderDrawColor(ren, 50, 50, 50, 255);
                SDL_RenderDrawRect(ren, &cell);

                if (board[r][c] >= 0) {
                    int ci = board[r][c];
                    drawBlock(ren, x, y, COLORS[ci], DARK_COLORS[ci], GLOW_COLORS[ci]);
                }
            }
        }

        // Спалах при фіксації блоку
        if (lock_flash_timer > 0) {
            SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
            Uint8 alpha = (Uint8)(lock_flash_timer / 0.15f * 180);
            SDL_SetRenderDrawColor(ren, 255, 255, 255, alpha);
            for (int i = 0; i < 4; i++) {
                SDL_Rect r = {lock_flash_cells[i][0]*CELL+ox, lock_flash_cells[i][1]*CELL+oy,
                              CELL, CELL};
                SDL_RenderFillRect(ren, &r);
            }
        }

        // Білий спалах при очищенні ліній
        if (flash_timer > 0) {
            SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
            Uint8 alpha = (Uint8)(flash_timer / 0.2f * 200);
            SDL_SetRenderDrawColor(ren, 255, 255, 255, alpha);
            SDL_Rect fr = {ox, 0 + oy, CELL * COLS, H};
            SDL_RenderFillRect(ren, &fr);
        }

        if (!over) {
            // Привид з анімацією
            int gy = ghostY();
            int gc[4][2]; cur.cells(gc, 0, gy);
            float ghost_pulse = (sinf(SDL_GetTicks() / 200.0f) + 1.0f) / 2.0f;
            Uint8 ghost_alpha = (Uint8)(40 + ghost_pulse * 40);
            for (int i = 0; i < 4; i++) {
                if (gc[i][1] >= 0) {
                    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(ren, COLORS[cur.idx].r, COLORS[cur.idx].g,
                                           COLORS[cur.idx].b, ghost_alpha);
                    SDL_Rect r = {gc[i][0]*CELL+2+ox, gc[i][1]*CELL+2+oy, CELL-4, CELL-4};
                    SDL_RenderFillRect(ren, &r);
                    SDL_SetRenderDrawColor(ren, GLOW_COLORS[cur.idx].r, GLOW_COLORS[cur.idx].g,
                                           GLOW_COLORS[cur.idx].b, ghost_alpha + 30);
                    SDL_RenderDrawRect(ren, &r);
                }
            }
            // Поточна фігура
            int cc[4][2]; cur.cells(cc);
            for (int i = 0; i < 4; i++) {
                if (cc[i][1] >= 0) {
                    drawBlock(ren, cc[i][0]*CELL+ox, cc[i][1]*CELL+oy,
                              COLORS[cur.idx], DARK_COLORS[cur.idx], GLOW_COLORS[cur.idx]);
                }
            }
        }

        // Частинки
        particles.draw(ren);

        // Бічна панель
        int sx = CELL * COLS + 15;
        SDL_SetRenderDrawColor(ren, 50, 50, 50, 255);
        SDL_RenderDrawLine(ren, CELL*COLS, 0, CELL*COLS, H);

        // ── Hold ─────────
        drawText(ren, small_font, "HOLD [C]", sx, 15, {150,150,170,255});
        if (has_hold) {
            for (int i = 0; i < 4; i++) {
                int bx = sx + SHAPES[hold_piece.idx][0][i][0] * 20;
                int by = 42 + SHAPES[hold_piece.idx][0][i][1] * 20;
                SDL_Color hc = hold_used ? SDL_Color{80,80,80,255} : COLORS[hold_piece.idx];
                SDL_Rect r = {bx, by, 18, 18};
                SDL_SetRenderDrawColor(ren, hc.r, hc.g, hc.b, 255);
                SDL_RenderFillRect(ren, &r);
            }
        }

        // ── Наступна ─────
        drawText(ren, small_font, "\xD0\x9D\xD0\x90\xD0\xA1\xD0\xA2\xD0\xA3\xD0\x9F\xD0\x9D\xD0\x90", sx, 130, {180,180,200,255});
        for (int i = 0; i < 4; i++) {
            int bx = sx + SHAPES[nxt.idx][0][i][0] * 22;
            int by = 155 + SHAPES[nxt.idx][0][i][1] * 22;
            SDL_Rect r = {bx, by, 20, 20};
            SDL_SetRenderDrawColor(ren, COLORS[nxt.idx].r, COLORS[nxt.idx].g, COLORS[nxt.idx].b, 255);
            SDL_RenderFillRect(ren, &r);
        }

        // ── Рахунок ──────
        int yoff = 260;
        const char* labels[] = {
            "\xD0\xA0\xD0\x90\xD0\xA5\xD0\xA3\xD0\x9D\xD0\x9E\xD0\x9A",
            "\xD0\x9B\xD0\x86\xD0\x9D\xD0\x86\xD0\x87",
            "\xD0\xA0\xD0\x86\xD0\x92\xD0\x95\xD0\x9D\xD0\xAC"
        };
        int values[] = {score, lines, level};
        for (int i = 0; i < 3; i++) {
            drawText(ren, small_font, labels[i], sx, yoff, {150,150,170,255});
            char buf[32]; snprintf(buf, sizeof(buf), "%d", values[i]);
            drawText(ren, font, buf, sx, yoff + 22, {255,255,255,255});
            yoff += 55;
        }

        // ── Комбо ────────
        if (combo > 1 && combo_display_timer > 0) {
            char buf[32]; snprintf(buf, sizeof(buf), "COMBO x%d", combo);
            SDL_Color cc = {255, (Uint8)(200 - combo * 20), 0, 255};
            drawText(ren, font, buf, sx, yoff, cc);
            yoff += 30;
        }

        // ── Tetris count ─
        if (total_tetris > 0) {
            char buf[32]; snprintf(buf, sizeof(buf), "TETRIS: %d", total_tetris);
            drawText(ren, small_font, buf, sx, yoff + 5, {255, 220, 50, 255});
        }

        // ── Max combo ────
        if (max_combo > 1) {
            char buf[32]; snprintf(buf, sizeof(buf), "MAX COMBO: %d", max_combo);
            drawText(ren, small_font, buf, sx, yoff + 30, {100, 200, 255, 255});
        }

        // ── Керування ────
        yoff = H - 170;
        const char* controls[] = {
            "\xE2\x86\x90 \xE2\x86\x92  \xD1\x80\xD1\x83\xD1\x85",
            "\xE2\x86\x91     \xD0\xBE\xD0\xB1\xD0\xB5\xD1\x80\xD1\x82",
            "\xE2\x86\x93     \xD0\xBC'\xD1\x8F\xD0\xBA\xD0\xB5",
            "SPACE \xD0\xB4\xD1\x80\xD0\xBE\xD0\xBF",
            "C     hold",
            "P     \xD0\xBF\xD0\xB0\xD1\x83\xD0\xB7\xD0\xB0",
        };
        for (auto line : controls) {
            drawText(ren, small_font, line, sx, yoff, {100,100,120,255});
            yoff += 22;
        }

        // Автор
        drawText(ren, small_font, "\xC2\xA9 sanyarud", sx, H - 28, {80,80,100,255});

        // ── Level Up! ────
        if (level_up_timer > 0) {
            SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
            float alpha = std::min(1.0f, level_up_timer);
            char buf[32]; snprintf(buf, sizeof(buf),
                "\xD0\xA0\xD0\x86\xD0\x92\xD0\x95\xD0\x9D\xD0\xAC %d!", level);
            SDL_Color lc = {100, 255, 100, (Uint8)(alpha * 255)};
            drawTextCentered(ren, big_font, buf, CELL*COLS/2 + ox, H/2 - 80 + oy, lc);
        }

        // ── Game Over ────
        if (over) {
            SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 180);
            SDL_Rect ov = {0, 0, CELL*COLS, H};
            SDL_RenderFillRect(ren, &ov);

            int cx = CELL*COLS/2;
            drawTextCentered(ren, big_font,
                "\xD0\x93\xD0\xA0\xD0\xA3 \xD0\x97\xD0\x90\xD0\x9A\xD0\x86\xD0\x9D\xD0\xA7\xD0\x95\xD0\x9D\xD0\x9E",
                cx, H/2-80, {255,60,60,255});

            char buf[64];
            snprintf(buf, sizeof(buf), "\xD0\xA0\xD0\xB0\xD1\x85\xD1\x83\xD0\xBD\xD0\xBE\xD0\xBA: %d", score);
            drawTextCentered(ren, font, buf, cx, H/2-20, {255,255,255,255});

            snprintf(buf, sizeof(buf), "\xD0\x9B\xD1\x96\xD0\xBD\xD1\x96\xD0\xB9: %d  \xD0\xA0\xD1\x96\xD0\xB2\xD0\xB5\xD0\xBD\xD1\x8C: %d", lines, level);
            drawTextCentered(ren, small_font, buf, cx, H/2+15, {200,200,200,255});

            if (max_combo > 1) {
                snprintf(buf, sizeof(buf), "Max Combo: %d", max_combo);
                drawTextCentered(ren, small_font, buf, cx, H/2+45, {100,200,255,255});
            }
            if (total_tetris > 0) {
                snprintf(buf, sizeof(buf), "Tetris: %d", total_tetris);
                drawTextCentered(ren, small_font, buf, cx, H/2+70, {255,220,50,255});
            }

            drawTextCentered(ren, small_font,
                "R \xE2\x80\x94 \xD1\x80\xD0\xB5\xD1\x81\xD1\x82\xD0\xB0\xD1\x80\xD1\x82   ESC \xE2\x80\x94 \xD0\xB2\xD0\xB8\xD1\x85\xD1\x96\xD0\xB4",
                cx, H/2+110, {255,255,255,255});
        }

        // ── Пауза ────────
        if (paused && !over) {
            SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 180);
            SDL_Rect ov = {0, 0, CELL*COLS, H};
            SDL_RenderFillRect(ren, &ov);
            drawTextCentered(ren, big_font,
                "\xD0\x9F\xD0\x90\xD0\xA3\xD0\x97\xD0\x90",
                CELL*COLS/2, H/2-20, {255,255,255,255});
        }

        SDL_RenderPresent(ren);
    }

private:
    void drawBlock(SDL_Renderer* ren, int x, int y, SDL_Color col, SDL_Color dark, SDL_Color glow) {
        int bevel = CELL / 5;
        // Тінь
        SDL_Rect shadow = {x+2, y+2, CELL-2, CELL-2};
        SDL_SetRenderDrawColor(ren, dark.r/2, dark.g/2, dark.b/2, 255);
        SDL_RenderFillRect(ren, &shadow);

        // Основний блок
        SDL_Rect r = {x+1, y+1, CELL-2, CELL-2};
        SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, 255);
        SDL_RenderFillRect(ren, &r);

        // Верхній блиск (градієнт)
        SDL_SetRenderDrawColor(ren, glow.r, glow.g, glow.b, 255);
        SDL_Rect hl_top = {x+1, y+1, CELL-2, bevel};
        SDL_RenderFillRect(ren, &hl_top);
        SDL_Rect hl_left = {x+1, y+1, bevel, CELL-2};
        SDL_RenderFillRect(ren, &hl_left);

        // Нижня тінь
        SDL_SetRenderDrawColor(ren, dark.r, dark.g, dark.b, 255);
        SDL_Rect sh_bottom = {x+1, y+CELL-bevel-1, CELL-2, bevel};
        SDL_RenderFillRect(ren, &sh_bottom);
        SDL_Rect sh_right = {x+CELL-bevel-1, y+1, bevel, CELL-2};
        SDL_RenderFillRect(ren, &sh_right);

        // Маленький блиск
        SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ren, 255, 255, 255, 80);
        SDL_Rect shine = {x+3, y+3, CELL/3, CELL/3};
        SDL_RenderFillRect(ren, &shine);
    }

    void drawText(SDL_Renderer* ren, TTF_Font* f, const char* text, int x, int y, SDL_Color col) {
        SDL_Surface* s = TTF_RenderUTF8_Blended(f, text, col);
        if (!s) return;
        SDL_Texture* t = SDL_CreateTextureFromSurface(ren, s);
        SDL_Rect dst = {x, y, s->w, s->h};
        SDL_RenderCopy(ren, t, nullptr, &dst);
        SDL_DestroyTexture(t);
        SDL_FreeSurface(s);
    }

    void drawTextCentered(SDL_Renderer* ren, TTF_Font* f, const char* text, int cx, int y, SDL_Color col) {
        SDL_Surface* s = TTF_RenderUTF8_Blended(f, text, col);
        if (!s) return;
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

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL Init: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() < 0) {
        fprintf(stderr, "TTF Init: %s\n", TTF_GetError());
        return 1;
    }
    if (Mix_OpenAudio(44100, AUDIO_S16SYS, 1, 2048) < 0) {
        fprintf(stderr, "Mixer Init: %s\n", Mix_GetError());
        return 1;
    }
    Mix_AllocateChannels(16);

    SDL_Window* win = SDL_CreateWindow(
        "\xD0\xA2\xD0\xB5\xD1\x82\xD1\x80\xD1\x96\xD1\x81",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W, H, 0);
    if (!win) { fprintf(stderr, "Window: %s\n", SDL_GetError()); return 1; }

    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font* font      = TTF_OpenFont("/System/Library/Fonts/Helvetica.ttc", 22);
    TTF_Font* big_font  = TTF_OpenFont("/System/Library/Fonts/Helvetica.ttc", 36);
    TTF_Font* small_font= TTF_OpenFont("/System/Library/Fonts/Helvetica.ttc", 16);

    Tetris game;
    Uint32 last = SDL_GetTicks();

    while (true) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) goto quit;
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) goto quit;
                if (e.key.keysym.sym == SDLK_r && game.isOver()) {
                    game.reset(); last = SDL_GetTicks(); continue;
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
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
