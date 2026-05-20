#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/EditLevelLayer.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>

using namespace geode::prelude;

// =======================================================
// STRUCTURES AND DATA
// =======================================================
struct Run {
    float start;
    float end;
};

// g_levelLinks : Online_ID -> Editor_Name
std::map<int, std::string> g_levelLinks;

// g_levelRuns : ID_or_Name -> List_of_Runs
std::map<std::string, std::vector<Run>> g_levelRuns;

// Get the correct save key (ID if online, Name if editor)
std::string getLevelKey(GJGameLevel* level) {
    if (level->m_levelID.value() > 0) return std::to_string(level->m_levelID.value());
    return std::string(level->m_levelName);
}

// Mathematical function for ABSOLUTE PERCENTAGE (Fixes Start Pos everywhere!)
float getAbsolutePercent(PlayLayer* layer) {
    if (!layer || layer->m_levelLength == 0.f) return 0.0f;
    float percent = (layer->m_player1->getPositionX() / layer->m_levelLength) * 100.0f;
    return std::clamp(percent, 0.0f, 100.0f);
}

void saveLinksToDisk() {
    std::ofstream file("links.txt");
    for (auto const& [id, name] : g_levelLinks) {
        file << id << "|" << name << "\n";
    }
}

void loadLinksFromDisk() {
    std::ifstream file("links.txt");
    std::string line;
    while (std::getline(file, line)) {
        size_t sep = line.find('|');
        if (sep != std::string::npos) {
            int id = std::stoi(line.substr(0, sep));
            std::string name = line.substr(sep + 1);
            g_levelLinks[id] = name;
        }
    }
}

void saveRunsToDisk() {
    std::ofstream file("runs_data.txt");
    if (file.is_open()) {
        for (auto const& [key, runs] : g_levelRuns) {
            file << key << "|";
            for (const auto& r : runs) {
                file << r.start << "," << r.end << " ";
            }
            file << "\n";
        }
        file.close();
    }
}

void loadRunsFromDisk() {
    std::ifstream file("runs_data.txt");
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        size_t sep = line.find('|');
        if (sep != std::string::npos) {
            std::string key = line.substr(0, sep);
            std::string runsPart = line.substr(sep + 1);
            std::stringstream ss(runsPart);
            std::string runStr;
            while (ss >> runStr) {
                size_t commaPos = runStr.find(',');
                if (commaPos != std::string::npos) {
                    float s = std::stof(runStr.substr(0, commaPos));
                    float e = std::stof(runStr.substr(commaPos + 1));
                    g_levelRuns[key].push_back({ s, e });
                }
            }
        }
        else {
            std::stringstream ss(line);
            std::string key;
            if (!(ss >> key)) continue;

            std::string runStr;
            while (ss >> runStr) {
                size_t commaPos = runStr.find(',');
                if (commaPos != std::string::npos) {
                    float s = std::stof(runStr.substr(0, commaPos));
                    float e = std::stof(runStr.substr(commaPos + 1));
                    g_levelRuns[key].push_back({ s, e });
                }
            }
        }
    }
    file.close();
}
// =======================================================
// 1. LOCAL LEVEL CHOICE WINDOW (LINK)
// =======================================================
class LinkPopup : public CCLayerColor {
protected:
    GJGameLevel* m_onlineLevel;
    CCArray* m_localLevels;

    int m_currentPage = 0;
    int m_itemsPerPage = 6; // <-- 6 Levels per page to fit perfectly

    CCMenu* m_itemsMenu;
    CCLabelBMFont* m_pageLabel;
    CCMenuItemSpriteExtra* m_prevBtn;
    CCMenuItemSpriteExtra* m_nextBtn;

