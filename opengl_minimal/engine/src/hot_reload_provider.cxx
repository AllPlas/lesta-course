//
// Created by Алексей Крукович on 9.05.23.
//

#include "hot_reload_provider.hxx"

#include <boost/json.hpp>

#include <algorithm>
#include <fstream>
#include <ranges>
#include <thread>
#include <utility>

namespace json = boost::json;

HotReloadProvider::HotReloadProvider(fs::path path) : m_configPath{ std::move(path) } {}

void HotReloadProvider::check() {
    configFileChanged();
    std::ranges::for_each(m_map, [](auto& reloader) { reloader.second.fn(); });
}

void HotReloadProvider::readFile() {
    std::ifstream in{ m_configPath };
    if (!in.is_open()) throw std::runtime_error{ "Error : extractGame : bad open file"s };

    while (in) {
        std::string buf{};
        std::getline(in, buf);
        if (!in) break;

        m_fileData += buf;
    }
}

void HotReloadProvider::extractGame() {
    json::value value(m_fileData);
    auto game{ value.as_object().find("game") };

    auto reloader{ m_map[game->key()] };
    reloader.path = game->value().as_string().c_str();
}

void HotReloadProvider::extractShaders() {
    json::value value(m_fileData);

    auto vertexShader{ value.as_object().find("vertex_shader") };
    auto reloaderVertex{ m_map[vertexShader->key()] };
    reloaderVertex.path = vertexShader->value().as_string().c_str();

    auto fragmentShader{ value.as_object().find("fragment_shader") };
    auto reloaderFragment{ m_map[fragmentShader->key()] };
    reloaderFragment.path = fragmentShader->value().as_string().c_str();
}

void HotReloadProvider::addToCheck(std::string_view name, std::function<void()> fn) {
    auto reloader{ m_map.at(name.data()) };

    auto function{ [&, f = std::move(fn)]() {
        static fs::file_time_type lastWriteTime{};
        auto currentWriteTime{ fs::last_write_time(reloader.path) };

        if (lastWriteTime == currentWriteTime) return;

        while (lastWriteTime != currentWriteTime) {
            std::this_thread::sleep_for(100ms);

            lastWriteTime = currentWriteTime;
            currentWriteTime = fs::last_write_time(reloader.path);
        }
        f();
    } };

    reloader.fn = std::move(function);
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

    extractGame();
    extractShaders();
}

std::string_view HotReloadProvider::getPath(std::string_view name) const noexcept {
    return m_map.at(name.data()).path.c_str();
}
