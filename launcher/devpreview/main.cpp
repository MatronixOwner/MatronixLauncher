#include <windows.h>
#include <shellapi.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

namespace {

enum ResourceId : WORD {
    WinterLabHtml = 101,
    WinterLabCss,
    WinterLabJs,
    SantaHatPng,
    SnowmanPng,
    LauncherLogoSvg,
};

bool extractResource(WORD id, const std::filesystem::path& destination)
{
    const HMODULE module = GetModuleHandleW(nullptr);
    const HRSRC resource = FindResourceW(module, MAKEINTRESOURCEW(id), RT_RCDATA);
    if (!resource)
        return false;

    const DWORD size = SizeofResource(module, resource);
    const HGLOBAL loaded = LoadResource(module, resource);
    const void* bytes = loaded ? LockResource(loaded) : nullptr;
    if (!bytes || size == 0)
        return false;

    std::ofstream output(destination, std::ios::binary | std::ios::trunc);
    if (!output)
        return false;
    output.write(static_cast<const char*>(bytes), static_cast<std::streamsize>(size));
    return output.good();
}

bool preparePreview(const std::filesystem::path& root)
{
    std::error_code error;
    std::filesystem::create_directories(root / L"assets" / L"seasonal", error);
    if (error)
        return false;

    return extractResource(WinterLabHtml, root / L"winter-lab.html") &&
           extractResource(WinterLabCss, root / L"winter-lab.css") &&
           extractResource(WinterLabJs, root / L"winter-lab.js") &&
           extractResource(SantaHatPng, root / L"assets" / L"seasonal" / L"santa-hat.png") &&
           extractResource(SnowmanPng, root / L"assets" / L"seasonal" / L"snowman.png") &&
           extractResource(LauncherLogoSvg, root / L"assets" / L"matronixprismlauncher.svg");
}

void showError(const wchar_t* message)
{
    MessageBoxW(nullptr, message, L"Matronix Winter Theme Lab", MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
}

}  // namespace

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    wchar_t localAppData[MAX_PATH] = {};
    const DWORD length = GetEnvironmentVariableW(L"LOCALAPPDATA", localAppData, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) {
        showError(L"Windows did not provide a valid Local AppData folder.");
        return 1;
    }

    const std::filesystem::path previewRoot =
        std::filesystem::path(localAppData) / L"Matronix" / L"WinterThemeLab";
    if (!preparePreview(previewRoot)) {
        showError(L"Could not unpack the preview files. Check access to your Local AppData folder and try again.");
        return 1;
    }

    const std::wstring page = (previewRoot / L"winter-lab.html").wstring();
    const auto result = reinterpret_cast<INT_PTR>(ShellExecuteW(nullptr, L"open", page.c_str(), nullptr, nullptr, SW_SHOWNORMAL));
    if (result <= 32) {
        showError(L"Could not open the preview in your default browser.");
        return 1;
    }
    return 0;
}
