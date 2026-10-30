# MatronixPrismLauncher website

This is a static, dependency-free landing page. GitHub Pages serves the files in this directory; the deployment workflow is `.github/workflows/deploy-website.yml`.

- Intended URL: <https://matronixowner.github.io/MatronixLauncher/>
- Hosting: GitHub Pages (free for this public repository; no web server or database to keep running).
- The download buttons query the public GitHub Releases API for the newest release with `MatronixPrismLauncher-Setup.exe`. A pinned installer URL is kept as a fallback.
- The launcher preview on the page is an interactive design mockup, not a screenshot of a running build. No authentic launcher screenshots are checked into this repository.
- The cat gallery switches the mockup artwork client-side; its image files live in `assets/`. Project cards link to public MatronixOwner GitHub repositories.
- A custom domain is optional. If one is added, its owner must keep the domain registration renewed; the default GitHub Pages address does not require a domain purchase.
- In repository **Settings → Pages**, set **Build and deployment → Source** to **GitHub Actions**. Later updates to this directory deploy automatically.

GitHub Pages and GitHub Releases are third-party services, so neither can be guaranteed to operate forever. Keeping the site source and installers in GitHub makes the site straightforward to redeploy or migrate if hosting needs to change.
