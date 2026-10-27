<script setup>
import { computed, nextTick, onMounted, ref, watch } from 'vue';
import brandMark from './assets/matronix-mined-mark.png';

const activePage = ref('Главная');
const activeTab = ref('Modrinth');
const search = ref('');
const loading = ref(false);
const error = ref('');
const projects = ref([]);
const toast = ref('');
const selectedLoader = ref(localStorage.getItem('matronix-loader') || 'Vanilla');
const selectedVersion = ref(localStorage.getItem('matronix-version') || '1.21.1');
const versionList = ref([]);
const versionLoading = ref(false);
const versionLoadError = ref('');
const showVersionPicker = ref(false);
const versionSearch = ref('');
const versionTypeFilter = ref('all');
const gameBusy = ref(false);
const gameStatus = ref('');
const gameProgress = ref(null);
const elyProfile = ref(null);
const showElyLoginDialog = ref(false);
const elyUsername = ref('');
const elyPassword = ref('');
const elyTotp = ref('');
const elyLoginBusy = ref(false);
const elyLoginError = ref('');
const fallbackVersions = [
  { id: '1.21.1', type: 'release' }, { id: '1.20.1', type: 'release' },
  { id: '1.19.2', type: 'release' }, { id: '1.18.2', type: 'release' },
  { id: '1.16.5', type: 'release' }
];
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
const filteredVersions = computed(() => {
  const query = versionSearch.value.trim().toLowerCase();
  return versionList.value.filter((version) => {
    const matchesQuery = !query || version.id.toLowerCase().includes(query);
    const matchesType = versionTypeFilter.value === 'all'
      || version.type === versionTypeFilter.value
      || (versionTypeFilter.value === 'old' && (version.type === 'old_alpha' || version.type === 'old_beta'));
    return matchesQuery && matchesType;
  });
});

watch(selectedVersion, (value) => localStorage.setItem('matronix-version', value));
watch(selectedLoader, (value) => localStorage.setItem('matronix-loader', value));

function notify(message) {
  toast.value = message;
  window.clearTimeout(notify.timer);
  notify.timer = window.setTimeout(() => (toast.value = ''), 3200);
}

async function loadMinecraftVersions() {
  versionLoading.value = true;
  versionLoadError.value = '';
  try {
    let manifest;
    if (window.elyprism?.desktop && window.elyprism.getMinecraftVersions) {
      manifest = await window.elyprism.getMinecraftVersions();
    } else {
      const response = await fetch('https://piston-meta.mojang.com/mc/game/version_manifest_v2.json');
      if (!response.ok) throw new Error(`Minecraft manifest request failed: ${response.status}`);
      manifest = await response.json();
    }
    versionList.value = (manifest.versions || []).filter((version) =>
      ['release', 'snapshot', 'old_beta', 'old_alpha'].includes(version.type)
    );
    if (!versionList.value.length) throw new Error('The version manifest is empty.');
    if (!versionList.value.some((version) => version.id === selectedVersion.value)) {
      selectedVersion.value = manifest.latest?.release || versionList.value[0].id;
    }
  } catch (error) {
    versionList.value = fallbackVersions;
    versionLoadError.value = 'Не удалось загрузить полный список версий. Проверь подключение к интернету.';
  } finally {
    versionLoading.value = false;
  }
}

function toggleVersionPicker() {
  showVersionPicker.value = !showVersionPicker.value;
  versionSearch.value = '';
  versionTypeFilter.value = 'all';
}

function selectVersion(version) {
  selectedVersion.value = version.id;
  showVersionPicker.value = false;
}

