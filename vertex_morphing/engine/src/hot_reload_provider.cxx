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

HotReloadProvider::HotReloadProvider(fs::path path) : m_configPath{ std::move(path) } {
    readFile();
    extractGame();
    extractShaders();
    extractBuffers();
}

void HotReloadProvider::check() {
    configFileChanged();
    for (const auto& [name, handler] : m_map) {
        handler.fn();
    }
}

void HotReloadProvider::readFile() {
    std::ifstream in{ m_configPath };
    if (!in.is_open()) throw std::runtime_error{ "Error : extractGame : bad open file"s };

    m_fileData.clear();
    while (in) {
        std::string buf{};
        std::getline(in, buf);
        if (!in) break;

        m_fileData += buf;
    }
}

void HotReloadProvider::extractGame() {
    json::value value(json::parse(m_fileData));
    auto game{ value.as_object().find("game") };

    auto& reloader{ m_map[game->key()] };
    reloader.path = game->value().as_string().c_str();
}

void HotReloadProvider::extractShaders() {
    json::value value(json::parse(m_fileData));

    auto vertexShader{ value.as_object().find("vertex_shader") };
    auto& reloaderVertex{ m_map[vertexShader->key()] };
    reloaderVertex.path = vertexShader->value().as_string().c_str();

    auto fragmentShader{ value.as_object().find("fragment_shader") };
    auto& reloaderFragment{ m_map[fragmentShader->key()] };
    reloaderFragment.path = fragmentShader->value().as_string().c_str();
}

void HotReloadProvider::extractBuffers() {
    json::value value(json::parse(m_fileData));

    auto indicesBuffer{ value.as_object().find("indices") };
    auto& reloaderIndices{ m_map[indicesBuffer->key()] };
    reloaderIndices.path = indicesBuffer->value().as_string().c_str();

    auto verticesBuffer{ value.as_object().find("vertices") };
    auto& reloaderVertices{ m_map[verticesBuffer->key()] };
    reloaderVertices.path = verticesBuffer->value().as_string().c_str();
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
    extractGame();
    extractShaders();
    extractBuffers();
}

std::string_view HotReloadProvider::getPath(std::string_view name) const noexcept {
    return m_map.at(name.data()).path.c_str();
}
