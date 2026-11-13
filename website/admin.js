(() => {
  const DRAFT_KEY = "matronix-site-admin-draft-v1";
  const releasePattern = /^v\d+(?:\.\d+){1,3}(?:-[A-Za-z0-9.-]+)?$/;
  const defaults = {
    releaseVersion: "v0.1.0-alpha.11",
    announcementEnabled: false,
    announcement: "",
    customization: {
      theme: "matronix-black",
      accentColor: "#a9a9ad",
      eyebrow: "Лаунчер Minecraft Java",
      heroCopy: "Сборки Minecraft. Моды из Modrinth. Ничего лишнего.",
      ctaLabel: "Установить последнюю версию",
      showModrinth: true,
      showFeatures: true,
      showProjects: true,
      showPlatforms: true,
    },
  };

  const byId = (id) => document.getElementById(id);
  const frame = byId("site-preview");
  const frameWrap = byId("preview-frame-wrap");
  const badge = byId("draft-state");
  const toast = byId("toast");
  let publishedConfig = JSON.parse(JSON.stringify(defaults));
  let config = JSON.parse(JSON.stringify(defaults));
  let toastTimer = 0;

  const clone = (value) => JSON.parse(JSON.stringify(value));
  const mergeConfig = (base, incoming) => ({
    ...base,
    ...(incoming && typeof incoming === "object" ? incoming : {}),
    customization: {
      ...base.customization,
      ...(incoming?.customization && typeof incoming.customization === "object" ? incoming.customization : {}),
    },
  });

  function showToast(message) {
    toast.textContent = message;
    toast.classList.add("is-visible");
    window.clearTimeout(toastTimer);
    toastTimer = window.setTimeout(() => toast.classList.remove("is-visible"), 4600);
  }

  function sendPreview() {
    if (frame.contentWindow) {
      frame.contentWindow.postMessage({ type: "matronix-site-preview", config }, window.location.origin);
    }
  }

  function fillForm() {
    const custom = config.customization;
    byId("theme").value = custom.theme;
    byId("accent-color").value = custom.accentColor;
    byId("eyebrow").value = custom.eyebrow;
    byId("hero-copy").value = custom.heroCopy;
    byId("cta-label").value = custom.ctaLabel;
    byId("release-version").value = config.releaseVersion;
    byId("announcement-enabled").checked = config.announcementEnabled;
    byId("announcement").value = config.announcement;
    byId("show-modrinth").checked = custom.showModrinth;
    byId("show-features").checked = custom.showFeatures;
    byId("show-projects").checked = custom.showProjects;
    byId("show-platforms").checked = custom.showPlatforms;
    byId("announcement-count").textContent = String(config.announcement.length);
  }

  function readForm() {
    config.customization = {
      ...config.customization,
      theme: byId("theme").value,
      accentColor: byId("accent-color").value.toLowerCase(),
      eyebrow: byId("eyebrow").value.trim(),
      heroCopy: byId("hero-copy").value.trim(),
      ctaLabel: byId("cta-label").value.trim(),
      showModrinth: byId("show-modrinth").checked,
      showFeatures: byId("show-features").checked,
      showProjects: byId("show-projects").checked,
      showPlatforms: byId("show-platforms").checked,
    };
    config.releaseVersion = byId("release-version").value.trim();
    config.announcementEnabled = byId("announcement-enabled").checked;
    config.announcement = byId("announcement").value.trim().slice(0, 240);
    byId("announcement-count").textContent = String(config.announcement.length);
  }

  function updateDraft() {
    readForm();
    sendPreview();
    try {
      localStorage.setItem(DRAFT_KEY, JSON.stringify(config));
      badge.textContent = "Черновик сохранён";
      badge.classList.add("is-saved");
    } catch (_) {
      badge.textContent = "Черновик в памяти";
      badge.classList.remove("is-saved");
    }
    const validRelease = releasePattern.test(config.releaseVersion);
    byId("release-version").setCustomValidity(validRelease ? "" : "Укажи тег формата v0.1.0-alpha.11");
    byId("preview-message").textContent = validRelease ? "Предпросмотр обновлён · изменения ещё не опубликованы" : "Проверь формат тега релиза";
  }

  function downloadConfig() {
    readForm();
    const blob = new Blob([`${JSON.stringify(config, null, 2)}\n`], { type: "application/json" });
    const url = URL.createObjectURL(blob);
    const link = document.createElement("a");
    link.href = url;
    link.download = "site-config.json";
    link.click();
    URL.revokeObjectURL(url);
    showToast("Конфигурация сайта скачана. Предпросмотр не означает публикацию.");
  }

  async function preparePublish() {
    readForm();
    if (!releasePattern.test(config.releaseVersion)) {
      byId("release-version").reportValidity();
      return;
    }
    const actionTab = window.open("https://github.com/MatronixOwner/MatronixLauncher/actions/workflows/deploy-website.yml", "_blank");
    if (actionTab) actionTab.opener = null;
    const payload = JSON.stringify(config);
    const fallback = byId("config-copy-fallback");
    try {
      await navigator.clipboard.writeText(payload);
      showToast(actionTab ? "JSON скопирован. В GitHub Actions выбери ветку arena/1e9c3b0a-matronixlauncher, вставь JSON в site_config_json и запусти публикацию." : "JSON скопирован. Разреши всплывающие окна и открой workflow site-admin в GitHub.");
    } catch (_) {
      fallback.hidden = false;
      fallback.value = payload;
      fallback.focus();
      fallback.select();
      showToast("Скопируй выделенный JSON, затем вставь его в поле site_config_json в GitHub Actions.");
    }
  }

  document.querySelectorAll("#site-editor input, #site-editor textarea, #site-editor select").forEach((input) => {
    input.addEventListener("input", updateDraft);
    input.addEventListener("change", updateDraft);
  });

  byId("reset-draft").addEventListener("click", () => {
    config = clone(publishedConfig);
    localStorage.removeItem(DRAFT_KEY);
    fillForm();
    sendPreview();
    badge.textContent = "Опубликованная версия";
    badge.classList.remove("is-saved");
    byId("preview-message").textContent = "Черновик сброшен к опубликованной версии";
  });
  byId("export-config").addEventListener("click", downloadConfig);
  byId("publish-config").addEventListener("click", preparePublish);

  document.querySelectorAll("[data-device]").forEach((button) => {
    button.addEventListener("click", () => {
      document.querySelectorAll("[data-device]").forEach((item) => item.classList.toggle("is-active", item === button));
      frameWrap.classList.toggle("is-mobile", button.dataset.device === "mobile");
    });
  });

  frame.addEventListener("load", sendPreview);
  window.addEventListener("message", (event) => {
    if (event.origin !== window.location.origin || event.source !== frame.contentWindow) return;
    if (event.data?.type === "matronix-preview-ready") sendPreview();
  });

  fetch("site-config.json", { cache: "no-store" })
    .then((response) => {
      if (!response.ok) throw new Error("Published settings unavailable");
      return response.json();
    })
    .then((loaded) => {
      publishedConfig = mergeConfig(defaults, loaded);
      config = clone(publishedConfig);
      let restoredDraft = false;
      try {
        const saved = localStorage.getItem(DRAFT_KEY);
        if (saved) {
          config = mergeConfig(publishedConfig, JSON.parse(saved));
          restoredDraft = true;
        }
      } catch (_) {
        localStorage.removeItem(DRAFT_KEY);
      }
      fillForm();
      sendPreview();
      badge.textContent = restoredDraft ? "Черновик восстановлен" : "Опубликованная версия";
      badge.classList.toggle("is-saved", restoredDraft);
    })
    .catch(() => {
      fillForm();
      sendPreview();
      showToast("Не удалось загрузить опубликованные настройки сайта.");
    });
})();
