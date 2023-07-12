#include "map.hxx"

#include <random>

#include "engine.hxx"

static int generateRandomNumber(int min, int max) {
    static std::seed_seq seed{
        std::random_device{}(),
        static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count())
    };

    static std::mt19937_64 engine{ seed };

    std::uniform_int_distribution die{ min, max };
    return die(engine);
}

Map::Map(const fs::path& waterTexturePath,
         const fs::path& airTexturePath,
         const fs::path& bottleTexturePath,
         const fs::path& treasureTexturePath,
         const fs::path& xMarkTexturePath,
         Size textureSize,
         Size mapSize)
    : m_waterSprite{ waterTexturePath, textureSize }
    , m_airSprite{ airTexturePath, textureSize }
    , m_bottle{ bottleTexturePath, textureSize }
    , m_treasure{ treasureTexturePath, xMarkTexturePath, textureSize }
    , m_textureSize{ textureSize }
    , m_mapSize{ mapSize } {
    float xOffset = -((800 / 2.0f) - (m_textureSize.width / 2.0f));
    float yOffset = -((600 / 2.0f) - (m_textureSize.height / 2.0f));
    for (std::ptrdiff_t h{}; h < m_mapSize.height / m_textureSize.height; ++h) {
        for (std::ptrdiff_t w{}; w < m_mapSize.width / m_textureSize.width; ++w) {
            float xPos = xOffset + (w * m_textureSize.width);
            float yPos = yOffset + (h * m_textureSize.height);
            m_waterPositions.push_back({ xPos, yPos });
        }
    }

    std::vector<Vertex2> vertices{};
    std::vector<std::uint32_t> indices{};
    for (auto pos : m_waterPositions) {
        auto leftXpos{ (pos.x + 400) - textureSize.width / 2.0f };
        auto rightXpos{ (pos.x + 400) + textureSize.width / 2.0f };
        auto topYpos{ (pos.y + 300) + textureSize.height / 2.0f };
        auto bottomYpos{ (pos.y + 300) - textureSize.height / 2.0f };

        auto normalizedXLeftPos{ (leftXpos / (800.f * 0.5f)) - 1.0f };
        auto normalizedXRightPos{ (rightXpos / (800.f * 0.5f)) - 1.0f };
        auto normalizedYTopPos{ (topYpos / (600.f * 0.5f)) - 1.0f };
        auto normalizedYBottomPos{ (bottomYpos / (600.f * 0.5f)) - 1.0f };

        Vertex2 v1{};
        v1.x = normalizedXLeftPos;
        v1.y = normalizedYTopPos;
        v1.texX = 0.0;
        v1.texY = 0.0;

        vertices.push_back(v1);
        auto firstIdx{ vertices.size() - 1 };

        Vertex2 v2{};
        v2.x = normalizedXRightPos;
        v2.y = normalizedYTopPos;
        v2.texX = 1.0;
        v2.texY = 0.0;
        vertices.push_back(v2);
        auto secondIdx{ vertices.size() - 1 };

        Vertex2 v3{};
        v3.x = normalizedXRightPos;
        v3.y = normalizedYBottomPos;
        v3.texX = 1.0;
        v3.texY = 1.0;
        vertices.push_back(v3);
        auto thirdIdx{ vertices.size() - 1 };

        Vertex2 v4{};
        v4.x = normalizedXLeftPos;
        v4.y = normalizedYBottomPos;
        v4.texX = 0.0;
        v4.texY = 1.0;
        vertices.push_back(v4);
        auto fourthIdx{ vertices.size() - 1 };

        indices.push_back(firstIdx);
        indices.push_back(secondIdx);
        indices.push_back(thirdIdx);

        indices.push_back(firstIdx);
        indices.push_back(thirdIdx);
        indices.push_back(fourthIdx);
    }

    m_gridPtr = std::make_unique<VertexBuffer<Vertex2>>(vertices);
    m_idxGridPtr = std::make_unique<IndexBuffer<std::uint32_t>>(indices);

    std::vector<Vertex2> v2{};
    std::vector<std::uint32_t> u32{};
    for (auto& buffer : m_islandVertexBuffers)
        buffer = std::make_unique<VertexBuffer<Vertex2>>(v2);

    for (auto& buffer : m_islandIndexBuffers)
        buffer = std::make_unique<IndexBuffer<std::uint32_t>>(u32);

    m_bottleVertexBuffer = std::make_unique<VertexBuffer<Vertex2>>(v2);
    m_bottleIndexBuffer = std::make_unique<IndexBuffer<std::uint32_t>>(u32);
}

