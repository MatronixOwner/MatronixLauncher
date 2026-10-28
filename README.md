# MatronixLauncher

MatronixLauncher is a Minecraft launcher project. Its initial implementation is based on the open-source launcher code at [ElyPrismLauncher/Launcher](https://github.com/ElyPrismLauncher/Launcher), which itself builds on Prism Launcher. The upstream launcher code is licensed under GPL-3.0-only; the inherited logo artwork is CC BY-SA 4.0. See `LICENSE`, `program_info/LICENSE`, and `UPSTREAM_README.md` for notices and attribution. MatronixLauncher is an independent project and is not endorsed by Ely.by, Prism Launcher, or Mojang/Microsoft.

## Current direction

- Keep the upstream instance management, Minecraft version support, and vanilla/mod-loader launch flow as the technical foundation.
- Build a MatronixLauncher-branded Windows installer (`MatronixLauncher-Setup.exe`) from CI on code pushes, and publish verified builds as GitHub releases.
- Improve the UI and customization, and retain mod discovery/download integrations for Modrinth and CurseForge.
- Support Ely.by sign-in only with a MatronixLauncher-owned OAuth application.
- Provide configurable endpoints and mirror/fallback behavior. No launcher can promise access in every country: Minecraft, account, and mod services are operated by third parties and can be restricted or unavailable on a user's network.

## Build and release status

The Windows packaging workflow is configured in `.github/workflows/build.yml`; it is intended to produce an unsigned installer until project-owned code-signing is configured. A GitHub Actions build must pass before an installer can be called tested or published. This repository does not yet contain a verified release installer.

Before enabling account and provider integrations in public builds, register project-owned API applications/keys with Ely.by and CurseForge (and Microsoft/Imgur if those optional services are enabled). Their identifiers are deliberately blank in `CMakeLists.txt`: credentials or API keys belonging to upstream projects must not be reused. To enable them in Actions, configure repository variable `MATRONIX_ELY_CLIENT_ID`, secret `MATRONIX_CURSEFORGE_API_KEY`, and optionally `MATRONIX_MSA_CLIENT_ID` / `MATRONIX_IMGUR_CLIENT_ID`. Add credentials only in the repository's GitHub Actions settings—never commit private keys or paste secrets into chat.

## License

Launcher source: GPL-3.0-only. Inherited logo artwork: CC BY-SA 4.0. Preserve the relevant notices and share adaptations of the artwork under the same terms.
