#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <chrono>

using namespace geode::prelude;
using Clock = std::chrono::steady_clock;

namespace {
    Clock::time_point g_lastClick = Clock::time_point::min();
    int g_heldButtons = 0;
}

class $modify(SafeClickBGL, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        GJBaseGameLayer::handleButton(down, button, isPlayer1);

        if (down) {
            g_lastClick = Clock::now();
            g_heldButtons++;
        } else if (g_heldButtons > 0) {
            g_heldButtons--;
        }
    }
};

class $modify(SafeClickPL, PlayLayer) {
    void resetLevel() {
        g_heldButtons = 0;
        g_lastClick = Clock::time_point::min();
        PlayLayer::resetLevel();
    }

    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        auto mod = Mod::get();
        if (!mod->getSettingValue<bool>("enabled")) {
            return PlayLayer::destroyPlayer(player, obj);
        }

        // Nunca bloquear el anticheat spike de GD
        if (obj != nullptr && obj == m_anticheatSpike) {
            return PlayLayer::destroyPlayer(player, obj);
        }

        bool holding = mod->getSettingValue<bool>("protect-while-holding") && g_heldButtons > 0;

        auto windowMs = mod->getSettingValue<int64_t>("window-ms");
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now() - g_lastClick
        ).count();
        bool recentClick = g_lastClick != Clock::time_point::min() && elapsed <= windowMs;

        if (holding || recentClick) {
            return; // click activo: no mueres
        }

        PlayLayer::destroyPlayer(player, obj);
    }
};
