const { app, BrowserWindow, shell, ipcMain, net, safeStorage } = require('electron');
const path = require('node:path');
const fs = require('node:fs/promises');
const { randomUUID, createHash } = require('node:crypto');
const { MinecraftFolder, launch } = require('@xmcl/core');
const {
  getVersionList,
  installTask,
  fetchJavaRuntimeManifest,
  installJavaRuntimeTask
} = require('@xmcl/installer');

let gameOperation;
let activeWindow;
let activeElyAccount;

const ELY_AUTH_URL = 'https://authserver.ely.by';
const AUTHLIB_INJECTOR = {
  version: '1.2.8',
  url: 'https://github.com/yushijinhun/authlib-injector/releases/download/v1.2.8/authlib-injector-1.2.8.jar',
  sha256: '9c7f4343e6c82034958ffb48c14a2cb0c85928be7283103ce17da00c6d5a7b10'
};

function reportGameProgress(message, percent = null, state = 'progress') {
  if (!activeWindow || activeWindow.isDestroyed()) return;
  activeWindow.webContents.send('minecraft-progress', { message, percent, state });
}

function explainError(error, depth = 0) {
  if (!error || depth > 4) return '';
  const label = [error.name, error.message].filter(Boolean).join(': ') || String(error);
  const nested = [];
  if (Array.isArray(error.errors)) nested.push(...error.errors);
  if (error.cause) nested.push(error.cause);
  const details = nested.map((item) => explainError(item, depth + 1)).filter(Boolean);
  return [...new Set([label, ...details])].join(' → ').slice(0, 1800);
}

function createWindow() {
  const win = new BrowserWindow({
    width: 1440,
    height: 920,
    minWidth: 1060,
    minHeight: 700,
    backgroundColor: '#111019',
    title: 'MatronixLauncher',
    autoHideMenuBar: true,
    webPreferences: {
      preload: path.join(__dirname, 'preload.cjs'),
      contextIsolation: true,
      nodeIntegration: false,
      sandbox: true
    }
  });
  activeWindow = win;
  if (app.isPackaged) win.loadFile(path.join(__dirname, '../dist/index.html'));
  else win.loadURL('http://localhost:5173');
}

ipcMain.handle('open-external', (_event, url) => {
  if (typeof url === 'string' && /^https:\/\//i.test(url)) return shell.openExternal(url);
});

ipcMain.handle('minecraft-version-manifest', async () => {
  const response = await net.fetch('https://piston-meta.mojang.com/mc/game/version_manifest_v2.json', {
    headers: { Accept: 'application/json' }
  });
  if (!response.ok) throw new Error(`Minecraft manifest request failed: ${response.status}`);
  const manifest = await response.json();
  return {
    latest: manifest.latest,
    versions: (manifest.versions || []).map(({ id, type, releaseTime }) => ({ id, type, releaseTime }))
  };
});

const minecraftRoot = () => path.join(app.getPath('userData'), 'minecraft');
const accountTokenPath = () => path.join(app.getPath('userData'), 'elyby-account.bin');

async function postElyAuth(endpoint, payload) {
  const response = await fetch(`${ELY_AUTH_URL}/auth/${endpoint}`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json', Accept: 'application/json' },
    body: JSON.stringify(payload)
  });
  let result = {};
  try { result = await response.json(); } catch { /* The auth server may return an empty success response. */ }
  if (!response.ok) {
    const reason = result.errorMessage || result.error || `HTTP ${response.status}`;
    const error = new Error(`Ely.by: ${reason}`);
    error.status = response.status;
    throw error;
  }
  return result;
}

async function persistElyAccount(account) {
  activeElyAccount = account;
  if (!safeStorage.isEncryptionAvailable()) return;
  const encrypted = safeStorage.encryptString(JSON.stringify(account));
  await fs.writeFile(accountTokenPath(), encrypted, { mode: 0o600 });
}

async function readSavedElyAccount() {
  if (activeElyAccount) return activeElyAccount;
  if (!safeStorage.isEncryptionAvailable()) return undefined;
  try {
    const encrypted = await fs.readFile(accountTokenPath());
    const saved = JSON.parse(safeStorage.decryptString(encrypted));
    if (saved?.accessToken && saved?.clientToken && saved?.profile?.id && saved?.profile?.name) {
      activeElyAccount = saved;
      return saved;
    }
  } catch (error) {
    if (error?.code !== 'ENOENT') await fs.rm(accountTokenPath(), { force: true }).catch(() => {});
  }
  return undefined;
}