void Map::addIsland(Position position, const std::vector<std::string>& pattern) {
    Rectangle rectangle{ .xy = position,
                         .wh = { m_textureSize.width * static_cast<float>(pattern.at(0).size()),
                                 m_textureSize.height * static_cast<float>(pattern.size()) } };
    m_islands.emplace_back(m_textureSize, rectangle, pattern);

    for (const auto& pos : m_islands.back().getPositions()) {
        auto found{ std::find(m_waterPositions.begin(), m_waterPositions.end(), pos.second) };
        if (found != m_waterPositions.end()) m_waterPositions.erase(found);

        auto leftXpos{ pos.second.x + 400 - m_textureSize.width / 2.0f };
        auto rightXpos{ pos.second.x + 400 + m_textureSize.width / 2.0f };
        auto topYpos{ pos.second.y + 300 + m_textureSize.height / 2.0f };
        auto bottomYpos{ pos.second.y + 300 - m_textureSize.height / 2.0f };

        auto normalizedXLeftPos{ (leftXpos / (800.f * 0.5f)) - 1.0f };
        auto normalizedXRightPos{ (rightXpos / (800.f * 0.5f)) - 1.0f };
        auto normalizedYTopPos{ (topYpos / (600.f * 0.5f)) - 1.0f };
        auto normalizedYBottomPos{ (bottomYpos / (600.f * 0.5f)) - 1.0f };

        Vertex2 v1{};
        v1.x = normalizedXLeftPos;
        v1.y = normalizedYTopPos;
        v1.texX = 0.0;
        v1.texY = 0.0;

        Vertex2 v2{};
        v2.x = normalizedXRightPos;
        v2.y = normalizedYTopPos;
        v2.texX = 1.0;
        v2.texY = 0.0;

        Vertex2 v3{};
        v3.x = normalizedXRightPos;
        v3.y = normalizedYBottomPos;
        v3.texX = 1.0;
        v3.texY = 1.0;

        Vertex2 v4{};
        v4.x = normalizedXLeftPos;
        v4.y = normalizedYBottomPos;
        v4.texX = 0.0;
        v4.texY = 1.0;

        std::size_t idx{};
        switch (pos.first) {
        case 'S':
            idx = 0;
            break;
        case 'B':
            idx = 1;
            break;
        case 'G':
            idx = 2;
            break;
        case 'R':
            idx = 3;
            break;
        case 'P':
            idx = 4;
            break;
        }
        auto& vBuffer{ m_islandVertexBuffers.at(idx) };
        auto& iBuffer{ m_islandIndexBuffers.at(idx) };
        auto size{ static_cast<std::uint32_t>(vBuffer->size()) };

        std::vector<Vertex2> vert{ v1, v2, v3, v4 };
        vBuffer->addData(vert);

        std::vector<std::uint32_t> indices{ size + 0, size + 1, size + 2,
                                            size + 0, size + 2, size + 3 };
        iBuffer->addData(indices);
    }
}

const std::vector<Position>& Map::getWaterPositions() const noexcept { return m_waterPositions; }

void Map::resizeUpdate() {
    for (auto& island : m_islands)
        island.resizeUpdate();

    m_bottle.resizeUpdate();
    m_treasure.resizeUpdate();

    m_waterSprite.updateWindowSize();
    m_waterSprite.checkAspect({ 800, 600 });
    m_airSprite.updateWindowSize();
    m_airSprite.checkAspect({ 800, 600 });
}

void Map::render(const View& view) {
    for (std::size_t i{}; i < 5; ++i) {
        char isl{};
        switch (i) {
        case 0:
            isl = 'S';
            break;
        case 1:
            isl = 'B';
            break;
        case 2:
            isl = 'G';
            break;
        case 3:
            isl = 'R';
            break;
        case 4:
            isl = 'P';
            break;
        }

        auto& sprite{ Island::getIslandTiles()->at(Island::getChatToIsland()->at(isl)) };
        sprite.setPosition({ 0, 0 });

        getEngineInstance()->render(*m_islandVertexBuffers.at(i),
                                    *m_islandIndexBuffers.at(i),
                                    sprite.getTexture(),
                                    sprite.getResultMatrix(),
                                    view);
    }

    m_bottle.setPosition({ 0, 0 });
    getEngineInstance()->render(*m_bottleVertexBuffer,
                                *m_bottleIndexBuffer,
                                m_bottle.getSprite().getTexture(),
                                m_bottle.getSprite().getResultMatrix(),
                                view);

    m_waterSprite.setPosition({ 0, 0 });
    getEngineInstance()->render(*m_gridPtr,
                                *m_idxGridPtr,
                                m_waterSprite.getTexture(),
                                m_waterSprite.getResultMatrix(),
                                view);
}

Sprite& Map::getWaterSprite() noexcept { return m_waterSprite; }
Island& Map::getIsland(std::size_t id) noexcept { return m_islands.at(id); }