    bool init(GJGameLevel* onlineLevel) {
        if (!CCLayerColor::initWithColor({ 0, 0, 0, 160 })) return false;
        m_onlineLevel = onlineLevel;
        m_localLevels = LocalLevelManager::sharedState()->m_localLevels;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        this->setTouchEnabled(true);
        this->setKeypadEnabled(true);

        auto mainLayer = CCLayer::create();
        this->addChild(mainLayer);

        // Background (Standard GD Size: 260 height)
        auto bg = CCScale9Sprite::create("GJ_square01.png");
        bg->setContentSize({ 340.f, 260.f });
        bg->setPosition(winSize.width / 2, winSize.height / 2);
        mainLayer->addChild(bg);

        // Title
        auto title = CCLabelBMFont::create("LINK LEVEL", "goldFont.fnt");
        title->setPosition(winSize.width / 2, winSize.height / 2 + 110.f);
        title->setScale(0.7f);
        mainLayer->addChild(title);

        // Central Menu
        m_itemsMenu = CCMenu::create();
        m_itemsMenu->setPosition(winSize.width / 2, winSize.height / 2 + 10.f);
        mainLayer->addChild(m_itemsMenu);

        // Pagination Label
        m_pageLabel = CCLabelBMFont::create("Page 1/1", "chatFont.fnt");
        m_pageLabel->setPosition(winSize.width / 2, winSize.height / 2 - 85.f);
        m_pageLabel->setScale(0.5f);
        mainLayer->addChild(m_pageLabel);

        // Navigation Arrows
        auto prevSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        m_prevBtn = CCMenuItemSpriteExtra::create(prevSprite, this, menu_selector(LinkPopup::onPrev));
        m_prevBtn->setPosition(winSize.width / 2 - 190.f, winSize.height / 2 + 10.f);

        auto nextSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        nextSprite->setFlipX(true);
        m_nextBtn = CCMenuItemSpriteExtra::create(nextSprite, this, menu_selector(LinkPopup::onNext));
        m_nextBtn->setPosition(winSize.width / 2 + 190.f, winSize.height / 2 + 10.f);

        auto navMenu = CCMenu::create();
        navMenu->setPosition(0, 0);
        navMenu->addChild(m_prevBtn);
        navMenu->addChild(m_nextBtn);
        mainLayer->addChild(navMenu);

        // Close Button
        auto closeSprite = ButtonSprite::create("CANCEL", "goldFont.fnt", "GJ_button_06.png", 0.7f);
        auto closeBtn = CCMenuItemSpriteExtra::create(closeSprite, this, menu_selector(LinkPopup::onClose));
        auto closeMenu = CCMenu::create();
        closeMenu->addChild(closeBtn);
        closeMenu->setPosition(winSize.width / 2, winSize.height / 2 - 110.f);
        mainLayer->addChild(closeMenu);

        loadPage();

        mainLayer->setScale(0.1f);
        mainLayer->runAction(CCEaseElasticOut::create(CCScaleTo::create(0.4f, 1.0f), 0.5f));
        return true;
    }

    void loadPage() {
        m_itemsMenu->removeAllChildrenWithCleanup(true);

        if (!m_localLevels || m_localLevels->count() == 0) {
            m_pageLabel->setString("No levels found");
            m_prevBtn->setVisible(false);
            m_nextBtn->setVisible(false);
            return;
        }

        int totalLevels = m_localLevels->count();
        int totalPages = (totalLevels + m_itemsPerPage - 1) / m_itemsPerPage;

        m_pageLabel->setString(fmt::format("Page {}/{}", m_currentPage + 1, totalPages).c_str());
        m_prevBtn->setVisible(m_currentPage > 0);
        m_nextBtn->setVisible(m_currentPage < totalPages - 1);

        int startIdx = m_currentPage * m_itemsPerPage;
        int endIdx = std::min(startIdx + m_itemsPerPage, totalLevels);

        for (int i = startIdx; i < endIdx; i++) {
            auto level = static_cast<GJGameLevel*>(m_localLevels->objectAtIndex(i));

            std::string lvlName = level->m_levelName;
            if (lvlName.empty()) lvlName = "Unnamed";

            // Level Row
            auto row = CCNode::create();
            row->setContentSize({ 280.f, 24.f });

            auto rowBg = CCScale9Sprite::create("square02_small.png");
            rowBg->setContentSize({ 280.f, 24.f });
            rowBg->setPosition(140.f, 12.f);
            rowBg->setOpacity(150);
            row->addChild(rowBg);

            auto label = CCLabelBMFont::create(lvlName.c_str(), "chatFont.fnt");
            label->setAnchorPoint({ 0.f, 0.5f });
            label->setPosition(10.f, 12.f);
            label->setScale(0.55f);
            row->addChild(label);

            auto btn = CCMenuItemSpriteExtra::create(row, this, menu_selector(LinkPopup::onSelectLevel));
            btn->setUserData(level);
            m_itemsMenu->addChild(btn);
        }

        m_itemsMenu->alignItemsVerticallyWithPadding(3.f);
    }