async function authenticateEly({ username, password, totp } = {}) {
  if (typeof username !== 'string' || !username.trim() || typeof password !== 'string' || !password) {
    throw new Error('Укажи логин и пароль Ely.by.');
  }
  const clientToken = randomUUID();
  const combinedPassword = totp?.trim() ? `${password}:${totp.trim()}` : password;
  const result = await postElyAuth('authenticate', {
    username: username.trim(),
    password: combinedPassword,
    clientToken,
    requestUser: true
  });
  if (!result.accessToken || !result.selectedProfile?.id || !result.selectedProfile?.name) {
    throw new Error('Ely.by не вернул игровой профиль. Проверь аккаунт и наличие профиля Minecraft.');
  }
  const account = {
    accessToken: result.accessToken,
    clientToken: result.clientToken || clientToken,
    profile: { id: result.selectedProfile.id, name: result.selectedProfile.name }
  };
  await persistElyAccount(account);
  return account;
}

async function getMinecraftAccount() {
  reportGameProgress('Подключение к Ely.by…', null);
  const saved = await readSavedElyAccount();
  if (!saved) throw new Error('Сначала войди через Ely.by.');
  try {
    const result = await postElyAuth('refresh', {
      accessToken: saved.accessToken,
      clientToken: saved.clientToken,
      requestUser: true
    });
    const account = {
      accessToken: result.accessToken || saved.accessToken,
      clientToken: result.clientToken || saved.clientToken,
      profile: result.selectedProfile?.id && result.selectedProfile?.name
        ? { id: result.selectedProfile.id, name: result.selectedProfile.name }
        : saved.profile
    };
    await persistElyAccount(account);
    return account;
  } catch (error) {
    if (error?.status === 400 || error?.status === 401) {
      activeElyAccount = undefined;
      await fs.rm(accountTokenPath(), { force: true }).catch(() => {});
      throw new Error('Сессия Ely.by истекла. Войди в аккаунт снова.');
    }
    throw error;
  }
}

ipcMain.handle('ely-account', async () => (await readSavedElyAccount())?.profile || null);
ipcMain.handle('ely-login', async (_event, credentials) => {
  try {
    const account = await authenticateEly(credentials);
    return { ok: true, profile: account.profile };
  } catch (error) {
    return { ok: false, error: explainError(error) || 'Не удалось войти в Ely.by.' };
  }
});
ipcMain.handle('ely-logout', async () => {
  const account = await readSavedElyAccount();
  activeElyAccount = undefined;
  if (account) {
    await postElyAuth('invalidate', { accessToken: account.accessToken, clientToken: account.clientToken }).catch(() => {});
  }
  await fs.rm(accountTokenPath(), { force: true }).catch(() => {});
  return true;
});


function taskContext(label) {
  return {
    onUpdate(task) {
      const total = Number(task.total) || 0;
      const progress = Number(task.progress) || 0;
      const percent = total > 0 ? Math.min(99, Math.round((progress / total) * 100)) : null;
      reportGameProgress(`${label}${task.path ? `: ${task.path.split('.').slice(-1)[0]}` : ''}`, percent);
    }
  };
}

async function installJava(version, folder) {
  const java = version.javaVersion;
  if (!java?.component) throw new Error('Не удалось определить требуемую версию Java.');
  const runtimeRoot = path.join(folder.root, 'runtime', java.component, String(java.majorVersion));
  const javaExecutable = path.join(runtimeRoot, 'bin', process.platform === 'win32' ? 'java.exe' : 'java');
  try {
    await fs.access(javaExecutable);
    return javaExecutable;
  } catch {
    // Install the official runtime for the selected Minecraft version.
  }

  reportGameProgress(`Установка Java ${java.majorVersion}…`, null);
  const manifest = await fetchJavaRuntimeManifest({ target: java.component });
  await installJavaRuntimeTask({ manifest, destination: runtimeRoot }).startAndWait(taskContext('Java'));
  await fs.access(javaExecutable);
  return javaExecutable;
}

