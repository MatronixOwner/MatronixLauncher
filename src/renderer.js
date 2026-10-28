const api = window.api;

let currentVersion = null;
let currentAccount = null;
let selectedLoader = null;

const els = {
  pages: document.querySelectorAll('.page'),
  navItems: document.querySelectorAll('.nav-item'),
  versionList: document.getElementById('version-list'),
  btnPlay: document.getElementById('btn-play'),
  btnRefresh: document.getElementById('btn-refresh'),
  btnInstallFabric: document.getElementById('btn-install-fabric'),
  btnInstallForge: document.getElementById('btn-install-forge'),
  launchStatus: document.getElementById('launch-status'),
  btnLogin: document.getElementById('btn-login'),
  btnLogout: document.getElementById('btn-logout'),
  accountInfo: document.getElementById('account-info'),
  accName: document.getElementById('acc-name'),
  loginUsername: document.getElementById('login-username'),
  loginPassword: document.getElementById('login-password'),
  login2fa: document.getElementById('login-2fa'),
  loginOtp: document.getElementById('login-otp'),
  btnDoLogin: document.getElementById('btn-dologin'),
  loginError: document.getElementById('login-error'),
  linkRegister: document.getElementById('link-register'),
  modSearch: document.getElementById('mod-search'),
  modLoader: document.getElementById('mod-loader'),
  btnSearchMods: document.getElementById('btn-search-mods'),
  modList: document.getElementById('mod-list'),
  pathAppdata: document.getElementById('path-appdata')
};

function showPage(name) {
  els.pages.forEach(p => p.classList.remove('active'));
  document.getElementById('page-' + name).classList.add('active');
  els.navItems.forEach(b => b.classList.remove('active'));
  document.querySelector('[data-page="' + name + '"]').classList.add('active');
}

els.navItems.forEach(btn => {
  btn.addEventListener('click', () => showPage(btn.dataset.page));
});

async function updateAuthUI() {
  const acc = await api.elyGetAccount();
  currentAccount = acc;
  if (acc && acc.authenticated) {
    els.btnLogin.style.display = 'none';
    els.btnLogout.style.display = 'block';
    els.accountInfo.style.display = 'flex';
    els.accName.textContent = acc.username;
    updatePlayButton();
  } else {
    els.btnLogin.style.display = 'block';
    els.btnLogout.style.display = 'none';
    els.accountInfo.style.display = 'none';
    els.btnPlay.disabled = true;
  }
}

els.btnLogin.addEventListener('click', () => showPage('account'));
els.btnLogout.addEventListener('click', async () => {
  await api.elyLogout();
  updateAuthUI();
});

els.login2fa.addEventListener('change', () => {
  els.loginOtp.disabled = !els.login2fa.checked;
});

els.linkRegister.addEventListener('click', (e) => {
  e.preventDefault();
  api.elyOpenRegister();
});

els.btnDoLogin.addEventListener('click', async () => {
  const username = els.loginUsername.value.trim();
  const password = els.loginPassword.value;
  const otp = els.login2fa.checked ? els.loginOtp.value.trim() : '';

  if (!username || !password) {
    els.loginError.textContent = 'Заполните логин и пароль';
    return;
  }

  els.loginError.textContent = 'Вход в Ely.by...';
  const res = await api.elyLogin({ username, password, otp });

  if (res.success) {
    els.loginError.textContent = '';
    els.loginUsername.value = '';
    els.loginPassword.value = '';
    els.loginOtp.value = '';
    await updateAuthUI();
    showPage('versions');
  } else {
    els.loginError.textContent = res.error;
  }
});

async function loadVersions() {
  els.launchStatus.textContent = 'Загрузка версий...';
  const res = await api.fetchVersions();
  if (!res.success) {
    els.launchStatus.textContent = 'Ошибка: ' + res.error;
    return;
  }

  els.versionList.innerHTML = '';
  res.versions.forEach(v => {
    const div = document.createElement('div');
    div.className = 'version-item';
    div.innerHTML = `<span>${v.id}</span><span class="version-badge">${v.type}</span>`;
    div.addEventListener('click', () => {
      document.querySelectorAll('.version-item').forEach(i => i.classList.remove('selected'));
      div.classList.add('selected');
      currentVersion = v.id;
      els.btnInstallFabric.disabled = false;
      els.btnInstallForge.disabled = false;
      updatePlayButton();
      els.launchStatus.textContent = 'Версия ' + v.id + ' выбрана';
    });
    els.versionList.appendChild(div);
  });

  els.launchStatus.textContent = res.versions.length + ' версий загружено';
}

els.btnRefresh.addEventListener('click', loadVersions);

els.btnInstallFabric.addEventListener('click', async () => {
  if (!currentVersion) return;
  els.launchStatus.textContent = 'Установка Fabric...';
  const res = await api.installFabric(currentVersion);
  if (res.success) {
    selectedLoader = 'fabric';
    els.launchStatus.textContent = 'Fabric ' + res.loaderVersion + ' готов для ' + res.mcVersion;
    updatePlayButton();
  } else {
    els.launchStatus.textContent = 'Ошибка Fabric: ' + res.error;
  }
});