void Map::interact(Ship& ship) {
    if (!ship.isInteract())
        for (auto& island : m_islands) {
            island.interact(ship);
            if (ship.isInteract()) {
                m_interactIsland = &island;
                break;
            }
        }

    if (!ship.getPlayer().hasBottle()) {
        for (std::size_t i{}; i < m_bottlePositions.size(); ++i) {
            m_bottle.setPosition(m_bottlePositions[i]);
            if (intersect(m_bottle.getSprite(), ship.getSprite())) {
                generateTreasure();
                ship.getPlayer().setBottle(true);
                m_isTreasureUnearthed = false;
                m_bottlePositions.erase(m_bottlePositions.begin() + i);
                generateBottles();
                break;
            }
        }
    }

    Rectangle rect{ .xy{ -400.0f, -300.f }, .wh{ m_mapSize.width, m_mapSize.height } };
    if (auto intersectRect{ intersect(rect, ship.getSprite().getRectangle()) }) {
        if (!rect.contains(ship.getPosition())) {
            ship.forceStop();
            ship.setInteract(false);
        }
    }
    m_shipRectangle = ship.getSprite().getRectangle();
}

void Map::interact(Player& player) {
    m_interactIsland->interact(player);

    if (player.isDigging()) {
        auto is{ intersect(m_treasure.getTreasureSprite(), player.getSprite()) };
        if (is && player.hasBottle()) {
            if (!m_isTreasureUnearthed)
                m_isTreasureUnearthed = true;
            else {
                player.setBottle(false);
                m_isTreasureUnearthed = false;
                generateBottles();
                player.addMoney(1);
            }
        }
        player.stopDig();
    }

    player.setNearShip(m_shipRectangle.contains(player.getPosition()));
}

void Map::generateBottles() {
    while (m_countOfBottles < s_maxCountOfBottles) {
        auto randomPos{ generateRandomNumber(0, static_cast<int>(m_waterPositions.size()) - 1) };
        m_bottlePositions.emplace_back(m_waterPositions.at(randomPos));
        ++m_countOfBottles;
    }

    updateBottlePositions();
}

void Map::generateTreasure() {
    auto rndIsland{ generateRandomNumber(0, static_cast<int>(m_islands.size()) - 1) };
    auto rndPos{ generateRandomNumber(
        0, static_cast<int>(m_islands[rndIsland].getPositions().size()) - 1) };
    auto pos{ m_islands[rndIsland].getPositions().at(rndPos).second };
    m_treasure.setPosition(pos);
}

Treasure& Map::getTreasure() noexcept { return m_treasure; }

bool Map::isTreasureUnearthed() const noexcept { return m_isTreasureUnearthed; }

void Map::updateBottlePositions() {
    std::vector<Vertex2> vertices{};
    std::vector<std::uint32_t> indices{};
    for (auto pos : m_bottlePositions) {
        auto leftXpos{ (pos.x + 400) - m_textureSize.width / 2.0f };
        auto rightXpos{ (pos.x + 400) + m_textureSize.width / 2.0f };
        auto topYpos{ (pos.y + 300) + m_textureSize.height / 2.0f };
        auto bottomYpos{ (pos.y + 300) - m_textureSize.height / 2.0f };

        auto normalizedXLeftPos{ (leftXpos / (800.f * 0.5f)) - 1.0f };
        auto normalizedXRightPos{ (rightXpos / (800.f * 0.5f)) - 1.0f };
        auto normalizedYTopPos{ (topYpos / (600.f * 0.5f)) - 1.0f };
        auto normalizedYBottomPos{ (bottomYpos / (600.f * 0.5f)) - 1.0f };

        Vertex2 v1{};
        v1.x = normalizedXLeftPos;
        v1.y = normalizedYTopPos;
        v1.texX = 0.0;
        v1.texY = 0.0;

        vertices.push_back(v1);
        auto firstIdx{ vertices.size() - 1 };

        Vertex2 v2{};
        v2.x = normalizedXRightPos;
        v2.y = normalizedYTopPos;
        v2.texX = 1.0;
        v2.texY = 0.0;
        vertices.push_back(v2);
        auto secondIdx{ vertices.size() - 1 };

        Vertex2 v3{};
        v3.x = normalizedXRightPos;
        v3.y = normalizedYBottomPos;
        v3.texX = 1.0;
        v3.texY = 1.0;
        vertices.push_back(v3);
        auto thirdIdx{ vertices.size() - 1 };

        Vertex2 v4{};
        v4.x = normalizedXLeftPos;
        v4.y = normalizedYBottomPos;
        v4.texX = 0.0;
        v4.texY = 1.0;
        vertices.push_back(v4);
        auto fourthIdx{ vertices.size() - 1 };

        indices.push_back(firstIdx);
        indices.push_back(secondIdx);
        indices.push_back(thirdIdx);

        indices.push_back(firstIdx);
        indices.push_back(thirdIdx);
        indices.push_back(fourthIdx);
    }
    m_bottleVertexBuffer->updateData(std::move(vertices));
    m_bottleIndexBuffer->updateData(std::move(indices));
}
