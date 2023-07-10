#include "menu.hxx"

#include <imgui.h>

#include "config.hxx"

void Menu::render() {
    if (m_isBindKey) {
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
        ImGui::SliderFloat("Camera height", &Config::camera_height, 0.5f, 1.25f);

        auto audioDevices{ getEngineInstance()->getAudioDeviceNames() };
        static int selectedAudioDevice =
            std::find(audioDevices.begin(),
                      audioDevices.end(),
                      getEngineInstance()->getCurrentAudioDeviceName()) -
            audioDevices.begin();

        if (ImGui::Combo("Select an option",
                         &selectedAudioDevice,
                         audioDevices.data()->data(),
                         audioDevices.size())) {
            getEngineInstance()->setAudioDevice(audioDevices.at(selectedAudioDevice));
        }

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

    // Получите размер окна ImGui
    ImVec2 windowSize = ImGui::GetWindowSize();

    // Рассчитайте размеры кнопок относительно размера окна
    float buttonWidth = windowSize.x * buttonWidthRatio;
    float buttonHeight = windowSize.y * buttonHeightRatio;

    // Рассчитайте промежуток между кнопками
    float buttonSpacing = (windowSize.y - (buttonHeight * 7)) / 2.0f - 10.f;

    // Рассчитайте позицию по центру горизонтально
    float posX = (windowSize.x - buttonWidth) * 0.5f;

    // Установите позицию курсора по вертикали перед первой кнопкой
    ImGui::SetCursorPosY(buttonSpacing);

    // Установите позицию курсора по горизонтали перед каждой кнопкой
    ImGui::SetCursorPosX(posX);

    // Отрисуйте кнопки
    if (ImGui::Button("Play", ImVec2(buttonWidth, buttonHeight))) { m_isActive = false; }
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + buttonHeight);

    ImGui::SetCursorPosX(posX);

    if (ImGui::Button("Reset Progress", ImVec2(buttonWidth, buttonHeight))) {
        // Действия при нажатии на кнопку 2
    }
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + buttonHeight);

    ImGui::SetCursorPosX(posX);

    if (ImGui::Button("Settings", ImVec2(buttonWidth, buttonHeight))) { m_isSettingMenu = true; }
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + buttonHeight);

    ImGui::SetCursorPosX(posX);

    if (ImGui::Button("Exit", ImVec2(buttonWidth, buttonHeight))) {
        // Действия при нажатии на кнопку 4
    }

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
