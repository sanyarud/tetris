import pygame
import random
import sys

# Ініціалізація
pygame.init()

# Кольори фігур
COLORS = [
    (0, 220, 220),   # I — ціан
    (220, 220, 0),   # O — жовтий
    (160, 0, 220),   # T — фіолетовий
    (0, 220, 0),     # S — зелений
    (220, 0, 0),     # Z — червоний
    (0, 0, 220),     # J — синій
    (220, 150, 0),   # L — помаранчевий
]

DARK_COLORS = [tuple(max(0, c - 60) for c in col) for col in COLORS]

# Фігури (4 обертання кожна)
SHAPES = [
    # I
    [[(0,1),(1,1),(2,1),(3,1)],
     [(2,0),(2,1),(2,2),(2,3)],
     [(0,2),(1,2),(2,2),(3,2)],
     [(1,0),(1,1),(1,2),(1,3)]],
    # O
    [[(1,0),(2,0),(1,1),(2,1)],
     [(1,0),(2,0),(1,1),(2,1)],
     [(1,0),(2,0),(1,1),(2,1)],
     [(1,0),(2,0),(1,1),(2,1)]],
    # T
    [[(1,0),(0,1),(1,1),(2,1)],
     [(1,0),(1,1),(2,1),(1,2)],
     [(0,1),(1,1),(2,1),(1,2)],
     [(1,0),(0,1),(1,1),(1,2)]],
    # S
    [[(1,0),(2,0),(0,1),(1,1)],
     [(1,0),(1,1),(2,1),(2,2)],
     [(1,1),(2,1),(0,2),(1,2)],
     [(0,0),(0,1),(1,1),(1,2)]],
    # Z
    [[(0,0),(1,0),(1,1),(2,1)],
     [(2,0),(1,1),(2,1),(1,2)],
     [(0,1),(1,1),(1,2),(2,2)],
     [(1,0),(0,1),(1,1),(0,2)]],
    # J
    [[(0,0),(0,1),(1,1),(2,1)],
     [(1,0),(2,0),(1,1),(1,2)],
     [(0,1),(1,1),(2,1),(2,2)],
     [(1,0),(1,1),(0,2),(1,2)]],
    # L
    [[(2,0),(0,1),(1,1),(2,1)],
     [(1,0),(1,1),(1,2),(2,2)],
     [(0,1),(1,1),(2,1),(0,2)],
     [(0,0),(1,0),(1,1),(1,2)]],
]

# Налаштування
CELL = 30
COLS = 10
ROWS = 20
SIDEBAR = 180
WIDTH = CELL * COLS + SIDEBAR
HEIGHT = CELL * ROWS
FPS = 60

screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Тетріс")
clock = pygame.time.Clock()
font = pygame.font.SysFont("Arial", 22, bold=True)
big_font = pygame.font.SysFont("Arial", 36, bold=True)
small_font = pygame.font.SysFont("Arial", 18)

BLACK = (0, 0, 0)
GRAY = (50, 50, 50)
WHITE = (255, 255, 255)
DARK_BG = (20, 20, 30)


class Piece:
    def __init__(self, idx=None):
        self.idx = idx if idx is not None else random.randint(0, len(SHAPES) - 1)
        self.rot = 0
        self.x = COLS // 2 - 2
        self.y = 0

    def cells(self, ox=0, oy=0, rot=None):
        r = rot if rot is not None else self.rot
        return [(self.x + cx + ox, self.y + cy + oy) for cx, cy in SHAPES[self.idx][r % len(SHAPES[self.idx])]]

    def color(self):
        return COLORS[self.idx]

    def dark_color(self):
        return DARK_COLORS[self.idx]


