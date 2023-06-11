#include <stdexcept>
#include <array>
#include <thread>
#include <iostream>
#include <iomanip>
#include <engine.hxx>

using namespace std::literals;

class ConsoleGame : public IGame
{
private:
  int m_percent{};
  char m_symbol{ '#' };
  std::string m_loadingPanel{};

  struct Config
  {
    Config() = delete;
    inline static constexpr int maxPercent{ 100 };
    inline static constexpr int difPercentStrSize{ 2 };
  };

public:
  void initialize() override {}
  void onEvent(const Event& event) override {}

  void update() override {
    if (m_percent == Config::maxPercent) {
      m_loadingPanel.clear();
      m_percent = 0;
    }
    m_loadingPanel += m_symbol;
    m_percent += Config::difPercentStrSize;
    std::this_thread::sleep_for(100ms);
  }

  void render() const override {
    std::system("clear || cls");
    std::cout << '[' << std::setw(Config::maxPercent / Config::difPercentStrSize) << std::left
              << m_loadingPanel << "]  "sv << m_percent << '%' << std::endl;
  }
};

IGame* createGame(IEngine* engine) {
  if (engine == nullptr) throw std::runtime_error{ "Error : createGame : engine is nullptr"s };
  return new ConsoleGame{};
}

void destroyGame(IGame* game) {
  if (game == nullptr) throw std::runtime_error{ "Error : destroyGame : game is nullptr"s };
  delete game;
}