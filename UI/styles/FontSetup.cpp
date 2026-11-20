#include "FontSetup.h"
#include <fontconfig/fontconfig.h>
#include <glib.h>
#include <cstdlib>
#include <string>
#include <windows.h>

static std::string exeDir() {
    char buf[MAX_PATH];
    DWORD n = GetModuleFileNameA(nullptr, buf, MAX_PATH);
    std::string p(buf, n);
    return p.substr(0, p.find_last_of("\\/"));
}

void setupEmojiFont() {
    // Pango trên Windows mặc định dùng backend win32 (emoji màu kém).
    // Ép dùng backend fontconfig để nhận được font nạp thêm.
    _putenv_s("PANGOCAIRO_BACKEND", "fc");

    FcInit();
    const std::string candidates[] = {
        exeDir() + "\\assets\\fonts\\NotoColorEmoji.ttf",
        "assets/fonts/NotoColorEmoji.ttf"};
    for (const auto& path : candidates) {
        if (g_file_test(path.c_str(), G_FILE_TEST_EXISTS) &&
            FcConfigAppFontAddFile(nullptr, (const FcChar8*)path.c_str())) {
            g_message("Loaded emoji font: %s", path.c_str());
            return;
        }
    }
    g_warning("Khong tim thay NotoColorEmoji.ttf");
}