class Tetris:
    def __init__(self):
        self.board = [[None] * COLS for _ in range(ROWS)]
        self.piece = Piece()
        self.next = Piece()
        self.score = 0
        self.lines = 0
        self.level = 1
        self.game_over = False
        self.fall_timer = 0
        self.fall_speed = 500  # мс

    def valid(self, piece, ox=0, oy=0, rot=None):
        for x, y in piece.cells(ox, oy, rot):
            if x < 0 or x >= COLS or y >= ROWS:
                return False
            if y >= 0 and self.board[y][x] is not None:
                return False
        return True

    def lock(self):
        for x, y in self.piece.cells():
            if y < 0:
                self.game_over = True
                return
            self.board[y][x] = self.piece.idx

        # Перевірка заповнених ліній
        full = [r for r in range(ROWS) if all(self.board[r][c] is not None for c in range(COLS))]
        for r in sorted(full):
            del self.board[r]
            self.board.insert(0, [None] * COLS)

        cleared = len(full)
        points = {0: 0, 1: 100, 2: 300, 3: 500, 4: 800}
        self.score += points.get(cleared, 800) * self.level
        self.lines += cleared
        self.level = self.lines // 10 + 1
        self.fall_speed = max(50, 500 - (self.level - 1) * 40)

        self.piece = self.next
        self.next = Piece()
        if not self.valid(self.piece):
            self.game_over = True

    def ghost_y(self):
        oy = 0
        while self.valid(self.piece, oy=oy + 1):
            oy += 1
        return oy

    def hard_drop(self):
        oy = self.ghost_y()
        self.score += oy * 2
        self.piece.y += oy
        self.lock()

    def draw(self):
        screen.fill(DARK_BG)

        # Поле
        for r in range(ROWS):
            for c in range(COLS):
                x = c * CELL
                y = r * CELL
                pygame.draw.rect(screen, (30, 30, 40), (x, y, CELL, CELL))
                pygame.draw.rect(screen, GRAY, (x, y, CELL, CELL), 1)
                if self.board[r][c] is not None:
                    idx = self.board[r][c]
                    pygame.draw.rect(screen, COLORS[idx], (x + 1, y + 1, CELL - 2, CELL - 2), border_radius=3)
                    # Блиск
                    pygame.draw.rect(screen, tuple(min(255, c + 60) for c in COLORS[idx]),
                                     (x + 3, y + 3, CELL // 3, CELL // 3), border_radius=2)

        # Привид (тінь падіння)
        if not self.game_over:
            gy = self.ghost_y()
            for cx, cy in self.piece.cells(oy=gy):
                if cy >= 0:
                    x = cx * CELL
                    y = cy * CELL
                    pygame.draw.rect(screen, (80, 80, 80), (x + 1, y + 1, CELL - 2, CELL - 2), 2, border_radius=3)

            # Поточна фігура
            for cx, cy in self.piece.cells():
                if cy >= 0:
                    x = cx * CELL
                    y = cy * CELL
                    pygame.draw.rect(screen, self.piece.color(), (x + 1, y + 1, CELL - 2, CELL - 2), border_radius=3)
                    pygame.draw.rect(screen, tuple(min(255, c + 60) for c in self.piece.color()),
                                     (x + 3, y + 3, CELL // 3, CELL // 3), border_radius=2)

        # Бічна панель
        sx = CELL * COLS + 15
        # Рамка
        pygame.draw.line(screen, GRAY, (CELL * COLS, 0), (CELL * COLS, HEIGHT), 2)

        # Наступна фігура
        t = font.render("НАСТУПНА", True, WHITE)
        screen.blit(t, (sx, 15))
        for cx, cy in SHAPES[self.next.idx][0]:
            px = sx + cx * 22
            py = 50 + cy * 22
            pygame.draw.rect(screen, COLORS[self.next.idx], (px, py, 20, 20), border_radius=2)

        # Рахунок
        y_off = 170
        labels = [
            ("РАХУНОК", str(self.score)),
            ("ЛІНІЇ", str(self.lines)),
            ("РІВЕНЬ", str(self.level)),
        ]
        for label, value in labels:
            t = small_font.render(label, True, (180, 180, 180))
            screen.blit(t, (sx, y_off))
            t = font.render(value, True, WHITE)
            screen.blit(t, (sx, y_off + 22))
            y_off += 60

        # Керування
        y_off = HEIGHT - 140
        controls = [
            "← →  рух",
            "↑     обертання",
            "↓     м'яке падіння",
            "SPACE хард дроп",
            "P     пауза",
        ]
        for line in controls:
            t = small_font.render(line, True, (120, 120, 140))
            screen.blit(t, (sx, y_off))
            y_off += 24

        if self.game_over:
            overlay = pygame.Surface((CELL * COLS, HEIGHT), pygame.SRCALPHA)
            overlay.fill((0, 0, 0, 150))
            screen.blit(overlay, (0, 0))
            t1 = big_font.render("ГРУ ЗАКІНЧЕНО", True, (255, 60, 60))
            t2 = font.render(f"Рахунок: {self.score}", True, WHITE)
            t3 = small_font.render("R — рестарт   ESC — вихід", True, WHITE)
            screen.blit(t1, (CELL * COLS // 2 - t1.get_width() // 2, HEIGHT // 2 - 50))
            screen.blit(t2, (CELL * COLS // 2 - t2.get_width() // 2, HEIGHT // 2 + 10))
            screen.blit(t3, (CELL * COLS // 2 - t3.get_width() // 2, HEIGHT // 2 + 50))

        pygame.display.flip()


def main():
    game = Tetris()
    paused = False

    while True:
        dt = clock.tick(FPS)

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()

            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    pygame.quit()
                    sys.exit()

                if game.game_over:
                    if event.key == pygame.K_r:
                        game = Tetris()
                    continue

                if event.key == pygame.K_p:
                    paused = not paused
                    continue

                if paused:
                    continue

                if event.key == pygame.K_LEFT:
                    if game.valid(game.piece, ox=-1):
                        game.piece.x -= 1
                elif event.key == pygame.K_RIGHT:
                    if game.valid(game.piece, ox=1):
                        game.piece.x += 1
                elif event.key == pygame.K_DOWN:
                    if game.valid(game.piece, oy=1):
                        game.piece.y += 1
                        game.score += 1
                elif event.key == pygame.K_UP:
                    new_rot = (game.piece.rot + 1) % len(SHAPES[game.piece.idx])
                    if game.valid(game.piece, rot=new_rot):
                        game.piece.rot = new_rot
                    # Wall kick спроба
                    elif game.valid(game.piece, ox=-1, rot=new_rot):
                        game.piece.x -= 1
                        game.piece.rot = new_rot
                    elif game.valid(game.piece, ox=1, rot=new_rot):
                        game.piece.x += 1
                        game.piece.rot = new_rot
                    elif game.valid(game.piece, ox=-2, rot=new_rot):
                        game.piece.x -= 2
                        game.piece.rot = new_rot
                    elif game.valid(game.piece, ox=2, rot=new_rot):
                        game.piece.x += 2
                        game.piece.rot = new_rot
                elif event.key == pygame.K_SPACE:
                    game.hard_drop()

        if not game.game_over and not paused:
            game.fall_timer += dt
            if game.fall_timer >= game.fall_speed:
                game.fall_timer = 0
                if game.valid(game.piece, oy=1):
                    game.piece.y += 1
                else:
                    game.lock()

        game.draw()

        if paused and not game.game_over:
            overlay = pygame.Surface((CELL * COLS, HEIGHT), pygame.SRCALPHA)
            overlay.fill((0, 0, 0, 150))
            screen.blit(overlay, (0, 0))
            t = big_font.render("ПАУЗА", True, WHITE)
            screen.blit(t, (CELL * COLS // 2 - t.get_width() // 2, HEIGHT // 2 - 20))
            pygame.display.flip()


if __name__ == "__main__":
    main()