#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/EditLevelLayer.hpp>

#include "Tracker.hpp"
#include "LinkPopup.hpp"
#include "RunsPopup.hpp"

using namespace geode::prelude;

class $modify(MyPL, PlayLayer) {
    struct Fields { float m_startPercent = 0.0f; };

    void resetLevel() {
        PlayLayer::resetLevel();
        if (this->m_level->isPlatformer()) return;

        m_fields->m_startPercent = RT::getAbsPct(this);
    }

    void destroyPlayer(PlayerObject * player, GameObject * object) {
        PlayLayer::destroyPlayer(player, object);
        if (this->m_level->isPlatformer()) return;

        float end = RT::getAbsPct(this);
        float start = m_fields->m_startPercent;

        if (end - start > 1.0f) {
            std::string key = RT::getLvlKey(this->m_level);
            RT::runs[key].push_back({ start, end });
            RT::saveRns();
        }
    }

    void levelComplete() {
        PlayLayer::levelComplete();
        if (this->m_level->isPlatformer()) return;

        std::string key = RT::getLvlKey(this->m_level);
        RT::runs[key].push_back({ m_fields->m_startPercent, 100.0f });
        RT::saveRns();
    }
};

class $modify(MyLIL, LevelInfoLayer) {
    bool init(GJGameLevel * level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;

        auto menu = this->getChildByID("left-side-menu");
        if (menu) {
            auto spriteRuns = ButtonSprite::create("Runs", "goldFont.fnt", "GJ_button_02.png", 0.7f);
            spriteRuns->setScale(0.85f);
            auto btnRuns = CCMenuItemSpriteExtra::create(spriteRuns, this, menu_selector(MyLIL::onViewRuns));
            btnRuns->setID("tracker-runs-button"_spr);
            menu->addChild(btnRuns);
            menu->updateLayout();
        }
        return true;
    }

    void onViewRuns(CCObject*) {
        checkUpdates(this->m_level);
    }
};

class $modify(MyELL, EditLevelLayer) {
    struct Fields { GJGameLevel* m_level; };

    bool init(GJGameLevel * level) {
        m_fields->m_level = level;
        if (!EditLevelLayer::init(level)) return false;

        auto winSize = CCDirector::get()->getWinSize();
        auto btnMenu = CCMenu::create();

        auto spriteRuns = ButtonSprite::create("Runs", "goldFont.fnt", "GJ_button_02.png", 0.7f);
        spriteRuns->setScale(0.85f);
        auto btnRuns = CCMenuItemSpriteExtra::create(spriteRuns, this, menu_selector(MyELL::onViewRuns));

        btnMenu->addChild(btnRuns);
        btnMenu->setPosition(30.f, winSize.height - 80.f);
        this->addChild(btnMenu);

        return true;
    }

    void onViewRuns(CCObject*) {
        checkUpdates(m_fields->m_level);
    }
};

$execute{
    RT::loadRns();
    RT::loadLnk();
}