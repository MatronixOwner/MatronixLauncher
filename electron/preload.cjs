const { contextBridge, ipcRenderer } = require('electron');
contextBridge.exposeInMainWorld('elyprism', {
  desktop: true,
  openExternal: (url) => ipcRenderer.invoke('open-external', url),
  getMinecraftVersions: () => ipcRenderer.invoke('minecraft-version-manifest')
});
