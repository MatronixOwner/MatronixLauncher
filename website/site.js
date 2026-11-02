(() => {
  const releasePattern = /^v\d+(?:\.\d+){1,3}(?:-[A-Za-z0-9.-]+)?$/;
  const releaseAssets = {
    windows: "MatronixPrismLauncher-Setup.exe",
    appImage: "MatronixPrismLauncher-Linux-x86_64.AppImage",
    portable: "MatronixPrismLauncher-Linux-portable.tar.gz",
  };

  fetch("site-config.json", { cache: "no-store" })
    .then((response) => {
      if (!response.ok) throw new Error("Site settings unavailable");
      return response.json();
    })
    .then((config) => {
      const version = typeof config.releaseVersion === "string" ? config.releaseVersion : "";
      if (!releasePattern.test(version)) return;

      const releaseBase = `https://github.com/MatronixOwner/MatronixLauncher/releases/download/${encodeURIComponent(version)}`;
      document.querySelectorAll("[data-release-asset]").forEach((link) => {
        const filename = releaseAssets[link.dataset.releaseAsset];
        if (filename) link.href = `${releaseBase}/${filename}`;
      });
      document.querySelectorAll("[data-release-version]").forEach((label) => {
        label.textContent = version;
      });

      const text = typeof config.announcement === "string" ? config.announcement.trim() : "";
      const announcementState = document.querySelector("[data-announcement-state]");
      if (announcementState) {
        announcementState.textContent = config.announcementEnabled === true && text ? "включён" : "выключен";
      }
      if (config.announcementEnabled === true && text) {
        const announcement = document.getElementById("site-announcement");
        if (announcement) {
          announcement.querySelector("p").textContent = text;
          announcement.hidden = false;
        }
      }
    })
    .catch(() => {
      // The release links in the HTML remain usable if the optional config is unavailable.
    });
})();
