(() => {
  const repository = "d50130483-ship-it/MatronixLauncher";
  const fallbackVersion = "v0.1.0-alpha.9";
  const fallbackInstaller = `https://github.com/${repository}/releases/download/${fallbackVersion}/MatronixPrismLauncher-Setup.exe`;
  const fallbackRelease = `https://github.com/${repository}/releases/tag/${fallbackVersion}`;

  async function refreshReleaseLinks() {
    const installLinks = [...document.querySelectorAll("[data-download]")];
    const releaseLinks = [...document.querySelectorAll("[data-release]")];
    const versionLabels = [...document.querySelectorAll("[data-version]")];

    try {
      const response = await fetch(`https://api.github.com/repos/${repository}/releases?per_page=20`, {
        headers: { Accept: "application/vnd.github+json" },
        cache: "no-cache"
      });
      if (!response.ok) throw new Error(`GitHub release lookup failed: ${response.status}`);
      const releases = await response.json();
      const latest = releases.find(release => {
        if (release.draft) return false;
        return release.assets?.some(asset => asset.name === "MatronixPrismLauncher-Setup.exe");
      });
      if (!latest) throw new Error("No installer asset found in public releases");

      const installer = latest.assets.find(asset => asset.name === "MatronixPrismLauncher-Setup.exe");
      installLinks.forEach(link => { link.href = installer.browser_download_url; });
      releaseLinks.forEach(link => { link.href = latest.html_url; });
      versionLabels.forEach(label => { label.textContent = latest.tag_name; });
    } catch (error) {
      // Keep the known-good release URL in the page if the API is unavailable.
      installLinks.forEach(link => { link.href = fallbackInstaller; });
      releaseLinks.forEach(link => { link.href = fallbackRelease; });
      versionLabels.forEach(label => { label.textContent = fallbackVersion; });
      console.info("Using the fallback installer link.", error);
    }
  }

  refreshReleaseLinks();
})();
