const { app, BrowserWindow, ipcMain, shell } = require('electron');
const path = require('path');
const fs = require('fs');
const axios = require('axios');

function createWindow() {
  const win = new BrowserWindow({
    width: 1100,
    height: 750,
    minWidth: 900,
    minHeight: 650,
    icon: path.join(__dirname, '../assets/icon.ico'),
    webPreferences: {
      nodeIntegration: false,
      contextIsolation: true,
      preload: path.join(__dirname, 'preload.js')
    },
    show: false
  });

  win.loadFile(path.join(__dirname, '../src/index.html'));

  win.once('ready-to-show', () => {
    win.show();
  });

  return win;
}

let mainWindow = null;

app.whenReady().then(() => {
  mainWindow = createWindow();

  app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      mainWindow = createWindow();
    }
  });
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

// ---------- Ely.by authentication ----------
ipcMain.handle('ely-login', async (event, { username, password, otp }) => {
  const clientToken = getClientToken();
  const body = {
    username,
    password: otp ? password + ':' + otp : password,
    clientToken,
    requestUser: true
  };

  try {
    const resp = await axios.post('https://authserver.ely.by/auth/authenticate', body, {
      headers: {
        'Content-Type': 'application/json',
        'User-Agent': 'MatronixLauncher/1.0'
      },
      timeout: 30000
    });

    const data = resp.data;
    if (data.error) {
      return { success: false, error: data.errorMessage || data.error };
    }

    const account = {
      username: data.selectedProfile.name,
      uuid: data.selectedProfile.id,
      accessToken: data.accessToken,
      clientToken: data.clientToken,
      authenticated: true
    };

    saveAccount(account);
    return { success: true, account };
  } catch (err) {
    return { success: false, error: err.message || 'Unknown error' };
  }
});

ipcMain.handle('ely-logout', () => {
  clearAccount();
  return { success: true };
});

ipcMain.handle('ely-get-account', () => {
  return getSavedAccount();
});

ipcMain.handle('ely-open-register', () => {
  shell.openExternal('https://ely.by/register');
});

// ---------- Minecraft versions ----------
ipcMain.handle('fetch-versions', async () => {
  try {
    const resp = await axios.get('https://launchermeta.mojang.com/mc/game/version_manifest.json', {
      timeout: 30000
    });
    const versions = resp.data.versions
      .filter(v => v.type === 'release')
      .map(v => ({
        id: v.id,
        type: v.type,
        releaseTime: v.releaseTime,
        url: v.url
      }));
    return { success: true, versions };
  } catch (err) {
    return { success: false, error: err.message };
  }
});

ipcMain.handle('fetch-version-details', async (event, versionId) => {
  try {
    // Find version url from manifest first
    const manifestResp = await axios.get('https://launchermeta.mojang.com/mc/game/version_manifest.json', {
      timeout: 30000
    });
    const entry = manifestResp.data.versions.find(v => v.id === versionId);
    if (!entry) {
      return { success: false, error: 'Version not found' };
    }

    const detailResp = await axios.get(entry.url, { timeout: 30000 });
    const detail = detailResp.data;

    return {
      success: true,
      id: detail.id,
      javaVersion: detail.javaVersion ? detail.javaVersion.component : '8',
      downloadUrl: detail.downloads.client.url,
      assets: detail.assetIndex,
      libraries: detail.libraries,
      mainClass: detail.mainClass,
      arguments: detail.arguments
    };
  } catch (err) {
    return { success: false, error: err.message };
  }
});

// ---------- Fabric / Forge ----------
ipcMain.handle('install-fabric', async (event, mcVersion) => {
  try {
    const resp = await axios.get(`https://meta.fabricmc.net/v2/versions/loader/${mcVersion}`, {
      timeout: 30000
    });
    const loader = resp.data[0];
    return {
      success: true,
      installerUrl: `https://maven.fabricmc.net/net/fabricmc/fabric-installer/${loader.version}/fabric-installer-${loader.version}.jar`,
      loaderVersion: loader.version,
      mcVersion
    };
  } catch (err) {
    return { success: false, error: err.message };
  }
});

ipcMain.handle('install-forge', async (event, mcVersion) => {
  try {
    const resp = await axios.get(`https://files.minecraftforge.net/maven/net/minecraftforge/forge/index_${mcVersion}.json`, {
      timeout: 30000
    });
    const versions = resp.data.versions || [];
    const latest = versions[0] || null;
    return {
      success: true,
      mcVersion,
      forgeVersion: latest ? latest.version : null,
      url: latest ? `https://files.minecraftforge.net/net/minecraftforge/forge/${mcVersion}-${latest.version}/${mcVersion}-${latest.version}-installer.jar` : null
    };
  } catch (err) {
    return { success: false, error: err.message };
  }
});

// ---------- Modrinth ----------
ipcMain.handle('search-modrinth', async (event, { query, version, loader }) => {
  try {
    const params = { query, limit: 20 };
    if (version) params.versions = JSON.stringify([version]);
    if (loader) params.loaders = JSON.stringify([loader]);

    const resp = await axios.get('https://api.modrinth.com/v2/search', {
      params,
      timeout: 30000,
      headers: { 'User-Agent': 'MatronixLauncher/1.0' }
    });

    const mods = resp.data.hits.map(h => ({
      id: h.project_id,
      name: h.title,
      description: h.description,
      version: h.latest_version,
      gameVersions: h.game_versions,
      loaders: h.loaders,
      source: 'modrinth',
      downloads: h.downloads
    }));

    return { success: true, mods };
  } catch (err) {
    return { success: false, error: err.message };
  }
});

// ---------- CurseForge ----------
ipcMain.handle('search-curseforge', async (event, { query }) => {
  // CurseForge requires an API key. For MVP we tell user to add it.
  return {
    success: false,
    error: 'CurseForge API requires a personal API key. Add it in Settings.'
  };
});

// ---------- Download mod ----------
ipcMain.handle('download-file', async (event, { url, dest }) => {
  try {
    const writer = fs.createWriteStream(dest);
    const resp = await axios({
      method: 'get',
      url,
      responseType: 'stream',
      timeout: 60000
    });
    resp.data.pipe(writer);
    return new Promise((resolve, reject) => {
      writer.on('finish', resolve);
      writer.on('error', reject);
    });
  } catch (err) {
    return { success: false, error: err.message };
  }
});

// ---------- Paths ----------
ipcMain.handle('get-app-path', () => {
  return app.getPath('userData');
});

// ---------- Client token helpers ----------
const TOKEN_FILE = path.join(app.getPath('userData'), 'client_token');
const ACCOUNT_FILE = path.join(app.getPath('userData'), 'account.json');

function getClientToken() {
  if (fs.existsSync(TOKEN_FILE)) {
    return fs.readFileSync(TOKEN_FILE, 'utf8').trim();
  }
  const token = require('crypto').randomBytes(16).toString('hex');
  fs.writeFileSync(TOKEN_FILE, token);
  return token;
}

function saveAccount(account) {
  fs.writeFileSync(ACCOUNT_FILE, JSON.stringify(account, null, 2));
}

function getSavedAccount() {
  if (!fs.existsSync(ACCOUNT_FILE)) return null;
  try {
    return JSON.parse(fs.readFileSync(ACCOUNT_FILE, 'utf8'));
  } catch (e) {
    return null;
  }
}

function clearAccount() {
  if (fs.existsSync(ACCOUNT_FILE)) fs.unlinkSync(ACCOUNT_FILE);
}