    void onPrev(CCObject*) { if (m_currentPage > 0) { m_currentPage--; loadPage(); } }
    void onNext(CCObject*) { m_currentPage++; loadPage(); }

    void onSelectLevel(CCObject* sender) {
        auto btn = static_cast<CCNode*>(sender);
        auto level = static_cast<GJGameLevel*>(btn->getUserData());

        std::string localName = level->m_levelName;
        int onlineID = m_onlineLevel->m_levelID.value();

        g_levelLinks[onlineID] = localName;
        saveLinksToDisk();

        FLAlertLayer::create("Level Linked", fmt::format("Data is now merged with:\n{}", localName).c_str(), "OK")->show();
        this->removeFromParentAndCleanup(true);
    }

public:
    static LinkPopup* create(GJGameLevel* onlineLevel) {
        auto ret = new LinkPopup();
        if (ret && ret->init(onlineLevel)) { ret->autorelease(); return ret; }
        CC_SAFE_DELETE(ret); return nullptr;
    }

    void onClose(CCObject*) { this->removeFromParentAndCleanup(true); }
    void keyBackClicked() override { onClose(nullptr); }
    void registerWithTouchDispatcher() override {
        CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -501, true);
    }
    bool ccTouchBegan(CCTouch*, CCEvent*) override { return true; }
};

// =======================================================
// 2. STATISTICS WINDOW (RUNS)
// =======================================================
class RunsPopup : public CCLayerColor {
protected:
    GJGameLevel* m_level;

    bool init(std::vector<Run> runs, bool showTotal, GJGameLevel* level) {
        if (!CCLayerColor::initWithColor({ 0, 0, 0, 160 })) return false;
        m_level = level;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        this->setTouchEnabled(true);
        this->setKeypadEnabled(true);

        auto mainLayer = CCLayer::create();
        this->addChild(mainLayer);

        auto bg = CCScale9Sprite::create("GJ_square01.png");
        bg->setContentSize({ 340.f, 250.f });
        bg->setPosition(winSize.width / 2, winSize.height / 2);
        mainLayer->addChild(bg);

        auto title = CCLabelBMFont::create("STATISTICS", "goldFont.fnt");
        title->setPosition(winSize.width / 2, winSize.height / 2 + 100.f);
        title->setScale(0.7f);
        mainLayer->addChild(title);

        if (showTotal) {
            auto totalLabel = CCLabelBMFont::create(fmt::format("OPTIMAL: {} RUNS", runs.size()).c_str(), "chatFont.fnt");
            totalLabel->setScale(0.6f);
            totalLabel->setPosition(winSize.width / 2, winSize.height / 2 + 70.f);
            mainLayer->addChild(totalLabel);
        }

        auto listSize = CCSize(280.f, 140.f);
        auto center = CCPoint(winSize.width / 2, winSize.height / 2);
        float yOffset = showTotal ? 0.f : 20.f;

        auto scrollBg = CCScale9Sprite::create("square02_small.png");
        scrollBg->setContentSize(listSize);
        scrollBg->setOpacity(100);
        scrollBg->setPosition(center.x, center.y - 10.f + yOffset);
        mainLayer->addChild(scrollBg);

        auto scroll = ScrollLayer::create(listSize);
        scroll->setPosition(center.x - listSize.width / 2, center.y - listSize.height / 2 - 10.f + yOffset);
        mainLayer->addChild(scroll);

        float totalHeight = runs.size() * 25.f;
        if (totalHeight < listSize.height) totalHeight = listSize.height;
        scroll->m_contentLayer->setContentSize({ listSize.width, totalHeight });

        float currentY = totalHeight - 20.f;
        int index = 1;
        for (const auto& r : runs) {
            std::string text = fmt::format("{}.  {}% - {}%", index, (int)r.start, (int)r.end);
            auto label = CCLabelBMFont::create(text.c_str(), "chatFont.fnt");
            label->setScale(0.6f);
            label->setAnchorPoint({ 0.f, 0.5f });
            label->setPosition(10.f, currentY);
            scroll->m_contentLayer->addChild(label);
            currentY -= 25.f;
            index++;
        }

        auto menu = CCMenu::create();

        auto closeBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("OK", "goldFont.fnt", "GJ_button_02.png", 0.6f), this, menu_selector(RunsPopup::onClose));
        menu->addChild(closeBtn);

