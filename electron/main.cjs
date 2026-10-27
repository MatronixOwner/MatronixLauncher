const { app, BrowserWindow, shell, ipcMain, net, safeStorage } = require('electron');
const path = require('node:path');
const fs = require('node:fs/promises');
const { Auth } = require('msmc');
const { MinecraftFolder, launch } = require('@xmcl/core');
const {
  getVersionList,
  installTask,
  fetchJavaRuntimeManifest,
  installJavaRuntimeTask
} = require('@xmcl/installer');

let gameOperation;
let activeWindow;
let activeXbox;

function reportGameProgress(message, percent = null, state = 'progress') {
  if (!activeWindow || activeWindow.isDestroyed()) return;
  activeWindow.webContents.send('minecraft-progress', { message, percent, state });
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
const accountTokenPath = () => path.join(app.getPath('userData'), 'microsoft-account.bin');

async function getMinecraftAccount() {
  if (activeXbox?.validate()) {
    const cached = await activeXbox.getMinecraft();
    if (cached.profile && !cached.isDemo()) return cached;
  }

  const tokenFile = accountTokenPath();
  try {
    if (safeStorage.isEncryptionAvailable()) {
      const encrypted = await fs.readFile(tokenFile);
      const refreshToken = safeStorage.decryptString(encrypted);
      const auth = new Auth('select_account');
      activeXbox = await auth.refresh(refreshToken);
      const account = await activeXbox.getMinecraft();
      if (!account.profile) throw new Error('Для этого Microsoft-аккаунта не найден профиль Minecraft.');
      if (account.isDemo()) throw new Error('Для игры требуется аккаунт с лицензией Minecraft Java Edition.');
      return account;
    }
  } catch (error) {
    if (error?.code !== 'ENOENT') {
      activeXbox = undefined;
      if (error?.message?.includes('профиль Minecraft') || error?.message?.includes('лицензией Minecraft')) throw error;
    }
  }

  reportGameProgress('Ожидание входа Microsoft…', null, 'progress');
  const auth = new Auth('select_account');
  activeXbox = await auth.launch('electron', { width: 620, height: 760, resizable: true });
  const account = await activeXbox.getMinecraft();
  if (!account.profile) throw new Error('Для этого Microsoft-аккаунта не найден профиль Minecraft.');
  if (account.isDemo()) throw new Error('Для игры требуется аккаунт с лицензией Minecraft Java Edition.');

  if (safeStorage.isEncryptionAvailable()) {
    const encrypted = safeStorage.encryptString(activeXbox.save());
    await fs.writeFile(tokenFile, encrypted, { mode: 0o600 });
  }
  return account;
}

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

async function installAndLaunch(versionId) {
  if (typeof versionId !== 'string' || versionId.length > 80 || !/^[\w.-]+$/.test(versionId)) {
    throw new Error('Выбрана некорректная версия Minecraft.');
  }
  const account = await getMinecraftAccount();
  const manifest = await getVersionList();
  const metadata = manifest.versions.find((version) => version.id === versionId);
  if (!metadata) throw new Error(`Версия Minecraft ${versionId} отсутствует в официальном каталоге.`);

  const folder = MinecraftFolder.from(minecraftRoot());
  await fs.mkdir(folder.root, { recursive: true });
  reportGameProgress(`Установка Minecraft ${versionId}…`, null);
  const resolvedVersion = await installTask(metadata, folder).startAndWait(taskContext('Minecraft'));
  const javaPath = await installJava(resolvedVersion, folder);

  reportGameProgress(`Запуск Minecraft ${versionId}…`, null, 'progress');
  const child = await launch({
    version: resolvedVersion,
    gamePath: folder.root,
    resourcePath: folder.root,
    javaPath,
    gameProfile: { id: account.profile.id, name: account.profile.name },
    accessToken: account.mcToken,
    launcherName: 'MatronixLauncher',
    launcherBrand: 'MatronixLauncher',
    maxMemory: 4096,
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
  if (gameOperation) throw new Error('Установка или запуск Minecraft уже выполняется.');
  if (loader !== 'Vanilla') {
    throw new Error(`${loader} пока не подключён к установщику. Выбери Vanilla для установки и запуска.`);
  }
  gameOperation = installAndLaunch(version);
  try {
    return await gameOperation;
  } catch (error) {
    reportGameProgress(error?.message || 'Не удалось запустить Minecraft.', null, 'error');
    throw error;
  } finally {
    gameOperation = undefined;
  }
});

app.whenReady().then(() => {
  createWindow();
  app.on('activate', () => { if (BrowserWindow.getAllWindows().length === 0) createWindow(); });
});
app.on('window-all-closed', () => { if (process.platform !== 'darwin') app.quit(); });
