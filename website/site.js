(() => {
  const releasePattern = /^v\d+(?:\.\d+){1,3}(?:-[A-Za-z0-9.-]+)?$/;
  const hexColorPattern = /^#[0-9a-f]{6}$/i;
  const releaseAssets = {
    windows: "MatronixPrismLauncher-Setup.exe",
    appImage: "MatronixPrismLauncher-Linux-x86_64.AppImage",
    portable: "MatronixPrismLauncher-Linux-portable.tar.gz",
  };
  const defaults = {
    theme: "matronix-black",
    accentColor: "#a9a9ad",
    eyebrow: "Лаунчер Minecraft Java",
    heroCopy: "Сборки Minecraft. Моды из Modrinth. Ничего лишнего.",
    ctaLabel: "Установить последнюю версию",
    showModrinth: true,
    showFeatures: true,
    showProjects: true,
    showPlatforms: true,
  };

  function applyConfig(config) {
    if (!config || typeof config !== "object") return;

    const version = typeof config.releaseVersion === "string" ? config.releaseVersion.trim() : "";
    if (releasePattern.test(version)) {
      const releaseBase = `https://github.com/MatronixOwner/MatronixLauncher/releases/download/${encodeURIComponent(version)}`;
      document.querySelectorAll("[data-release-asset]").forEach((link) => {
        const filename = releaseAssets[link.dataset.releaseAsset];
        if (filename) link.href = `${releaseBase}/${filename}`;
      });
      document.querySelectorAll("[data-release-version]").forEach((label) => {
        label.textContent = version;
      });
    }

    const customization = { ...defaults, ...(config.customization || {}) };
    const theme = ["matronix-black", "matronix-purple", "graphite"].includes(customization.theme)
      ? customization.theme
      : defaults.theme;
    document.documentElement.dataset.siteTheme = theme;
    document.documentElement.style.setProperty(
      "--green",
      hexColorPattern.test(customization.accentColor) ? customization.accentColor : defaults.accentColor,
    );

    ["eyebrow", "heroCopy", "ctaLabel"].forEach((key) => {
      const value = typeof customization[key] === "string" ? customization[key].slice(0, 180) : defaults[key];
      document.querySelectorAll(`[data-site-field="${key}"]`).forEach((node) => {
        node.textContent = value;
      });
    });

    const visibility = {
      modrinth: customization.showModrinth !== false,
      features: customization.showFeatures !== false,
      projects: customization.showProjects !== false,
      platforms: customization.showPlatforms !== false,
    };
    Object.entries(visibility).forEach(([section, visible]) => {
      document.querySelectorAll(`[data-site-section="${section}"]`).forEach((node) => {
        node.hidden = !visible;
      });
    });

    const text = typeof config.announcement === "string" ? config.announcement.trim().slice(0, 240) : "";
    const announcementEnabled = config.announcementEnabled === true && Boolean(text);
    const announcementState = document.querySelector("[data-announcement-state]");
    if (announcementState) announcementState.textContent = announcementEnabled ? "включён" : "выключен";
    const announcement = document.getElementById("site-announcement");
    if (announcement) {
      announcement.querySelector("p").textContent = text;
      announcement.hidden = !announcementEnabled;
    }
  }

  fetch("site-config.json", { cache: "no-store" })
    .then((response) => {
      if (!response.ok) throw new Error("Site settings unavailable");
      return response.json();
    })
    .then(applyConfig)
    .catch(() => {});

  window.addEventListener("message", (event) => {
    if (event.origin !== window.location.origin || event.source !== window.parent) return;
    if (event.data?.type === "matronix-site-preview") applyConfig(event.data.config);
  });

  if (window.parent !== window && new URLSearchParams(window.location.search).has("admin-preview")) {
    window.parent.postMessage({ type: "matronix-preview-ready" }, window.location.origin);
  }
})();
