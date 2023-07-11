#ifndef ENGINE_PREPARE_TO_GAME_MENU_HXX
#define ENGINE_PREPARE_TO_GAME_MENU_HXX
#include <engine.hxx>

class Menu
{
private:
    bool m_isActive{};
    bool m_isSettingMenu{};
    bool m_isBindKey{};

    std::string m_keybindingMessage{};
    Event::Keyboard::Key* m_bindingKey{};

    std::vector<std::string> m_audioDevices{};
    std::vector<const char*> m_audioDevicesC{};
    int m_selectedAudioDevice{};
    bool m_isRequiredAudioDevicesUpdate{};
    int m_audioVolume{ getEngineInstance()->getAudioVolume() };

    std::vector<const char*> m_videoModes{ "bordered", "fullscreen" };
    int m_selectedVideoMode{ getEngineInstance()->isFullscreen() ? 1 : 0 };

    int m_framerate{ getEngineInstance()->getFramerate() };
    bool m_isVSync{ getEngineInstance()->getVSync() };

public:
    void render();
    [[nodiscard]] bool getActive() const noexcept { return m_isActive; }
    void setActive(bool isActive) noexcept { m_isActive = isActive; }

    [[nodiscard]] bool getSettingMenu() const noexcept { return m_isSettingMenu; }
    void setSettingMenu(bool isSettingMenu) noexcept { m_isSettingMenu = isSettingMenu; }

    [[nodiscard]] bool getBindKey() const noexcept { return m_isBindKey; }
    void setBindKey(bool isBindKey) noexcept { m_isBindKey = isBindKey; }

    void bindKey(const std::string& keyName, Event::Keyboard::Key& key);
};

#endif // ENGINE_PREPARE_TO_GAME_MENU_HXX
