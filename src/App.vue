<script setup>
import { computed, onMounted, ref } from 'vue';

const activePage = ref('Главная');
const activeTab = ref('Modrinth');
const search = ref('');
const loading = ref(false);
const error = ref('');
const projects = ref([]);
const toast = ref('');
const selectedLoader = ref('Fabric');
const selectedVersion = ref('1.21.1');
const curseKey = ref(localStorage.getItem('elyprism-cf-key') || '');
const keyDraft = ref(curseKey.value);
const showKeyDialog = ref(false);
const installed = ref([]);
const selectedInstance = ref('Prism Survival');
const navItems = [
  { label: 'Главная', icon: '⌂' },
  { label: 'Каталог модов', icon: '◈' },
  { label: 'Сборки', icon: '▦' },
  { label: 'Библиотека', icon: '▤' }
];

const filteredProjects = computed(() => projects.value);
const routeTitle = computed(() => activePage.value === 'Каталог модов' ? 'Открой новые миры' : activePage.value);

function notify(message) {
  toast.value = message;
  window.clearTimeout(notify.timer);
  notify.timer = window.setTimeout(() => (toast.value = ''), 3200);
}

async function loadModrinth() {
  loading.value = true;
  error.value = '';
  try {
    const params = new URLSearchParams({
      limit: '8',
      index: search.value.trim() ? 'relevance' : 'downloads',
      facets: JSON.stringify([['project_type:mod']])
    });
    if (search.value.trim()) params.set('query', search.value.trim());
    const response = await fetch(`https://api.modrinth.com/v2/search?${params}`);
    if (!response.ok) throw new Error(`Modrinth ответил кодом ${response.status}`);
    const data = await response.json();
    projects.value = data.hits || [];
  } catch (e) {
    error.value = 'Не удалось подключиться к Modrinth. Проверь подключение к интернету и попробуй ещё раз.';
    projects.value = [];
  } finally {
    loading.value = false;
  }
}

async function loadCurseForge() {
  if (!curseKey.value) {
    projects.value = [];
    showKeyDialog.value = true;
    return;
  }
  loading.value = true;
  error.value = '';
  try {
    const url = new URL('https://api.curseforge.com/v1/mods/search');
    url.searchParams.set('gameId', '432');
    url.searchParams.set('classId', '6');
    url.searchParams.set('pageSize', '8');
    url.searchParams.set('sortField', '2');
    url.searchParams.set('sortOrder', 'desc');
    if (search.value.trim()) url.searchParams.set('searchFilter', search.value.trim());
    const response = await fetch(url, { headers: { 'x-api-key': curseKey.value, 'Accept': 'application/json' } });
    if (!response.ok) throw new Error(`CurseForge ответил кодом ${response.status}`);
    const data = await response.json();
    projects.value = (data.data || []).map((m) => ({
      project_id: String(m.id), title: m.name, description: m.summary,
      icon_url: m.logo?.url, downloads: m.downloadCount, slug: m.slug,
      source: 'curseforge'
    }));
  } catch (e) {
    error.value = 'Не удалось загрузить CurseForge. Проверь API-ключ и доступность сервиса в твоём регионе.';
    projects.value = [];
  } finally {
    loading.value = false;
  }
}

function loadCatalog() { return activeTab.value === 'Modrinth' ? loadModrinth() : loadCurseForge(); }
function switchTab(tab) { activeTab.value = tab; loadCatalog(); }
function saveKey() {
  curseKey.value = keyDraft.value.trim();
  localStorage.setItem('elyprism-cf-key', curseKey.value);
  showKeyDialog.value = false;
  if (activeTab.value === 'CurseForge') loadCurseForge();
  notify('Настройки CurseForge сохранены');
}
function openProject(project) {
  const url = project.source === 'curseforge'
    ? `https://www.curseforge.com/minecraft/mc-mods/${project.slug}`
    : `https://modrinth.com/mod/${project.slug}`;
  if (window.elyprism?.desktop) window.elyprism.openExternal(url);
  else window.open(url, '_blank', 'noopener');
}
function installProject(project) {
  if (!installed.value.includes(project.project_id)) installed.value.push(project.project_id);
  notify(`«${project.title}» добавлен в очередь установки для ${selectedInstance.value}`);
}
function launchGame() {
  notify('Игровой движок ещё не подключён. Сейчас готовится интерфейс лаунчера.');
}
function createInstance() { notify('Создание профилей появится в следующем этапе разработки'); }
function setPage(page) {
  activePage.value = page;
  if (page === 'Каталог модов') loadCatalog();
}
function prettyDownloads(n = 0) {
  if (n >= 1_000_000) return `${(n / 1_000_000).toFixed(1)} млн`;
  if (n >= 1000) return `${(n / 1000).toFixed(0)} тыс.`;
  return String(n);
}
onMounted(() => loadModrinth());
</script>

