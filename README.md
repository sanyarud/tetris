# 🎮 Тетріс

Класична гра Тетріс написана на Python з використанням Pygame.

![Python](https://img.shields.io/badge/Python-3.9+-blue.svg)
![Pygame](https://img.shields.io/badge/Pygame-2.6+-green.svg)
![Platform](https://img.shields.io/badge/Platform-macOS-lightgrey.svg)

## Скріншот

![Тетріс](screenshot.png)

## Можливості

- 7 кольорових фігур з обертанням та wall kick
- Тінь падіння (привид) — бачити куди впаде фігура
- Попередній перегляд наступної фігури
- Зростаюча складність — швидкість збільшується з рівнем
- Підрахунок очок: 100/300/500/800 за 1/2/3/4 лінії
- Пауза та рестарт
- macOS .app додаток

## Керування

| Клавіша      | Дія              |
|-------------|------------------|
| ← →         | Рух ліворуч/праворуч |
| ↑           | Обертання        |
| ↓           | М'яке падіння    |
| SPACE       | Хард дроп        |
| P           | Пауза            |
| R           | Рестарт          |
| ESC         | Вихід            |

## Встановлення

```bash
pip3 install pygame
```

## Запуск

```bash
python3 tetris.py
```

## macOS Додаток

Завантажте `Тетріс.app` з [останнього релізу](https://github.com/sanyarud/tetris/releases/latest).

При першому запуску macOS може заблокувати додаток. Вирішення:

```bash
xattr -cr /Applications/Тетріс.app
```

## Ліцензія

MIT