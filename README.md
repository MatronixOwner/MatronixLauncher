# MatronixPrismLauncher

MatronixPrismLauncher is a Minecraft launcher project. Its initial implementation is based on the open-source launcher code at [ElyPrismLauncher/Launcher](https://github.com/ElyPrismLauncher/Launcher), which itself builds on Prism Launcher. The upstream launcher code is licensed under GPL-3.0-only; the inherited logo artwork is CC BY-SA 4.0. See `LICENSE`, `program_info/LICENSE`, and `UPSTREAM_README.md` for notices and attribution. MatronixPrismLauncher is an independent project and is not endorsed by Ely.by, Prism Launcher, or Mojang/Microsoft.

## Current direction

- Keep the upstream instance management, Minecraft version support, and vanilla/mod-loader launch flow as the technical foundation.
- Build a MatronixPrismLauncher-branded Windows installer (`MatronixPrismLauncher-Setup.exe`) from CI on code pushes, and publish verified builds as GitHub releases.
- Use the five owner-provided cat images as selectable home-screen backgrounds.
- Improve the UI and customization; mod discovery, downloads, and managed updates are Modrinth-only.
- Support Ely.by sign-in only with a MatronixPrismLauncher-owned OAuth application.
- Provide configurable endpoints and mirror/fallback behavior. No launcher can promise access in every country: Minecraft, account, and mod services are operated by third parties and can be restricted or unavailable on a user's network.

## Build and release status

The Windows packaging workflow is configured in `.github/workflows/build.yml`; it produces an unsigned installer until project-owned code-signing is configured. Preview releases are published by `.github/workflows/matronix-preview-release.yml`. The launcher uses a black interface by default, with bundled UI, instance, and application icons converted to monochrome grayscale. Ely.by sign-in requires the project's own OAuth Client ID. Set it in **Settings → APIs → Ely.by Authentication** for a local install, or provide `MATRONIX_ELY_CLIENT_ID` as a GitHub Actions repository variable for CI builds. A GitHub Actions build must pass before an installer can be called tested or published.

CurseForge browsing, downloads, and automatic updates are disabled in MatronixPrismLauncher; Modrinth is the sole managed mod source. The upstream CurseForge key is not reused. Use an Ely.by OAuth Client ID registered for MatronixPrismLauncher; never enter or share a client secret.

## License

Launcher source: GPL-3.0-only. Inherited logo artwork: CC BY-SA 4.0. Preserve the relevant notices and share adaptations of the artwork under the same terms.
