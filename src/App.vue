<script setup>
import { computed, nextTick, onMounted, ref } from 'vue';
import brandMark from './assets/matronix-mark.png';

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
const assistantOpen = ref(false);
const assistantInput = ref('');
const assistantTyping = ref(false);
const assistantMessages = ref([
  {
    id: 1,
    role: 'assistant',
    text: 'Привет. Спроси про моды, версии или настройки.'
  }
]);
const assistantSuggestions = ['Какой загрузчик выбрать?', 'Не запускается Minecraft', 'Как подключить CurseForge?'];
const navItems = ['Главная', 'Каталог модов', 'Сборки', 'Библиотека'];

const filteredProjects = computed(() => projects.value);

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
      icon_url: m.logo?.url, slug: m.slug,
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
function getAssistantReply(question) {
  const q = question.toLowerCase();
  if (/curseforge|curse forge|курс.?фордж|курсов|ключ/.test(q)) {
    return {
      text: 'Для CurseForge нужен API-ключ: открой настройки лаунчера и сохрани ключ в поле CurseForge API. Пока каталог недоступен без ключа; проверь также, открывается ли сам CurseForge в твоей сети.',
      action: 'curseforge',
      actionLabel: 'Открыть настройки'
    };
  }
  if (/modrinth|модринт|найти мод|каталог|поиск мод/.test(q)) {
    return {
      text: 'Открой «Каталог модов» и выбери Modrinth. Можно искать по названию. Карточки открывают страницу проекта на исходном сайте; фактическая установка модов в этой версии ещё не подключена.',
      action: 'catalog',
      actionLabel: 'Открыть каталог'
    };
  }
  if (/чёрн|черн|black screen|пуст(ой|ое) экран|не вид|завис/.test(q)) {
    return {
      text: 'В новых сборках исправлены пути интерфейса, которые могли вызывать чёрный экран. Полностью закрой лаунчер и запусти снова. Если экран останется, пришли версию Windows и текст ошибки.'
    };
  }
  if (/не запуска|ошибк|вылет|краш|не работает/.test(q)) {
    return {
      text: 'Пока игровой движок ещё не подключён, поэтому кнопка «Играть» не запускает Minecraft. Если проблема с интерфейсом: перезапусти лаунчер и проверь подключение. Напиши, что именно нажал и какой текст ошибки увидел.'
    };
  }
  if (/forge|fabric|загрузчик|loader/.test(q)) {
    return {
      text: 'Fabric обычно выбирают для лёгких модов и оптимизации; Forge — для модов и сборок, которым нужен Forge. Мод должен совпадать с версией Minecraft и загрузчиком. Сейчас выбор в интерфейсе демонстрационный — установка загрузчика ещё впереди.'
    };
  }
  if (/верси|стар(ая|ые)|все версии|1\.\d/.test(q)) {
    return {
      text: 'В интерфейсе пока показаны лишь примеры версий. Поддержка всех версий Minecraft потребует менеджера версий и отдельных установщиков загрузчиков; эта часть ещё не реализована.'
    };
  }
  if (/мод|установ/.test(q)) {
    return {
      text: 'Сейчас каталог показывает проекты, а кнопка «+» только демонстрирует очередь — она ещё не скачивает файлы. Для реальной установки нужно будет проверить совместимость мода с версией игры и Fabric/Forge.'
    };
  }
  if (/java|джава|памят|рам|лаг|фпс|fps|тормоз/.test(q)) {
    return {
      text: 'Для модов сначала подбирай версию Java и объём памяти под выбранную версию Minecraft и сборку. Слишком мало памяти вызывает вылеты, а слишком много не всегда повышает FPS. Настройки Java и тестирование производительности пока не добавлены.'
    };
  }
  if (/аккаунт|вход|майкрософт|microsoft/.test(q)) {
    return {
      text: 'Авторизация Microsoft ещё не подключена. Для будущего релиза нужно будет добавить официальный OAuth-вход; не вводи пароль от Microsoft в сторонние формы.'
    };
  }
  if (/интернет|страна|росси|доступ/.test(q)) {
    return {
      text: 'Сам интерфейс работает локально. Каталоги обращаются к официальным сервисам Modrinth и CurseForge напрямую, поэтому доступность зависит от региона, сети и правил самих платформ.'
    };
  }
  return {
    text: 'Я подскажу по каталогу модов, CurseForge, Modrinth, версиям, Fabric и Forge, а также объясню текущие ограничения лаунчера. Попробуй задать вопрос конкретнее — например: «как настроить CurseForge?»'
  };
}

