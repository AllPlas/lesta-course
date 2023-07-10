#ifndef __ANDROID__
#    ifndef SDL_ENGINE_EXE_HOTRELOADPROVIDER_HXX
#        define SDL_ENGINE_EXE_HOTRELOADPROVIDER_HXX

#        include <filesystem>
#        include <functional>
#        include <unordered_map>

using namespace std::literals;
namespace fs = std::filesystem;

class HotReloadProvider
{
public:
    template <typename Fn>
    struct Reloader
    {
        std::filesystem::path path{};
        Fn fn{};
    };

private:
    const fs::path m_configPath{};
    std::string m_fileData{};
    std::unordered_map<std::string, Reloader<std::function<void()>>> m_map{};

    inline static fs::path s_path{};

public:
    static void setPath(fs::path path);
    static HotReloadProvider& getInstance();

    void addToCheck(std::string_view name, std::function<void()> fn);
    void check();
    std::string_view getPath(std::string_view name) const noexcept;

private:
    explicit HotReloadProvider(fs::path path);

    void readFile();
    void extractData();

    void configFileChanged();
};

#    endif // SDL_ENGINE_EXE_HOTRELOADPROVIDER_HXX
#endif