        // Link or Unlink button (Only for Online levels)
        if (m_level->m_levelID.value() > 0) {
            bool isLinked = g_levelLinks.count(m_level->m_levelID.value()) > 0;
            auto linkSprite = ButtonSprite::create(isLinked ? "Unlink" : "Link", "goldFont.fnt", isLinked ? "GJ_button_06.png" : "GJ_button_01.png", 0.6f);
            auto linkBtn = CCMenuItemSpriteExtra::create(linkSprite, this, menu_selector(RunsPopup::onLink));
            menu->addChild(linkBtn);

            menu->alignItemsHorizontallyWithPadding(15.f);
        }

        menu->setPosition(winSize.width / 2, winSize.height / 2 - 95.f);
        mainLayer->addChild(menu);

        mainLayer->setScale(0.1f);
        mainLayer->runAction(CCEaseElasticOut::create(CCScaleTo::create(0.4f, 1.0f), 0.5f));
        return true;
    }

    void onLink(CCObject*) {
        int onlineID = m_level->m_levelID.value();

        if (g_levelLinks.count(onlineID)) {
            g_levelLinks.erase(onlineID);
            saveLinksToDisk();
            FLAlertLayer::create("Unlinked", "Editor statistics are no longer merged.", "OK")->show();
            this->onClose(nullptr);
        }
        else {
            auto popup = LinkPopup::create(m_level);
            CCDirector::sharedDirector()->getRunningScene()->addChild(popup, 105);
            this->onClose(nullptr);
        }
    }

public:
    static RunsPopup* create(std::vector<Run> runs, bool showTotal, GJGameLevel* level) {
        auto ret = new RunsPopup();
        if (ret && ret->init(runs, showTotal, level)) { ret->autorelease(); return ret; }
        CC_SAFE_DELETE(ret); return nullptr;
    }

    void onClose(CCObject*) { this->removeFromParentAndCleanup(true); }
    void keyBackClicked() override { onClose(nullptr); }
    void registerWithTouchDispatcher() override {
        CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -500, true);
    }
    bool ccTouchBegan(CCTouch*, CCEvent*) override { return true; }
};

// =======================================================
// 3. GLOBAL LOGIC TO GENERATE STATS
// =======================================================
void showRunsMenu(GJGameLevel* level) {
    std::string currentKey = getLevelKey(level);
    std::vector<Run> allRuns;

    if (g_levelRuns.count(currentKey)) {
        allRuns = g_levelRuns[currentKey];
    }

    // Merge if link exists
    if (level->m_levelID.value() > 0) {
        int onlineID = level->m_levelID.value();
        if (g_levelLinks.count(onlineID)) {
            std::string editorName = g_levelLinks[onlineID];
            if (g_levelRuns.count(editorName)) {
                auto& editorRuns = g_levelRuns[editorName];
                allRuns.insert(allRuns.end(), editorRuns.begin(), editorRuns.end());
            }
        }
    }

    if (allRuns.empty()) {
        FLAlertLayer::create("Statistics", "No data recorded.", "OK")->show();
        return;
    }

    bool hasCompletedLevel = false;
    for (const auto& r : allRuns) {
        if (r.end >= 100.0f) {
            hasCompletedLevel = true;
            break;
        }
    }

    std::sort(allRuns.begin(), allRuns.end(), [](const Run& a, const Run& b) {
        if (a.start == b.start) return a.end > b.end;
        return a.start < b.start;
        });

    std::vector<Run> bestRuns;
    float currentTarget = 0.0f;
    size_t i = 0;

    while (i < allRuns.size()) {
        if (allRuns[i].start > currentTarget) currentTarget = allRuns[i].start;
        float maxEnd = currentTarget;
        Run bestRun = { -1.0f, -1.0f };
        while (i < allRuns.size() && allRuns[i].start <= currentTarget) {
            if (allRuns[i].end > maxEnd) {
                maxEnd = allRuns[i].end;
                bestRun = allRuns[i];
            }
            i++;
        }
        if (bestRun.end > -1.0f) {
            bestRuns.push_back(bestRun);
            currentTarget = maxEnd;
        }
        if (currentTarget >= 100.0f) break;
    }

    auto popup = RunsPopup::create(bestRuns, hasCompletedLevel, level);
    CCDirector::sharedDirector()->getRunningScene()->addChild(popup, 100);
}

