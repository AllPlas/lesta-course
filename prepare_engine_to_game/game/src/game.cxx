#include <array>
#include <engine.hxx>
#include <sprite.hxx>
#include <stdexcept>
#include <texture.hxx>

using namespace std::literals;

class PirateGame : public IGame
{
private:
    //  Sprite sprite{ "/Users/aleksey/lesta-course/prepare_engine_to_game/data/tank_dark.png",
    //                  { 84, 92 } };

public:
    void initialize() override {}

    void onEvent(const Event& event) override {}

    void render() const override { getEngineInstance()->getWindowSize(); }

    void update() override {}
};

IGame* createGame(IEngine* engine) {
    if (engine == nullptr) throw std::runtime_error{ "Error : createGame : engine is nullptr"s };
    return new PirateGame{};
}

void destroyGame(IGame* game) {
    if (game == nullptr) throw std::runtime_error{ "Error : destroyGame : game is nullptr"s };
    delete game;
}