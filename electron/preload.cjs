const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('elyprism', {
  desktop: true,
  openExternal: (url) => ipcRenderer.invoke('open-external', url),
  getMinecraftVersions: () => ipcRenderer.invoke('minecraft-version-manifest'),
  getElyAccount: () => ipcRenderer.invoke('ely-account'),
  loginEly: (credentials) => ipcRenderer.invoke('ely-login', credentials),
  logoutEly: () => ipcRenderer.invoke('ely-logout'),
  installMinecraft: (version, loader, profile) => ipcRenderer.invoke('minecraft-install', { version, loader, profile }),
  launchMinecraft: (version, loader, profile) => ipcRenderer.invoke('minecraft-launch', { version, loader, profile }),
  onMinecraftProgress: (callback) => {
    const listener = (_event, progress) => callback(progress);
    ipcRenderer.on('minecraft-progress', listener);
    return () => ipcRenderer.removeListener('minecraft-progress', listener);
  }
});