// =======================================================
// 4. THE TRACKER (GAME)
// =======================================================
class $modify(MyPlayLayer, PlayLayer) {
    struct Fields { float m_startPercent = 0.0f; };

    void resetLevel() {
        PlayLayer::resetLevel();
        if (this->m_level->isPlatformer()) return;

        // Exact Start Pos tracking
        m_fields->m_startPercent = getAbsolutePercent(this);
    }

    void destroyPlayer(PlayerObject * player, GameObject * object) {
        PlayLayer::destroyPlayer(player, object);
        if (this->m_level->isPlatformer()) return;

        // Exact End tracking
        float end = getAbsolutePercent(this);
        float start = m_fields->m_startPercent;

        if (end - start > 1.0f) {
            std::string key = getLevelKey(this->m_level);
            g_levelRuns[key].push_back({ start, end });
            saveRunsToDisk();
        }
    }

    void levelComplete() {
        PlayLayer::levelComplete();
        if (this->m_level->isPlatformer()) return;

        std::string key = getLevelKey(this->m_level);
        g_levelRuns[key].push_back({ m_fields->m_startPercent, 100.0f });
        saveRunsToDisk();
    }
};

// =======================================================
// 5. BUTTON IN ONLINE MENU
// =======================================================
class $modify(MyLevelInfoLayer, LevelInfoLayer) {
    bool init(GJGameLevel * level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;

        auto menu = this->getChildByID("left-side-menu");
        if (menu) {
            auto spriteRuns = ButtonSprite::create("Runs", "goldFont.fnt", "GJ_button_02.png", 0.7f);
            spriteRuns->setScale(0.85f);
            auto btnRuns = CCMenuItemSpriteExtra::create(spriteRuns, this, menu_selector(MyLevelInfoLayer::onViewRuns));
            btnRuns->setID("tracker-runs-button"_spr);
            menu->addChild(btnRuns);
            menu->updateLayout();
        }
        return true;
    }

    void onViewRuns(CCObject*) {
        showRunsMenu(this->m_level);
    }
};

// =======================================================
// 6. BUTTON IN LOCAL MENU (EDITOR)
// =======================================================
class $modify(MyEditLevelLayer, EditLevelLayer) {
    struct Fields { GJGameLevel* m_level; };

    bool init(GJGameLevel * level) {
        m_fields->m_level = level;
        if (!EditLevelLayer::init(level)) return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto btnMenu = CCMenu::create();

        auto spriteRuns = ButtonSprite::create("Runs", "goldFont.fnt", "GJ_button_02.png", 0.7f);
        spriteRuns->setScale(0.85f);
        auto btnRuns = CCMenuItemSpriteExtra::create(spriteRuns, this, menu_selector(MyEditLevelLayer::onViewRuns));

        btnMenu->addChild(btnRuns);
        btnMenu->setPosition(30.f, winSize.height - 80.f);
        this->addChild(btnMenu);

        return true;
    }

    void onViewRuns(CCObject*) {
        showRunsMenu(m_fields->m_level);
    }
};

// =======================================================
// INITIALIZATION
// =======================================================
$execute{
    loadRunsFromDisk();
    loadLinksFromDisk();
}