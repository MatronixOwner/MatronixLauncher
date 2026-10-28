# MatronixLauncher

Лаунчер для Minecraft с поддержкой **Ely.by**, всех версий игры, модов и анимированным интерфейсом.

## Возможности
- **Все версии Minecraft** — Vanilla (Release), Fabric, Forge
- **Моды** — скачивание с Modrinth API (CurseForge требует API ключ)
- **Вход через ely.by** — Auth API с поддержкой 2FA, сохранение сессии
- **Анимированный интерфейс** — тёмная тема, плавные переходы
- **EXE инсталлятор** — собирается через GitHub Actions (Windows) и выкладывается в релизы
- **Работа в России и во всём мире**

## Установка для разработки
```bash
npm ci
npm start
```

## Сборка EXE
```bash
npm run dist
```
Или запуште тэг `v*` — GitHub Actions соберёт EXE и создаст релиз:
```bash
git tag v1.0.0 && git push origin v1.0.0
```

## Иконка
Чёрно-белая иконка (`assets/icon.ico`) перекрашена из иконки ElyPrismLauncher.

## Авторизация
Используется `https://authserver.ely.by/auth/authenticate` (ely.by).
