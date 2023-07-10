#ifndef __ANDROID__
#    include "hot_reload_provider.hxx"

#    include <boost/json.hpp>

#    include <algorithm>
#    include <fstream>
#    include <ranges>
#    include <thread>
#    include <utility>

namespace json = boost::json;

HotReloadProvider::HotReloadProvider(fs::path path) : m_configPath{ std::move(path) } {
    readFile();
    extractData();
}

void HotReloadProvider::check() {
    configFileChanged();
    std::ranges::for_each(m_map, [](const auto& el) {
        if (el.second.fn) el.second.fn();
    });
}

void HotReloadProvider::readFile() {
    std::ifstream in{ m_configPath };
    if (!in.is_open())
        throw std::runtime_error{ "Error : HotReloadProvider::readFile : bad open file"s };

    m_fileData.clear();
    while (in) {
        std::string buf{};
        std::getline(in, buf);
        if (!in) break;

        m_fileData += buf;
    }
}

void HotReloadProvider::addToCheck(std::string_view name, std::function<void()> fn) {
    auto& reloader{ m_map.at(name.data()) };

    auto functionFactory{ [&reloader, fn = std::move(fn)]() mutable {
        static fs::file_time_type lastWriteTime{};
        return [lastWriteTime = lastWriteTime, &reloader, fn = std::move(fn)]() mutable {
            auto currentWriteTime{ fs::last_write_time(reloader.path) };

            if (lastWriteTime == currentWriteTime) return;

            while (lastWriteTime != currentWriteTime) {
                std::this_thread::sleep_for(100ms);

                lastWriteTime = currentWriteTime;
                currentWriteTime = fs::last_write_time(reloader.path);
            }
            fn();
        };
    } };

    reloader.fn = functionFactory();
}

void HotReloadProvider::configFileChanged() {
    static fs::file_time_type lastWriteTime{};
    auto currentWriteTime{ fs::last_write_time(m_configPath) };

    if (lastWriteTime == currentWriteTime) return;

    while (lastWriteTime != currentWriteTime) {
        std::this_thread::sleep_for(100ms);

        lastWriteTime = currentWriteTime;
        currentWriteTime = fs::last_write_time(m_configPath);
    }

    readFile();
    extractData();
}

std::string_view HotReloadProvider::getPath(std::string_view name) const noexcept {
#    ifndef _WIN32
    return m_map.at(name.data()).path.c_str();
#    else
    return m_map.at(name.data()).path.string().c_str();
#    endif
}

void HotReloadProvider::extractData() {
    auto value(json::parse(m_fileData));

    for (const auto& [name, path] : value.as_object()) {
        fs::path fullPath{};
        if (!path.as_string().starts_with('/') || path.as_string()[1] != ':')
            fullPath += m_configPath.parent_path() / "";
        fullPath += path.as_string().c_str();
        m_map[name].path = fullPath;
    }
}

void HotReloadProvider::setPath(fs::path path) { s_path = std::move(path); }

HotReloadProvider& HotReloadProvider::getInstance() {
    static HotReloadProvider provider{ std::move(s_path) };

    return provider;
}
#endif