async function ensureAuthlibInjector(folder) {
  const directory = path.join(folder.root, 'libraries', 'authlib-injector');
  const destination = path.join(directory, `authlib-injector-${AUTHLIB_INJECTOR.version}.jar`);
  const matchesChecksum = async (file) => {
    try {
      const content = await fs.readFile(file);
      return createHash('sha256').update(content).digest('hex') === AUTHLIB_INJECTOR.sha256;
    } catch {
      return false;
    }
  };
  if (await matchesChecksum(destination)) return destination;

  reportGameProgress('Загрузка компонента авторизации Ely.by…', null);
  const response = await fetch(AUTHLIB_INJECTOR.url);
  if (!response.ok) throw new Error(`Не удалось получить authlib-injector: HTTP ${response.status}`);
  const content = Buffer.from(await response.arrayBuffer());
  const digest = createHash('sha256').update(content).digest('hex');
  if (digest !== AUTHLIB_INJECTOR.sha256) throw new Error('Контрольная сумма authlib-injector не совпала. Файл не установлен.');
  await fs.mkdir(directory, { recursive: true });
  const temporary = `${destination}.${process.pid}.tmp`;
  await fs.writeFile(temporary, content, { mode: 0o600 });
  await fs.rename(temporary, destination);
  return destination;
}

async function installAndLaunch(versionId) {
  if (typeof versionId !== 'string' || versionId.length > 80 || !/^[\w.-]+$/.test(versionId)) {
    throw new Error('Выбрана некорректная версия Minecraft.');
  }
  const account = await getMinecraftAccount();
  reportGameProgress('Получение каталога Minecraft…', null);
  const manifest = await getVersionList();
  const metadata = manifest.versions.find((version) => version.id === versionId);
  if (!metadata) throw new Error(`Версия Minecraft ${versionId} отсутствует в официальном каталоге.`);

  const folder = MinecraftFolder.from(minecraftRoot());
  await fs.mkdir(folder.root, { recursive: true });
  reportGameProgress(`Установка Minecraft ${versionId}…`, null);
  const resolvedVersion = await installTask(metadata, folder).startAndWait(taskContext('Minecraft'));
  const javaPath = await installJava(resolvedVersion, folder);

  const authlibInjector = await ensureAuthlibInjector(folder);
  reportGameProgress(`Запуск Minecraft ${versionId}…`, null, 'progress');
  const child = await launch({
    version: resolvedVersion,
    gamePath: folder.root,
    resourcePath: folder.root,
    javaPath,
    gameProfile: { id: account.profile.id, name: account.profile.name },
    accessToken: account.accessToken,
    launcherName: 'MatronixLauncher',
    launcherBrand: 'MatronixLauncher',
    maxMemory: 4096,
    extraJVMArgs: [`-javaagent:${authlibInjector}=ely.by`],
    extraExecOption: { windowsHide: true }
  });
  reportGameProgress(`Minecraft ${versionId} запущен`, null, 'running');
  child.once('error', (error) => reportGameProgress(error.message || 'Не удалось запустить Minecraft.', null, 'error'));
  child.once('exit', (code) => {
    const message = code === 0 ? 'Minecraft закрыт' : `Minecraft завершился с кодом ${code ?? 'неизвестно'}`;
    reportGameProgress(message, null, 'finished');
  });
  return { version: versionId, account: account.profile.name };
}

ipcMain.handle('minecraft-launch', async (_event, { version, loader }) => {
  if (gameOperation) return { ok: false, error: 'Установка или запуск Minecraft уже выполняется.' };
  if (loader !== 'Vanilla') {
    return { ok: false, error: `${loader} пока не подключён к установщику. Выбери Vanilla для установки и запуска.` };
  }
  gameOperation = installAndLaunch(version);
  try {
    return { ok: true, ...(await gameOperation) };
  } catch (error) {
    const message = explainError(error) || 'Не удалось установить или запустить Minecraft.';
    console.error('[minecraft-launch]', message);
    return { ok: false, error: message };
  } finally {
    gameOperation = undefined;
  }
});

app.whenReady().then(() => {
  createWindow();
  app.on('activate', () => { if (BrowserWindow.getAllWindows().length === 0) createWindow(); });
});
app.on('window-all-closed', () => { if (process.platform !== 'darwin') app.quit(); });
