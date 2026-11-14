<div align="center">
  <img src="program_info/com.matronixprismlauncher.MatronixPrismLauncher_256.png" alt="MatronixPrismLauncher logo" width="112" />
  <h1>MatronixPrismLauncher</h1>
  <p><strong>A customizable Minecraft launcher, shaped for the Matronix community.</strong></p>
  <p>
    <a href="https://github.com/MatronixOwner/MatronixLauncher/releases"><img alt="GitHub Releases" src="https://img.shields.io/github/v/release/MatronixOwner/MatronixLauncher?include_prereleases&style=flat-square&label=release"></a>
    <a href="https://github.com/MatronixOwner/MatronixLauncher/actions/workflows/build.yml"><img alt="Build workflow" src="https://img.shields.io/github/actions/workflow/status/MatronixOwner/MatronixLauncher/build.yml?branch=main&style=flat-square&label=build"></a>
    <a href="https://matronixowner.github.io/MatronixLauncher/"><img alt="Project website" src="https://img.shields.io/badge/website-Matronix-8a5cf6?style=flat-square"></a>
  </p>
  <p><a href="https://github.com/MatronixOwner/MatronixLauncher/releases">Download a release</a> · <a href="https://matronixowner.github.io/MatronixLauncher/">Visit the website</a></p>
</div>

---

MatronixPrismLauncher is an independent Minecraft launcher project based on the open-source code at [ElyPrismLauncher/Launcher](https://github.com/ElyPrismLauncher/Launcher), which itself builds on Prism Launcher. It keeps the upstream launch and instance-management foundations while developing a distinct Matronix-branded experience.

> **Independence notice:** MatronixPrismLauncher is not endorsed by or affiliated with Ely.by, Prism Launcher, Mojang, or Microsoft.

## Highlights

- **Manage Minecraft instances** and launch vanilla or mod-loader profiles using the upstream launcher foundation.
- **Discover and manage mods through Modrinth.** CurseForge browsing, downloads, and automatic updates are disabled; no upstream CurseForge API key is reused.
- **Make the launcher yours** with themes, icons, original cat backgrounds, and user-imported image packs.
- **Use builds for your platform.** CI packages Windows installers and Linux AppImage/portable archives. The Windows installer is currently unsigned.
- **Configure service endpoints and mirrors.** Third-party account, Minecraft, and mod services may be unavailable or restricted depending on network and region; no launcher can guarantee access everywhere.

## Video backgrounds

In **Settings → Appearance**, choose a local video and apply the settings. Press the video button next to the cat to start it. It plays silently in a loop behind the instance list; the button shows a stop icon when the video is on and a play icon when it is off. When both backgrounds are enabled, the video covers the cat background. Playback depends on the video codecs available on your system.

## Downloads

Get published builds from the [GitHub Releases page](https://github.com/MatronixOwner/MatronixLauncher/releases). Preview releases are produced by the [`matronix-preview-release` workflow](.github/workflows/matronix-preview-release.yml). Check each release's notes for available platforms and installation guidance.

The project website is hosted at [matronixowner.github.io/MatronixLauncher](https://matronixowner.github.io/MatronixLauncher/).

## Accounts and configuration

Ely.by sign-in uses an OAuth Client ID registered for MatronixPrismLauncher. The default ID is `matronixprismlauncher`; it can be changed in **Settings → APIs → Ely.by Authentication** or with the `MATRONIX_ELY_CLIENT_ID` GitHub Actions repository variable. Use a project-owned OAuth application. **Never enter or share an OAuth client secret.**

## Build and release status

GitHub Actions workflows build and package the launcher. A successful build workflow is required before an installer should be described as tested or published. Linux AppImage packaging uses XDG desktop portals for desktop integration, including KDE Plasma.

## Attribution and licenses

The launcher source is licensed under **GPL-3.0-only**. Inherited logo artwork is licensed under **CC BY-SA 4.0**; adaptations of that artwork must retain the relevant attribution and share-alike terms. See [`LICENSE`](LICENSE), [`program_info/LICENSE`](program_info/LICENSE), and [`UPSTREAM_README.md`](UPSTREAM_README.md) for licensing details and upstream notices.

The project credits [MatronixOwner](https://github.com/MatronixOwner). The launcher also includes original cat backgrounds, three CC BY-SA 4.0 cat images from Wikimedia Commons, and support for user-provided image packs.