els.btnInstallForge.addEventListener('click', async () => {
  if (!currentVersion) return;
  els.launchStatus.textContent = 'Установка Forge...';
  const res = await api.installForge(currentVersion);
  if (res.success) {
    selectedLoader = 'forge';
    els.launchStatus.textContent = 'Forge ' + (res.forgeVersion || '?') + ' готов для ' + res.mcVersion;
    updatePlayButton();
  } else {
    els.launchStatus.textContent = 'Ошибка Forge: ' + res.error;
  }
});

els.btnPlay.addEventListener('click', async () => {
  if (!currentVersion || !currentAccount) return;

  els.btnPlay.disabled = true;
  els.btnPlay.textContent = 'ЗАПУСК...';

  try {
    const details = await api.fetchVersionDetails(currentVersion);
    if (!details.success) {
      els.launchStatus.textContent = 'Ошибка: ' + details.error;
      els.btnPlay.disabled = false;
      els.btnPlay.textContent = 'ИГРАТЬ';
      return;
    }

    const path = require('path');
    const fs = require('fs');
    const appPath = await api.getAppPath();
    const instanceDir = path.join(appPath, 'instances', currentVersion + (selectedLoader ? '-' + selectedLoader : ''));
    if (!fs.existsSync(instanceDir)) fs.mkdirSync(instanceDir, { recursive: true });

    els.launchStatus.textContent = 'Скачивание клиента...';
    const clientPath = path.join(instanceDir, 'client.jar');
    await api.downloadFile({ url: details.downloadUrl, dest: clientPath });

    const injectUrl = 'https://github.com/boy0001/Authlib-Injector/releases/latest/download/authlib-injector.jar';
    const injectPath = path.join(appPath, 'authlib-injector.jar');
    if (!fs.existsSync(injectPath)) {
      els.launchStatus.textContent = 'Скачивание authlib-injector...';
      await api.downloadFile({ url: injectUrl, dest: injectPath });
    }

    const javaArgs = [
      '-Xmx2048M',
      '-Xms1024M',
      '-cp', clientPath,
      details.mainClass || 'net.minecraft.client.main.Main',
      '--username', currentAccount.username,
      '--uuid', currentAccount.uuid,
      '--accessToken', currentAccount.accessToken,
      '--clientToken', currentAccount.clientToken,
      '--version', currentVersion,
      '--gameDir', instanceDir,
      '--assetsDir', path.join(appPath, 'assets'),
      '--assetIndex', details.assets ? details.assets.id : 'legacy',
      '--userType', 'mojang',
      '--versionType', selectedLoader || 'release'
    ];
    javaArgs.unshift('-javaagent:' + injectPath + '=ely.by');

    console.log('Launch command:', 'java ' + javaArgs.join(' '));
    els.launchStatus.textContent = 'Minecraft запущен (смотрите консоль)';
    setTimeout(() => {
      els.btnPlay.disabled = false;
      els.btnPlay.textContent = 'ИГРАТЬ';
    }, 2000);

  } catch (err) {
    els.launchStatus.textContent = 'Ошибка запуска: ' + err.message;
    els.btnPlay.disabled = false;
    els.btnPlay.textContent = 'ИГРАТЬ';
  }
});

function updatePlayButton() {
  els.btnPlay.disabled = !(currentVersion && currentAccount);
}

els.btnSearchMods.addEventListener('click', async () => {
  const query = els.modSearch.value.trim() || 'optifine';
  const loader = els.modLoader.value || undefined;
  const version = currentVersion || undefined;

  els.modList.innerHTML = '<div style="grid-column:1/-1;color:#aaa;">Поиск...</div>';

  const res = await api.searchModrinth({ query, version, loader });
  if (!res.success) {
    els.modList.innerHTML = '<div style="grid-column:1/-1;color:#ff6b6b;">' + res.error + '</div>';
    return;
  }

  els.modList.innerHTML = '';
  res.mods.forEach(m => {
    const card = document.createElement('div');
    card.className = 'mod-card';
    const tags = (m.loaders || []).map(l => '<span class="mod-tag">' + l + '</span>').join('') +
                 (m.gameVersions || []).slice(0,3).map(v => '<span class="mod-tag">' + v + '</span>').join('');
    card.innerHTML = `
      <h3>${m.name}</h3>
      <p>${m.description || 'Описание отсутствует'}</p>
      <div class="mod-meta">${tags}</div>
      <div style="font-size:11px;color:#888;">Версия: ${m.version || '?'} • Modrinth</div>
    `;
    els.modList.appendChild(card);
  });
});

(async () => {
  const appPath = await api.getAppPath();
  els.pathAppdata.textContent = appPath;
  await updateAuthUI();
  await loadVersions();
})();
