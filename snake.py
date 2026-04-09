import pygame
import random
import sys

# Ініціалізація
pygame.init()

# Кольори
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
GREEN = (0, 200, 0)
DARK_GREEN = (0, 150, 0)
RED = (200, 0, 0)
GRAY = (40, 40, 40)

# Налаштування екрану
CELL_SIZE = 20
GRID_W = 30
GRID_H = 20
WIDTH = CELL_SIZE * GRID_W
HEIGHT = CELL_SIZE * GRID_H
FPS = 10

screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Змійка")
clock = pygame.time.Clock()
font = pygame.font.SysFont("Arial", 25, bold=True)
big_font = pygame.font.SysFont("Arial", 40, bold=True)


def new_food(snake):
    while True:
        pos = (random.randint(0, GRID_W - 1), random.randint(0, GRID_H - 1))
        if pos not in snake:
            return pos


def draw(snake, food, score, best):
    screen.fill(BLACK)

    # Сітка
    for x in range(0, WIDTH, CELL_SIZE):
        pygame.draw.line(screen, GRAY, (x, 0), (x, HEIGHT))
    for y in range(0, HEIGHT, CELL_SIZE):
        pygame.draw.line(screen, GRAY, (0, y), (WIDTH, y))

    # Їжа
    fx, fy = food
    pygame.draw.rect(screen, RED, (fx * CELL_SIZE, fy * CELL_SIZE, CELL_SIZE, CELL_SIZE), border_radius=4)

    # Змійка
    for i, (x, y) in enumerate(snake):
        color = DARK_GREEN if i == 0 else GREEN
        pygame.draw.rect(screen, color, (x * CELL_SIZE + 1, y * CELL_SIZE + 1, CELL_SIZE - 2, CELL_SIZE - 2), border_radius=3)

    # Рахунок
    text = font.render(f"Рахунок: {score}  Найкращий: {best}", True, WHITE)
    screen.blit(text, (10, 5))

    pygame.display.flip()


def show_game_over(score, best):
    screen.fill(BLACK)
    t1 = big_font.render("ГРУ ЗАКІНЧЕНО", True, RED)
    t2 = font.render(f"Рахунок: {score}  Найкращий: {best}", True, WHITE)
    t3 = font.render("Натисніть SPACE щоб грати знову або ESC щоб вийти", True, WHITE)
    screen.blit(t1, (WIDTH // 2 - t1.get_width() // 2, HEIGHT // 2 - 60))
    screen.blit(t2, (WIDTH // 2 - t2.get_width() // 2, HEIGHT // 2))
    screen.blit(t3, (WIDTH // 2 - t3.get_width() // 2, HEIGHT // 2 + 50))
    pygame.display.flip()

    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_SPACE:
                    return True
                if event.key == pygame.K_ESCAPE:
                    return False


def game():
    best = 0

    while True:
        snake = [(GRID_W // 2, GRID_H // 2)]
        direction = (1, 0)
        next_dir = direction
        food = new_food(snake)
        score = 0

        while True:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    pygame.quit()
                    sys.exit()
                if event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_UP and direction != (0, 1):
                        next_dir = (0, -1)
                    elif event.key == pygame.K_DOWN and direction != (0, -1):
                        next_dir = (0, 1)
                    elif event.key == pygame.K_LEFT and direction != (1, 0):
                        next_dir = (-1, 0)
                    elif event.key == pygame.K_RIGHT and direction != (-1, 0):
                        next_dir = (1, 0)
                    elif event.key == pygame.K_ESCAPE:
                        pygame.quit()
                        sys.exit()

            direction = next_dir
            hx, hy = snake[0]
            new_head = (hx + direction[0], hy + direction[1])

            # Перевірка зіткнення зі стінами або собою
            if (new_head[0] < 0 or new_head[0] >= GRID_W or
                    new_head[1] < 0 or new_head[1] >= GRID_H or
                    new_head in snake):
                best = max(best, score)
                if not show_game_over(score, best):
                    pygame.quit()
                    sys.exit()
                break

            snake.insert(0, new_head)

            if new_head == food:
                score += 1
                food = new_food(snake)
            else:
                snake.pop()

            draw(snake, food, score, best)
            clock.tick(FPS)


if __name__ == "__main__":
    game()