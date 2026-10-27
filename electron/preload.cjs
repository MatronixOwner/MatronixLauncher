const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('elyprism', {
  desktop: true,
  openExternal: (url) => ipcRenderer.invoke('open-external', url),
  getMinecraftVersions: () => ipcRenderer.invoke('minecraft-version-manifest'),
  launchMinecraft: (version, loader) => ipcRenderer.invoke('minecraft-launch', { version, loader }),
  onMinecraftProgress: (callback) => {
    const listener = (_event, progress) => callback(progress);
    ipcRenderer.on('minecraft-progress', listener);
    return () => ipcRenderer.removeListener('minecraft-progress', listener);
  }
});