<template>
  <div class="app-shell">
    <aside class="sidebar">
      <div class="brand"><div class="brand-mark"><span></span><span></span><span></span></div><div><b>ELYPRISM</b><small>LAUNCHER</small></div></div>
      <div class="side-label">МЕНЮ</div>
      <nav class="main-nav">
        <button v-for="item in navItems" :key="item.label" :class="['nav-item', { active: activePage === item.label }]" @click="setPage(item.label)">
          <span class="nav-icon">{{ item.icon }}</span><span>{{ item.label }}</span><span v-if="item.label === 'Каталог модов'" class="nav-dot"></span>
        </button>
      </nav>
      <div class="side-divider"></div>
      <div class="side-label">ТВОЯ СБОРКА</div>
      <button class="instance-card" @click="activePage = 'Библиотека'">
        <div class="instance-thumb"><span>✦</span></div>
        <div class="instance-info"><b>Prism Survival</b><small>1.21.1 · Fabric</small></div><span class="online-dot"></span>
      </button>
      <button class="add-instance" @click="createInstance"><span>＋</span> Новая сборка</button>
      <div class="sidebar-bottom"><div class="avatar">E</div><div class="user-meta"><b>Исследователь</b><small>Локальный профиль</small></div><button class="more-btn" title="Настройки" @click="setPage('Настройки')">•••</button></div>
    </aside>

    <main class="main-area">
      <header class="topbar">
        <div class="breadcrumbs"><span>ELYPRISM</span><i>/</i><b>{{ routeTitle }}</b></div>
        <div class="top-actions"><button class="icon-button" title="Уведомления" @click="notify('Новых уведомлений нет')">♧<span class="notification-dot"></span></button><div class="top-separator"></div><button class="account-button"><span class="tiny-avatar">E</span><span>Мой профиль</span><span class="chevron">⌄</span></button></div>
      </header>

      <section v-if="activePage === 'Главная'" class="content home-content">
        <div class="welcome-row"><div><div class="eyebrow"><span class="sparkle">✦</span> ТВОЁ ПРИКЛЮЧЕНИЕ НАЧИНАЕТСЯ ЗДЕСЬ</div><h1>Твой Minecraft.<br><span>Твои правила.</span></h1><p class="hero-copy">Собирай игру под себя — от чистой ванилы до бесконечных миров с модами.</p></div><div class="hero-orbit"><div class="orbit-ring ring-one"></div><div class="orbit-ring ring-two"></div><div class="crystal">✦</div><div class="orbit-dot dot-a"></div><div class="orbit-dot dot-b"></div><div class="orbit-dot dot-c"></div></div></div>
        <div class="launch-panel"><div class="launch-instance"><div class="block-icon">▧</div><div><small>ГОТОВО К ЗАПУСКУ</small><b>Prism Survival</b><span>Minecraft {{ selectedVersion }} <i>·</i> {{ selectedLoader }}</span></div></div><div class="launch-meta"><div><small>ВЕРСИЯ</small><select v-model="selectedVersion"><option>1.21.1</option><option>1.20.1</option><option>1.19.2</option><option>1.18.2</option></select></div><div class="meta-divider"></div><div><small>ЗАГРУЗЧИК</small><select v-model="selectedLoader"><option>Fabric</option><option>Forge</option><option>Vanilla</option></select></div><button class="play-button" @click="launchGame"><span>▶</span> Играть</button></div></div>

        <div class="section-heading"><div><div class="eyebrow muted">ТВОЯ ИГРА — ТВОЙ ВЫБОР</div><h2>Начни с основы</h2></div><button class="text-link" @click="setPage('Библиотека')">Все профили <span>↗</span></button></div>
        <div class="choice-grid">
          <button class="choice-card vanilla" @click="selectedLoader = 'Vanilla'; notify('Выбрана чистая Vanilla')"><div class="choice-glow"></div><div class="choice-icon">▧</div><div class="choice-copy"><span>01 / ОРИГИНАЛ</span><h3>Vanilla</h3><p>Чистая игра. Ничего лишнего.</p></div><span class="choice-arrow">↗</span></button>
          <button class="choice-card fabric" @click="selectedLoader = 'Fabric'; notify('Выбран Fabric')"><div class="choice-glow"></div><div class="choice-icon">✳</div><div class="choice-copy"><span>02 / ЛЁГКИЙ И БЫСТРЫЙ</span><h3>Fabric</h3><p>Лёгкие моды и высокая скорость.</p></div><span class="choice-arrow">↗</span></button>
          <button class="choice-card forge" @click="selectedLoader = 'Forge'; notify('Выбран Forge')"><div class="choice-glow"></div><div class="choice-icon">⚒</div><div class="choice-copy"><span>03 / БОЛЬШИЕ МИРЫ</span><h3>Forge</h3><p>Огромные моды и новые измерения.</p></div><span class="choice-arrow">↗</span></button>
        </div>

        <div class="discover-heading"><div><div class="eyebrow muted">ПОПУЛЯРНОЕ СООБЩЕСТВО</div><h2>Моды, которые любят</h2></div><button class="text-link" @click="setPage('Каталог модов')">Открыть каталог <span>↗</span></button></div>
        <div v-if="loading" class="mini-loading">Подбираем моды<span class="loading-dots"> ···</span></div>
        <div v-else class="popular-strip"><button v-for="project in projects.slice(0, 4)" :key="project.project_id" class="popular-card" @click="openProject(project)"><img :src="project.icon_url || ''" @error="$event.target.style.visibility='hidden'"/><span class="fallback-icon">✧</span><div><b>{{ project.title }}</b><small>{{ prettyDownloads(project.downloads) }} загрузок</small></div><span class="mini-arrow">↗</span></button></div>
      </section>

      <section v-else-if="activePage === 'Каталог модов'" class="content catalog-content">
        <div class="catalog-hero"><div><div class="eyebrow"><span class="sparkle">✦</span> МИР МОДИФИКАЦИЙ</div><h1>Открой новые<br><span>миры.</span></h1><p class="hero-copy">Тысячи проектов от создателей со всего мира — найди своё.</p></div><div class="catalog-orb">◈</div></div>
        <div class="catalog-toolbar"><div class="provider-tabs"><button :class="{ selected: activeTab === 'Modrinth' }" @click="switchTab('Modrinth')"><span class="provider-mark modrinth-mark">◈</span> Modrinth</button><button :class="{ selected: activeTab === 'CurseForge' }" @click="switchTab('CurseForge')"><span class="provider-mark curse-mark">✺</span> CurseForge <span class="beta-tag">BETA</span></button></div><button class="filter-button" @click="notify('Фильтры по версии и загрузчику — скоро')">☷ <span>Фильтры</span></button></div>
        <form class="search-box" @submit.prevent="loadCatalog"><span>⌕</span><input v-model="search" placeholder="Поиск модов, шейдеров и ресурсов…"/><kbd>ENTER</kbd><button type="submit" aria-label="Искать">↵</button></form>
        <div class="catalog-context"><span>{{ activeTab === 'Modrinth' ? 'ПОПУЛЯРНЫЕ МОДЫ' : 'CURSEFORGE · МОДЫ' }}</span><span class="results-count">{{ projects.length }} ПРОЕКТОВ</span></div>
        <div v-if="loading" class="catalog-state"><div class="spinner"></div><p>Ищем подходящие проекты…</p></div>
        <div v-else-if="error" class="catalog-state error-state"><div class="state-symbol">!</div><h3>Каталог пока недоступен</h3><p>{{ error }}</p><button class="retry-button" @click="loadCatalog">Попробовать снова</button></div>
        <div v-else-if="projects.length" class="project-grid"><article v-for="project in filteredProjects" :key="project.project_id" class="project-card"><div class="project-cover"><img v-if="project.icon_url" :src="project.icon_url" @error="$event.target.style.display='none'"/><span v-else class="project-fallback">✧</span><span class="project-loader">{{ activeTab }} · мод</span><button class="card-menu" @click="openProject(project)">↗</button></div><div class="project-info"><div class="project-title-row"><h3>{{ project.title }}</h3><button class="install-button" :class="{ installed: installed.includes(project.project_id) }" @click="installProject(project)">{{ installed.includes(project.project_id) ? '✓' : '+' }}</button></div><p>{{ project.description || 'Мод для Minecraft от сообщества.' }}</p><div class="project-stats"><span>↓ {{ prettyDownloads(project.downloads) }}</span><span class="compat-tag">{{ selectedLoader }}</span></div></div></article></div>
        <div v-else class="catalog-state"><div class="state-symbol">⌕</div><h3>Найди свой следующий мод</h3><p>Поиск работает по каталогу {{ activeTab }}.</p></div>
        <div class="catalog-footnote"><span>✦</span> ElyPrism не размещает моды — файлы загружаются с исходных платформ.</div>
      </section>

      <section v-else-if="activePage === 'Сборки' || activePage === 'Библиотека'" class="content empty-page"><div class="eyebrow muted">ТВОЯ КОЛЛЕКЦИЯ</div><h1>{{ activePage === 'Библиотека' ? 'Твоя библиотека.' : 'Сборки сообщества.' }}</h1><p>{{ activePage === 'Библиотека' ? 'Здесь будут жить твои игровые профили.' : 'Готовые сборки появятся здесь.' }}</p><div class="empty-card"><span>✦</span><h3>Место для новых миров</h3><p>Установка Minecraft и управление профилями будут добавлены в следующих версиях.</p><button class="primary-small" @click="createInstance">＋ Создать сборку</button></div></section>
      <section v-else class="content empty-page"><div class="eyebrow muted">ELYPRISM</div><h1>Настройки лаунчера.</h1><p>Подключи каталог CurseForge или настрой интерфейс под себя.</p><div class="empty-card settings-card"><div><div class="settings-icon">⚙</div><h3>CurseForge API</h3><p>Для поиска CurseForge нужен API-ключ. Он хранится только в этом приложении.</p></div><button class="primary-small" @click="showKeyDialog = true">Настроить ключ</button></div></section>

      <footer class="app-footer"><span>ELYPRISM LAUNCHER <i>·</i> РАННЯЯ ВЕРСИЯ</span><span>СДЕЛАНО ДЛЯ ИССЛЕДОВАТЕЛЕЙ <b>✦</b></span></footer>
    </main>

    <div v-if="showKeyDialog" class="modal-backdrop" @click.self="showKeyDialog = false"><div class="key-modal"><button class="modal-close" @click="showKeyDialog = false">×</button><div class="modal-icon">✺</div><div class="eyebrow muted">ИНТЕГРАЦИЯ КАТАЛОГА</div><h2>Подключи CurseForge</h2><p>Вставь свой CurseForge API-ключ. Он сохраняется локально в этом профиле приложения и не отправляется ElyPrism.</p><label for="cf-key">API-ключ</label><input id="cf-key" v-model="keyDraft" type="password" placeholder="Вставь ключ CurseForge"/><div class="modal-actions"><button class="cancel-button" @click="showKeyDialog = false">Отмена</button><button class="primary-small" @click="saveKey">Сохранить ключ</button></div><small class="modal-note">Получение ключа и правила использования регулируются CurseForge.</small></div></div>
    <div v-if="toast" class="toast"><span>✦</span>{{ toast }}</div>
  </div>
</template>
