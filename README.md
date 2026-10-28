# MatronixLauncher

Лаунчер для Minecraft на базе кода ElyPrismLauncher (форк Prism Launcher) с интеграцией **ely.by**.

## Возможности
- **Все версии Minecraft** — Vanilla, Fabric, Forge
- **Скачивание модов** — Modrinth и CurseForge API
- **Регистрация и вход через ely.by** — Auth API (с поддержкой 2FA)
- **Анимированный интерфейс** — тёмная тема в стиле ElyPrismLauncher
- **EXE инсталлятор** — собирается через GitHub Actions (Windows) и выкладывается в релизы
- **Работа в России и во всём мире**

## Сборка

### Локально (требуется Qt 6 + CMake)
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Windows EXE (GitHub Actions)
Запушить в репозиторий — workflow `.github/workflows/windows-release.yml` соберёт EXE и создаст релиз.

## Иконка
Иконка взята из ElyPrismLauncher (`elyprismlauncher.ico`) и перекрашена в чёрно-белый (grayscale).

## Авторизация ely.by
Используется `https://authserver.ely.by/auth/authenticate` (логин/пароль, 2FA через токен).