async function submitAssistant(text = assistantInput.value) {
  const question = text.trim();
  if (!question || assistantTyping.value) return;
  assistantMessages.value.push({ id: Date.now(), role: 'user', text: question });
  assistantInput.value = '';
  assistantTyping.value = true;
  await nextTick();
  const chat = document.querySelector('.assistant-messages');
  if (chat) chat.scrollTop = chat.scrollHeight;
  window.setTimeout(async () => {
    assistantMessages.value.push({ id: Date.now() + 1, role: 'assistant', ...getAssistantReply(question) });
    assistantTyping.value = false;
    await nextTick();
    const updatedChat = document.querySelector('.assistant-messages');
    if (updatedChat) updatedChat.scrollTop = updatedChat.scrollHeight;
  }, 350);
}

function runAssistantAction(action) {
  assistantOpen.value = false;
  if (action === 'curseforge') {
    activePage.value = 'Настройки';
    keyDraft.value = curseKey.value;
    showKeyDialog.value = true;
  } else if (action === 'catalog') {
    setPage('Каталог модов');
  }
}
onMounted(() => loadModrinth());
</script>

<template>
  <div class="app-shell">
    <aside class="sidebar">
      <div class="brand"><img class="brand-image" :src="brandMark" alt="MatronixLauncher"/><div><b>MATRONIX</b><small>LAUNCHER</small></div></div>
      <nav class="main-nav">
        <button v-for="item in navItems" :key="item" :class="['nav-item', { active: activePage === item }]" @click="setPage(item)">{{ item }}</button>
      </nav>
      <div class="side-divider"></div>
      <button class="instance-card" @click="activePage = 'Библиотека'">
        <img class="instance-thumb" :src="brandMark" alt="" />
        <div class="instance-info"><b>Prism Survival</b><small>1.21.1 · Fabric</small></div>
      </button>
      <button class="add-instance" @click="createInstance"><span>＋</span> Новая сборка</button>
      <div class="sidebar-bottom"><button class="settings-shortcut" title="Настройки" aria-label="Настройки" @click="setPage('Настройки')">⚙</button></div>
    </aside>

    <main class="main-area">
      <header class="topbar">
        <div class="top-actions"><button class="assistant-top-button" aria-label="ИИ-помощник" @click="assistantOpen = !assistantOpen">Помощник</button></div>
      </header>

      <section v-if="activePage === 'Главная'" class="content home-content">
        <div class="launch-panel"><div class="launch-instance"><div><b>Prism Survival</b><span>Minecraft {{ selectedVersion }} <i>·</i> {{ selectedLoader }}</span></div></div><div class="launch-meta"><div><small>ВЕРСИЯ</small><select v-model="selectedVersion"><option>1.21.1</option><option>1.20.1</option><option>1.19.2</option><option>1.18.2</option></select></div><div class="meta-divider"></div><div><small>ЗАГРУЗЧИК</small><select v-model="selectedLoader"><option>Fabric</option><option>Forge</option><option>Vanilla</option></select></div><button class="play-button" @click="launchGame"><span>▶</span> Играть</button></div></div>

        <div class="section-heading"><h2>Загрузчик</h2><button class="text-link" @click="setPage('Библиотека')">Профили</button></div>
        <div class="choice-grid">
          <button :class="['choice-card', 'vanilla', { selected: selectedLoader === 'Vanilla' }]" @click="selectedLoader = 'Vanilla'"><h3>Vanilla</h3></button>
          <button :class="['choice-card', 'fabric', { selected: selectedLoader === 'Fabric' }]" @click="selectedLoader = 'Fabric'"><h3>Fabric</h3></button>
          <button :class="['choice-card', 'forge', { selected: selectedLoader === 'Forge' }]" @click="selectedLoader = 'Forge'"><h3>Forge</h3></button>
        </div>

        <div class="discover-heading"><h2>Моды</h2><button class="text-link" @click="setPage('Каталог модов')">Каталог</button></div>
        <div v-if="loading" class="mini-loading">Загрузка</div>
        <div v-else class="popular-strip"><button v-for="project in projects.slice(0, 4)" :key="project.project_id" class="popular-card" @click="openProject(project)"><img :src="project.icon_url || ''" @error="$event.target.style.visibility='hidden'"/><span class="fallback-icon">{{ project.title?.slice(0, 1) }}</span><div><b>{{ project.title }}</b></div><span class="mini-arrow">↗</span></button></div>
      </section>

      <section v-else-if="activePage === 'Каталог модов'" class="content catalog-content">
        <div class="catalog-page-heading"><h1>Моды</h1></div>
        <div class="catalog-toolbar"><div class="provider-tabs"><button :class="{ selected: activeTab === 'Modrinth' }" @click="switchTab('Modrinth')">Modrinth</button><button :class="{ selected: activeTab === 'CurseForge' }" @click="switchTab('CurseForge')">CurseForge</button></div></div>
        <form class="search-box" @submit.prevent="loadCatalog"><input v-model="search" placeholder="Поиск модов"/><button type="submit" aria-label="Искать">↵</button></form>
        <div v-if="loading" class="catalog-state"><div class="spinner"></div><p>Ищем подходящие проекты…</p></div>
        <div v-else-if="error" class="catalog-state error-state"><div class="state-symbol">!</div><h3>Каталог пока недоступен</h3><p>{{ error }}</p><button class="retry-button" @click="loadCatalog">Попробовать снова</button></div>
        <div v-else-if="projects.length" class="project-grid"><article v-for="project in filteredProjects" :key="project.project_id" class="project-card"><div class="project-cover"><img v-if="project.icon_url" :src="project.icon_url" @error="$event.target.style.display='none'"/><span v-else class="project-fallback">{{ project.title?.slice(0, 1) }}</span><button class="card-menu" aria-label="Открыть страницу мода" @click="openProject(project)">↗</button></div><div class="project-info"><div class="project-title-row"><h3>{{ project.title }}</h3><button class="install-button" :class="{ installed: installed.includes(project.project_id) }" @click="installProject(project)">{{ installed.includes(project.project_id) ? '✓' : '+' }}</button></div><p>{{ project.description || 'Мод для Minecraft от сообщества.' }}</p></div></article></div>
        <div v-else class="catalog-state"><p>Нет результатов</p></div>
      </section>

      <section v-else-if="activePage === 'Сборки' || activePage === 'Библиотека'" class="content empty-page"><h1>{{ activePage === 'Библиотека' ? 'Библиотека' : 'Сборки' }}</h1><p>Пока пусто</p><button class="primary-small" @click="createInstance">Создать сборку</button></section>
      <section v-else class="content empty-page"><h1>Настройки</h1><div class="empty-card settings-card"><h3>CurseForge API</h3><button class="primary-small" @click="showKeyDialog = true">Настроить</button></div></section>

    </main>

    <div v-if="showKeyDialog" class="modal-backdrop" @click.self="showKeyDialog = false"><div class="key-modal"><button class="modal-close" @click="showKeyDialog = false">×</button><h2>CurseForge</h2><p>API-ключ хранится локально.</p><label for="cf-key">API-ключ</label><input id="cf-key" v-model="keyDraft" type="password" placeholder="Вставь ключ CurseForge"/><div class="modal-actions"><button class="cancel-button" @click="showKeyDialog = false">Отмена</button><button class="primary-small" @click="saveKey">Сохранить ключ</button></div></div></div>
    <section v-if="assistantOpen" class="assistant-panel" aria-label="ИИ-помощник MatronixLauncher">
      <header class="assistant-panel-head">
        <div class="assistant-avatar">M</div>
        <div class="assistant-head-copy"><b>Помощник</b></div>
        <button class="assistant-close" aria-label="Закрыть помощника" @click="assistantOpen = false">×</button>
      </header>
      <div class="assistant-messages" aria-live="polite">
        <div v-for="message in assistantMessages" :key="message.id" :class="['assistant-message', message.role === 'user' ? 'user-message' : 'bot-message']">
          <p>{{ message.text }}</p>
          <button v-if="message.action" class="assistant-action" @click="runAssistantAction(message.action)">{{ message.actionLabel }} <span>↗</span></button>
        </div>
        <div v-if="assistantTyping" class="assistant-message bot-message typing-message"><span></span><span></span><span></span></div>
      </div>
      <div v-if="assistantMessages.length === 1" class="assistant-suggestions">
        <button v-for="suggestion in assistantSuggestions" :key="suggestion" @click="submitAssistant(suggestion)">{{ suggestion }} <span>↗</span></button>
      </div>
      <form class="assistant-input-row" @submit.prevent="submitAssistant()">
        <input v-model="assistantInput" aria-label="Сообщение помощнику" placeholder="Спроси о лаунчере…" maxlength="400" />
        <button type="submit" :disabled="!assistantInput.trim() || assistantTyping" aria-label="Отправить вопрос">↑</button>
      </form>
    </section>
    <div v-if="toast" class="toast">{{ toast }}</div>
  </div>
</template>
