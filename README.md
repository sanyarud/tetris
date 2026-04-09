# 🎮 Тетріс

Класична гра Тетріс написана на C++ з використанням SDL2.

![C++](https://img.shields.io/badge/C++-17-blue.svg)
![SDL2](https://img.shields.io/badge/SDL2-2.0+-green.svg)
![Platform](https://img.shields.io/badge/Platform-macOS-lightgrey.svg)

## Скріншот

![Тетріс](screenshot.png)

## Можливості

- 7 кольорових фігур з обертанням та wall kick
- Тінь падіння (привід) — бачити куди впаде фігура
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

## Збірка

```bash
brew install sdl2 sdl2_ttf
cd tetris_cpp
make
```

## macOS Додаток

Завантажте `Тетріс.app` з [останнього релізу](https://github.com/sanyarud/tetris/releases/latest).

При першому запуску macOS може заблокувати додаток. Вирішення:

```bash
xattr -cr /Applications/Тетріс.app
```

## Автор

© sanyarud

## Ліцензія

MIT
