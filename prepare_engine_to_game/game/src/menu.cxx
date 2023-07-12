#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-avoid-magic-numbers"
#pragma ide diagnostic ignored "cppcoreguidelines-pro-type-vararg"
#include "menu.hxx"

#include <imgui.h>

#include "config.hxx"

void Menu::render() {
    if (m_isBindKey && m_bindingKey) {
        bindKey(m_keybindingMessage, *m_bindingKey);
        return;
    }

    if (m_isSettingMenu) {
        ImGui::SetNextWindowPos(ImVec2{ 0, 0 });
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Menu",
                     nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        if (m_isRequiredAudioDevicesUpdate) {
            m_audioDevices = getEngineInstance()->getAudioDeviceNames();
            m_audioDevicesC.clear();
            for (auto& name : m_audioDevices)
                m_audioDevicesC.push_back(name.c_str());
            m_selectedAudioDevice =
                static_cast<int>(std::find(m_audioDevices.begin(),
                                           m_audioDevices.end(),
                                           getEngineInstance()->getCurrentAudioDeviceName()) -
                                 m_audioDevices.begin());
            m_isRequiredAudioDevicesUpdate = false;
        }

        if (ImGui::SliderInt("FPS", &m_framerate, 60, 300))
            getEngineInstance()->setFramerate(m_framerate);

        if (ImGui::Checkbox("VSync", &m_isVSync)) getEngineInstance()->setVSync(m_isVSync);

        if (ImGui::Combo("Select an audio device",
                         &m_selectedAudioDevice,
                         m_audioDevicesC.data(),
                         static_cast<int>(m_audioDevicesC.size()))) {
            getEngineInstance()->setAudioDevice(m_audioDevices.at(m_selectedAudioDevice));
        }

        if (ImGui::SliderInt("Audio volume", &m_audioVolume, 0, 128)) {
            getEngineInstance()->setAudioVolume(m_audioVolume);
        }

        if (ImGui::Combo("Select a video mode",
                         &m_selectedVideoMode,
                         m_videoModes.data(),
                         static_cast<int>(m_videoModes.size()))) {
            getEngineInstance()->setFullscreen(m_selectedVideoMode);
        }

        ImGui::SliderFloat("Camera height", &Config::camera_height, 0.5f, 1.3f);

        ImGui::PushID(0);
        ImGui::Text("Ship Move Key: %s", keyToStr(Config::ship_move_key).data());
        ImGui::SameLine();
        if (ImGui::Button("change key")) {
            m_isBindKey = true;
            m_keybindingMessage = "Ship Move Key";
            m_bindingKey = &Config::ship_move_key;
        }
        ImGui::PopID();

        ImGui::PushID(1);
        ImGui::Text("Ship Left Turn Key: %s", keyToStr(Config::ship_rotate_left_key).data());
        ImGui::SameLine();
        if (ImGui::Button("change key")) {
            m_isBindKey = true;
            m_keybindingMessage = "Ship Left Turn Key";
            m_bindingKey = &Config::ship_rotate_left_key;
        }
        ImGui::PopID();

        ImGui::PushID(2);
        ImGui::Text("Ship Right Turn Key: %s", keyToStr(Config::ship_rotate_right_key).data());
        ImGui::SameLine();
        if (ImGui::Button("change key")) {
            m_isBindKey = true;
            m_keybindingMessage = "Ship Right Turn Key";
            m_bindingKey = &Config::ship_rotate_right_key;
        }
        ImGui::PopID();

        ImGui::PushID(3);
        ImGui::Text("Player Move Up Key: %s", keyToStr(Config::player_move_up_key).data());
        ImGui::SameLine();
        if (ImGui::Button("change key")) {
            m_isBindKey = true;
            m_keybindingMessage = "Player Move Up Key";
            m_bindingKey = &Config::player_move_up_key;
        }
        ImGui::PopID();

        ImGui::PushID(4);
        ImGui::Text("Player Move Down Key: %s", keyToStr(Config::player_move_down_key).data());
        ImGui::SameLine();
        if (ImGui::Button("change key")) {
            m_isBindKey = true;
            m_keybindingMessage = "Player Move Down Key";
            m_bindingKey = &Config::player_move_down_key;
        }
        ImGui::PopID();

        ImGui::PushID(5);
        ImGui::Text("Player Move Left Key: %s", keyToStr(Config::player_move_left_key).data());
        ImGui::SameLine();
        if (ImGui::Button("change key")) {
            m_isBindKey = true;
            m_keybindingMessage = "Player Move left Key";
            m_bindingKey = &Config::player_move_left_key;
        }
        ImGui::PopID();

        ImGui::PushID(6);
        ImGui::Text("Player Move Right Key: %s", keyToStr(Config::player_move_right_key).data());
        ImGui::SameLine();
        if (ImGui::Button("change key")) {
            m_isBindKey = true;
            m_keybindingMessage = "Player Move right Key";
            m_bindingKey = &Config::player_move_right_key;
        }
        ImGui::PopID();

        ImGui::PushID(7);
        ImGui::Text("Go Ashore/Get Onboard Key: %s", keyToStr(Config::interact_key).data());
        ImGui::SameLine();
        if (ImGui::Button("change key")) {
            m_isBindKey = true;
            m_keybindingMessage = "Go Ashore/Get Onboard Key";
            m_bindingKey = &Config::interact_key;
        }
        ImGui::PopID();

        ImGui::PushID(8);
        ImGui::Text("Open/Close Map Key: %s", keyToStr(Config::view_treasure_key).data());
        ImGui::SameLine();
        if (ImGui::Button("change key")) {
            m_isBindKey = true;
            m_keybindingMessage = "Open/Close Map Key";
            m_bindingKey = &Config::view_treasure_key;
        }
        ImGui::PopID();

        ImGui::PushID(9);
        ImGui::Text("Dig Key: %s", keyToStr(Config::dig_treasure_key).data());
        ImGui::SameLine();
        if (ImGui::Button("change key")) {
            m_isBindKey = true;
            m_keybindingMessage = "Dig Key";
            m_bindingKey = &Config::dig_treasure_key;
        }
        ImGui::PopID();

        ImGui::End();
        return;
    }

    ImGui::SetNextWindowPos(ImVec2{ 0, 0 });
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("Menu",
                 nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse);

    const float buttonWidthRatio = 0.3f;
    const float buttonHeightRatio = 0.1f;

    ImVec2 windowSize = ImGui::GetWindowSize();

    float buttonWidth = windowSize.x * buttonWidthRatio;
    float buttonHeight = windowSize.y * buttonHeightRatio;

    float buttonSpacing = (windowSize.y - (buttonHeight * 7)) / 2.0f - 10.f;

    float posX = (windowSize.x - buttonWidth) * 0.5f;

    ImGui::SetCursorPosY(buttonSpacing);

    ImGui::SetCursorPosX(posX);

    if (ImGui::Button("Play", ImVec2(buttonWidth, buttonHeight))) { m_isActive = false; }
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + buttonHeight);

    ImGui::SetCursorPosX(posX);

    if (ImGui::Button("Reset Progress", ImVec2(buttonWidth, buttonHeight))) {}
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + buttonHeight);

    ImGui::SetCursorPosX(posX);

    if (ImGui::Button("Settings", ImVec2(buttonWidth, buttonHeight))) {
        m_isSettingMenu = true;
        m_isRequiredAudioDevicesUpdate = true;
    }
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + buttonHeight);

    ImGui::SetCursorPosX(posX);

    if (ImGui::Button("Exit", ImVec2(buttonWidth, buttonHeight))) {}

    ImGui::End();
}

void Menu::bindKey(const std::string& keyName, Event::Keyboard::Key& bindingKey) {
    ImGui::Begin("Menu",
                 nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse);

    ImGui::Text("%s", keyName.c_str());
    ImGui::Text("Waiting for keyboard input...");
    for (ImGuiKey key{ ImGuiKey_NamedKey_BEGIN }; key < ImGuiKey_NamedKey_END;
         key = static_cast<ImGuiKey>(key + 1)) {
        if (ImGui::IsKeyPressed(key)) {
            if (auto eventKey{ ImGuiKeyToEventKey(key) };
                eventKey != Event::Keyboard::Key::not_key &&
                eventKey != Event::Keyboard::Key::escape)
                bindingKey = eventKey;

            m_isBindKey = false;
            break;
        }
    }

    ImGui::End();
}

#pragma clang diagnostic pop