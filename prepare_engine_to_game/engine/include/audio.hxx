#ifndef ENGINE_PREPARE_TO_GAME_AUDIO_HXX
#define ENGINE_PREPARE_TO_GAME_AUDIO_HXX

#include <filesystem>

using namespace std::literals;
namespace fs = std::filesystem;

class Audio final
{
private:
    std::uint8_t* m_start{ nullptr };
    std::uint32_t m_size{};
    std::uint32_t m_currentPos{};

    bool m_isPlaying{};
    bool m_isLooped{};

    std::uint16_t m_format{};
    std::uint8_t m_channels{};
    int m_freq{};

public:
    explicit Audio(const fs::path& path);
    ~Audio();

    void play(bool isLooped = false);

    friend class EngineImpl;
};

#endif // ENGINE_PREPARE_TO_GAME_AUDIO_HXX
