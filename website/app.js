(() => {
  const repository = "MatronixOwner/MatronixLauncher";
  const fallbackVersion = "v0.1.0-alpha.10";
  const fallbackInstaller = `https://github.com/${repository}/releases/download/${fallbackVersion}/MatronixPrismLauncher-Setup.exe`;
  const fallbackRelease = `https://github.com/${repository}/releases/tag/${fallbackVersion}`;

  async function refreshReleaseLinks() {
    const installLinks = [...document.querySelectorAll("[data-download]")];
    const releaseLinks = [...document.querySelectorAll("[data-release]")];
    const versionLabels = [...document.querySelectorAll("[data-version]")];
    const updateList = document.querySelector("[data-update-list]");

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

      renderReleaseHistory(releases, updateList);
    } catch (error) {
      // Keep the known-good release URL in the page if the API is unavailable.
      installLinks.forEach(link => { link.href = fallbackInstaller; });
      releaseLinks.forEach(link => { link.href = fallbackRelease; });
      versionLabels.forEach(label => { label.textContent = fallbackVersion; });
      console.info("Using the fallback installer link.", error);
    }
  }

  function renderReleaseHistory(releases, container) {
    if (!container) return;
    const published = releases.filter(release => !release.draft).slice(0, 3);
    if (!published.length) return;

    const fragment = document.createDocumentFragment();
    published.forEach(release => {
      const card = document.createElement("article");
      card.className = "release-card";

      const header = document.createElement("div");
      header.className = "release-card-top";
      const title = document.createElement("strong");
      title.textContent = release.name || release.tag_name;
      const date = document.createElement("time");
      const publishedAt = new Date(release.published_at || release.created_at);
      if (!Number.isNaN(publishedAt.getTime())) {
        date.dateTime = publishedAt.toISOString().slice(0, 10);
        date.textContent = new Intl.DateTimeFormat("ru-RU", { dateStyle: "medium" }).format(publishedAt);
      }
      header.append(title, date);

      const summary = document.createElement("p");
      const plainNotes = (release.body || "Подробности изменений — в заметках релиза.")
        .replace(/\s+/g, " ")
        .trim();
      summary.textContent = plainNotes.length > 260 ? `${plainNotes.slice(0, 257)}…` : plainNotes;

      const link = document.createElement("a");
      link.href = release.html_url;
      link.target = "_blank";
      link.rel = "noreferrer";
      link.append(document.createTextNode("Полные заметки релиза "));
      const arrow = document.createElement("span");
      arrow.setAttribute("aria-hidden", "true");
      arrow.textContent = "↗";
      link.append(arrow);
      card.append(header, summary, link);
      fragment.append(card);
    });
    container.replaceChildren(fragment);
  }

  function enableCatPreview() {
    const buttons = [...document.querySelectorAll("[data-cat-src]")];
    const previewImage = document.querySelector("[data-cat-preview-image]");
    const selectedName = document.querySelector("#selected-cat-name");
    if (!buttons.length || !previewImage || !selectedName) return;

    let selection = 0;
    buttons.forEach(button => {
      button.addEventListener("click", () => {
        const currentSelection = ++selection;
        const nextImage = new Image();
        previewImage.style.opacity = "0";
        nextImage.onload = () => {
          if (currentSelection !== selection) return;
          previewImage.src = nextImage.src;
          previewImage.alt = `${button.dataset.catName} на фоне предпросмотра лаунчера`;
          previewImage.style.opacity = "1";
        };
        nextImage.onerror = () => {
          if (currentSelection === selection) previewImage.style.opacity = "1";
        };
        nextImage.src = button.dataset.catSrc;

        selectedName.textContent = button.dataset.catName;
        buttons.forEach(item => item.setAttribute("aria-pressed", String(item === button)));
      });
    });
  }

  function enableMobileMenu() {
    const button = document.querySelector(".menu-toggle");
    const menu = document.querySelector("#main-nav");
    if (!button || !menu) return;

    const icon = button.querySelector("use");
    const closeMenu = () => {
      button.setAttribute("aria-expanded", "false");
      button.setAttribute("aria-label", "Открыть меню");
      menu.classList.remove("is-open");
      if (icon) icon.setAttribute("href", "assets/lucide-icons.svg#menu");
    };
    button.addEventListener("click", () => {
      const open = button.getAttribute("aria-expanded") !== "true";
      button.setAttribute("aria-expanded", String(open));
      button.setAttribute("aria-label", open ? "Закрыть меню" : "Открыть меню");
      menu.classList.toggle("is-open", open);
      if (icon) icon.setAttribute("href", `assets/lucide-icons.svg#${open ? "x" : "menu"}`);
    });
    menu.addEventListener("click", event => {
      if (event.target.closest("a")) closeMenu();
    });
    document.addEventListener("click", event => {
      if (!menu.contains(event.target) && !button.contains(event.target)) closeMenu();
    });
    document.addEventListener("keydown", event => {
      if (event.key === "Escape") closeMenu();
    });
  }

  function enableScrollReveals() {
    if (!("IntersectionObserver" in window) || window.matchMedia("(prefers-reduced-motion: reduce)").matches)
      return;

    const animated = document.querySelectorAll(
      ".section-heading, .feature-card, .cats-copy, .launcher-preview-wrap, .cat-tile, .project-card, .all-projects-link, .download-copy, .download-side, .release-card, .faq-item, .support-card, .about-copy, .about-details"
    );
    if (!animated.length) return;

    document.body.classList.add("js");
    const observer = new IntersectionObserver((entries, currentObserver) => {
      entries.forEach(entry => {
        if (entry.isIntersecting) {
          entry.target.classList.add("is-visible");
          currentObserver.unobserve(entry.target);
        }
      });
    }, { threshold: 0.12, rootMargin: "0px 0px -35px 0px" });

    animated.forEach(element => {
      element.classList.add("will-reveal");
      observer.observe(element);
    });
  }

  refreshReleaseLinks();
  enableCatPreview();
  enableMobileMenu();
  enableScrollReveals();
})();