function versionTypeLabel(type) {
  return ({ release: 'Релиз', snapshot: 'Снапшот', old_beta: 'Бета', old_alpha: 'Альфа' })[type] || type;
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
async function submitElyLogin() {
  if (elyLoginBusy.value) return;
  elyLoginBusy.value = true;
  elyLoginError.value = '';
  try {
    const result = await window.elyprism.loginEly({
      username: elyUsername.value.trim(),
      password: elyPassword.value,
      totp: elyTotp.value.trim()
    });
    if (!result?.ok) throw new Error(result?.error || 'Не удалось войти в Ely.by.');
    elyProfile.value = result.profile;
    elyPassword.value = '';
    elyTotp.value = '';
    showElyLoginDialog.value = false;
    notify(`Вход выполнен: ${elyProfile.value.name}`);
  } catch (error) {
    const message = error?.message || 'Не удалось войти в Ely.by.';
    elyLoginError.value = /two factor|2fa|totp/i.test(message)
      ? 'Для этого аккаунта требуется код двухфакторной защиты. Введи одноразовый код и попробуй снова.'
      : message;
  } finally {
    elyLoginBusy.value = false;
    elyPassword.value = '';
    elyTotp.value = '';
  }
}
async function logoutEly() {
  await window.elyprism?.logoutEly?.();
  elyProfile.value = null;
  elyPassword.value = '';
  elyTotp.value = '';
  showElyLoginDialog.value = false;
  notify('Выход из Ely.by выполнен');
}
function openElyRegister() {
  if (window.elyprism?.desktop) window.elyprism.openExternal('https://account.ely.by/register');
  else window.open('https://account.ely.by/register', '_blank', 'noopener');
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
async function launchGame() {
  if (!window.elyprism?.desktop || !window.elyprism.launchMinecraft) {
    notify('Запуск доступен в установленном приложении MatronixLauncher.');
    return;
  }
  if (!elyProfile.value) {
    showElyLoginDialog.value = true;
    return;
  }
  if (gameBusy.value) return;
  gameBusy.value = true;
  gameStatus.value = 'Подготовка…';
  gameProgress.value = null;
  try {
    const result = await window.elyprism.launchMinecraft(selectedVersion.value, selectedLoader.value);
    if (!result?.ok) throw new Error(result?.error || 'Не удалось установить или запустить Minecraft.');
  } catch (error) {
    notify(error?.message || 'Не удалось установить или запустить Minecraft.');
    if (/сессия Ely\.by истекла/i.test(error?.message || '')) {
      elyProfile.value = null;
      showElyLoginDialog.value = true;
    }
    gameBusy.value = false;
    gameStatus.value = '';
    gameProgress.value = null;
  }
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
      text: 'Кнопка «Играть» устанавливает выбранную Vanilla-версию с официальных серверов и запускает её после входа в аккаунт Ely.by. Первый запуск может занять время из-за загрузки игры, Java и компонента авторизации Ely.by. Fabric и Forge пока не подключены.'
    };
  }
  if (/forge|fabric|загрузчик|loader/.test(q)) {
    return {
      text: 'Fabric обычно выбирают для лёгких модов и оптимизации; Forge — для модов и сборок, которым нужен Forge. Сейчас реальная установка и запуск подключены для Vanilla. Для Fabric и Forge интеграция установщиков ещё в разработке.'
    };
  }
  if (/верси|стар(ая|ые)|все версии|1\.\d/.test(q)) {
    return {
      text: 'Список версий загружается из официального манифеста Minecraft. Кнопка «Играть» устанавливает выбранную Vanilla-версию, нужные файлы и Java, затем запускает игру после входа через Ely.by.'
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
      text: 'Войди через Ely.by в верхней панели лаунчера. Пароль отправляется напрямую на официальный сервер авторизации Ely.by по HTTPS и не сохраняется; сохраняется только зашифрованный токен сессии.'
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
onMounted(() => {
  loadModrinth();
  loadMinecraftVersions();
  window.elyprism?.getElyAccount?.().then((profile) => { elyProfile.value = profile; }).catch(() => {});
  window.elyprism?.onMinecraftProgress?.((progress) => {
    gameStatus.value = progress.message || '';
    gameProgress.value = progress.percent;
    if (progress.state === 'running') {
      gameBusy.value = false;
      notify(progress.message || 'Minecraft запущен');
    } else if (progress.state === 'finished') {
      gameBusy.value = false;
      gameStatus.value = '';
      gameProgress.value = null;
      notify(progress.message || 'Minecraft закрыт');
    } else if (progress.state === 'error') {
      gameBusy.value = false;
      notify(progress.message || 'Ошибка запуска Minecraft');
      gameStatus.value = '';
      gameProgress.value = null;
    }
  });
});
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
        <div class="instance-info"><b>Prism Survival</b><small>{{ selectedVersion }} · {{ selectedLoader }}</small></div>
      </button>
      <button class="add-instance" @click="createInstance"><span>＋</span> Новая сборка</button>
      <div class="sidebar-bottom"><button class="settings-shortcut" title="Настройки" aria-label="Настройки" @click="setPage('Настройки')">⚙</button></div>
    </aside>

    <main class="main-area" @click="showVersionPicker = false" @keydown.esc="showVersionPicker = false">
      <header class="topbar">
        <div class="top-actions"><button class="ely-account-button" @click="showElyLoginDialog = true">{{ elyProfile?.name || 'Войти через Ely.by' }}</button><button class="assistant-top-button" aria-label="ИИ-помощник" @click="assistantOpen = !assistantOpen">Помощник</button></div>
      </header>

      <section v-if="activePage === 'Главная'" class="content home-content">
        <div class="launch-panel"><div class="launch-instance"><div><b>Prism Survival</b><span>Minecraft {{ selectedVersion }} <i>·</i> {{ selectedLoader }}</span></div></div><div class="launch-meta"><div class="version-field" @click.stop><small>ВЕРСИЯ</small><button class="version-trigger" type="button" :aria-expanded="showVersionPicker" @click="toggleVersionPicker">{{ selectedVersion }} <span>{{ showVersionPicker ? '⌃' : '⌄' }}</span></button><div v-if="showVersionPicker" class="version-picker" @keydown.esc.stop="showVersionPicker = false"><input v-model="versionSearch" autofocus placeholder="Поиск версии" aria-label="Поиск версии"/><div class="version-kind-tabs"><button v-for="kind in [{ id: 'all', label: 'Все' }, { id: 'release', label: 'Релизы' }, { id: 'snapshot', label: 'Снапшоты' }, { id: 'old', label: 'Старые' }]" :key="kind.id" type="button" :class="{ selected: versionTypeFilter === kind.id }" @click="versionTypeFilter = kind.id">{{ kind.label }}</button></div><div v-if="versionLoading" class="version-message">Загрузка версий…</div><div v-else class="version-options"><button v-for="version in filteredVersions" :key="version.id" type="button" :class="{ selected: selectedVersion === version.id }" @click="selectVersion(version)"><span>{{ version.id }}</span><small>{{ versionTypeLabel(version.type) }}</small></button><p v-if="!filteredVersions.length" class="version-message">Версии не найдены</p></div><p v-if="versionLoadError" class="version-error">{{ versionLoadError }}</p></div></div><div class="meta-divider"></div><div><small>ЗАГРУЗЧИК</small><select v-model="selectedLoader"><option>Fabric</option><option>Forge</option><option>Vanilla</option></select></div><button class="play-button" :disabled="gameBusy" :title="gameBusy ? gameStatus : ''" @click="launchGame"><span v-if="!gameBusy">▶</span>{{ gameBusy ? `${gameStatus}${gameProgress !== null ? ` ${gameProgress}%` : ''}` : 'Играть' }}</button></div></div>

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
    <div v-if="showElyLoginDialog" class="modal-backdrop" @click.self="showElyLoginDialog = false"><div class="key-modal ely-login-modal"><button class="modal-close" aria-label="Закрыть" @click="showElyLoginDialog = false">×</button><h2>{{ elyProfile ? 'Аккаунт Ely.by' : 'Вход через Ely.by' }}</h2><template v-if="elyProfile"><p>Подключён аккаунт <b>{{ elyProfile.name }}</b>.</p><div class="modal-actions"><button class="cancel-button" @click="showElyLoginDialog = false">Закрыть</button><button class="primary-small" @click="logoutEly">Выйти</button></div></template><form v-else @submit.prevent="submitElyLogin"><p>Войди в свой аккаунт Ely.by. Пароль отправляется напрямую на authserver.ely.by по HTTPS и не сохраняется лаунчером.</p><label for="ely-username">Логин или e-mail</label><input id="ely-username" v-model="elyUsername" autocomplete="username" required maxlength="254"/><label for="ely-password">Пароль</label><input id="ely-password" v-model="elyPassword" type="password" autocomplete="current-password" required maxlength="256"/><label for="ely-totp">Код двухфакторной защиты (если включён)</label><input id="ely-totp" v-model="elyTotp" inputmode="numeric" autocomplete="one-time-code" maxlength="12" placeholder="Необязательно"/><p v-if="elyLoginError" class="ely-login-error">{{ elyLoginError }}</p><div class="modal-actions"><button type="button" class="cancel-button" :disabled="elyLoginBusy" @click="showElyLoginDialog = false">Отмена</button><button type="submit" class="primary-small" :disabled="elyLoginBusy">{{ elyLoginBusy ? 'Вход…' : 'Войти' }}</button></div><button type="button" class="ely-register-link" @click="openElyRegister">Создать аккаунт Ely.by</button></form></div></div>
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
