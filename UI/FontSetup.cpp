#include "FontSetup.h"

#include <fontconfig/fontconfig.h>
#include <glib.h>
#include <windows.h>

#include <array>
#include <cstdlib>
#include <string>

namespace {
std::string getExecutableDirectory() {
    std::array<char, 32768> buffer{};
    const DWORD length = GetModuleFileNameA(
        nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0 || length >= buffer.size()) {
        g_warning("Could not determine the executable directory for emoji font loading");
        return {};
    }

    const std::string executablePath(buffer.data(), length);
    const size_t separator = executablePath.find_last_of("\\/");
    if (separator == std::string::npos) {
        return ".";
    }
    return executablePath.substr(0, separator);
}
} // namespace

void setupEmojiFont() {
    if (_putenv_s("PANGOCAIRO_BACKEND", "fc") != 0) {
        g_warning("Could not set PANGOCAIRO_BACKEND=fc");
    }

    if (!FcInit()) {
        g_warning("Fontconfig initialization failed; emoji font was not loaded");
        return;
    }

    const std::string executableDirectory = getExecutableDirectory();
    const std::array<std::string, 3> candidates = {
        executableDirectory.empty()
            ? std::string()
            : executableDirectory + "\\UI\\styles\\NotoColorEmoji.ttf",
        "UI/styles/NotoColorEmoji.ttf",
        executableDirectory.empty()
            ? std::string()
            : executableDirectory + "\\NotoColorEmoji.ttf"};

    for (const std::string& path : candidates) {
        if (path.empty() || !g_file_test(path.c_str(), G_FILE_TEST_EXISTS)) {
            continue;
        }

        if (FcConfigAppFontAddFile(
                nullptr, reinterpret_cast<const FcChar8*>(path.c_str()))) {
            g_message("Loaded emoji font: %s", path.c_str());
            return;
        }

        g_warning("Fontconfig could not load emoji font: %s", path.c_str());
    }

    g_warning("NotoColorEmoji.ttf was not found or could not be loaded. "
              "Checked the executable UI\\styles directory, the working "
              "directory UI/styles, and the executable directory.");
}
