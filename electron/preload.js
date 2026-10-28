const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('api', {
  // Auth
  elyLogin: (data) => ipcRenderer.invoke('ely-login', data),
  elyLogout: () => ipcRenderer.invoke('ely-logout'),
  elyGetAccount: () => ipcRenderer.invoke('ely-get-account'),
  elyOpenRegister: () => ipcRenderer.invoke('ely-open-register'),

  // Versions
  fetchVersions: () => ipcRenderer.invoke('fetch-versions'),
  fetchVersionDetails: (id) => ipcRenderer.invoke('fetch-version-details', id),

  // Loaders
  installFabric: (mcVersion) => ipcRenderer.invoke('install-fabric', mcVersion),
  installForge: (mcVersion) => ipcRenderer.invoke('install-forge', mcVersion),

  // Mods
  searchModrinth: (params) => ipcRenderer.invoke('search-modrinth', params),
  searchCurseForge: (params) => ipcRenderer.invoke('search-curseforge', params),

  // Download
  downloadFile: (opts) => ipcRenderer.invoke('download-file', opts),

  // Paths
  getAppPath: () => ipcRenderer.invoke('get-app-path')
});
