#ifndef ENGINE_PREPARE_TO_GAME_CONFIG_HXX
#define ENGINE_PREPARE_TO_GAME_CONFIG_HXX

#include <engine.hxx>

struct Config
{
    Config() = delete;
    inline static Event::Keyboard::Key ship_move_key{ Event::Keyboard::Key::w };
    inline static Event::Keyboard::Key ship_rotate_left_key{ Event::Keyboard::Key::a };
    inline static Event::Keyboard::Key ship_rotate_right_key{ Event::Keyboard::Key::d };
    inline static Event::Keyboard::Key interact_key{ Event::Keyboard::Key::e };
    inline static Event::Keyboard::Key player_move_up_key{ Event::Keyboard::Key::w };
    inline static Event::Keyboard::Key player_move_left_key{ Event::Keyboard::Key::a };
    inline static Event::Keyboard::Key player_move_right_key{ Event::Keyboard::Key::d };
    inline static Event::Keyboard::Key player_move_down_key{ Event::Keyboard::Key::s };
    inline static Event::Keyboard::Key view_treasure_key{ Event::Keyboard::Key::space };
    inline static Event::Keyboard::Key dig_treasure_key{ Event::Keyboard::Key::f };

    inline static float camera_height{ 1.0f };
};

#endif // ENGINE_PREPARE_TO_GAME_CONFIG_HXX
