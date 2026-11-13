(() => {
  const preview = document.getElementById("mock-window");
  const toggle = document.getElementById("snow-toggle");
  const snow = document.getElementById("snow-layer");
  const dateLabel = document.getElementById("system-date");

  if (dateLabel) {
    const today = new Date();
    dateLabel.textContent = `Системная дата: ${new Intl.DateTimeFormat("ru-RU", { day: "numeric", month: "long", year: "numeric" }).format(today)}`;
  }

  if (snow) {
    for (let i = 0; i < 19; i += 1) {
      const flake = document.createElement("i");
      flake.className = "snowflake";
      flake.style.left = `${8 + ((i * 47) % 91)}%`;
      flake.style.animationDuration = `${12 + ((i * 7) % 13)}s`;
      flake.style.animationDelay = `${-((i * 11) % 24)}s`;
      flake.style.setProperty("--drift", `${((i % 5) - 2) * 17}px`);
      snow.append(flake);
    }
  }

  toggle?.addEventListener("click", () => {
    const enabled = toggle.getAttribute("aria-pressed") !== "true";
    toggle.setAttribute("aria-pressed", String(enabled));
    preview?.classList.toggle("no-snow", !enabled);
    toggle.querySelector("span:last-child").textContent = enabled ? "Снег включён" : "Снег выключен";
  });

  document.querySelectorAll(".sample-card").forEach((card) => {
    card.tabIndex = 0;
    card.setAttribute("role", "button");
    card.setAttribute("aria-pressed", String(card.classList.contains("selected")));
    const select = () => {
      document.querySelectorAll(".sample-card").forEach((item) => {
        item.classList.remove("selected");
        item.setAttribute("aria-pressed", "false");
      });
      card.classList.add("selected");
      card.setAttribute("aria-pressed", "true");
    };
    card.addEventListener("click", select);
    card.addEventListener("keydown", (event) => {
      if (event.key === "Enter" || event.key === " ") {
        event.preventDefault();
        select();
      }
    });
  });
})();
