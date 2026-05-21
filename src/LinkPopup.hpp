#pragma once
#include <Geode/Geode.hpp>
#include "Tracker.hpp"

using namespace geode::prelude;

class LinkPopup : public FLAlertLayer {
protected:
    GJGameLevel* m_lvl;
    CCArray* m_lvls;

    int m_page = 0;
    int m_limit = 6;

    CCMenu* m_listMenu;
    CCMenu* m_navMenu;
    CCLabelBMFont* m_pageLbl;
    CCMenuItemSpriteExtra* m_prev;
    CCMenuItemSpriteExtra* m_next;

    bool init(GJGameLevel* lvl) {
        if (!FLAlertLayer::init(150)) return false;
        m_lvl = lvl;
        m_lvls = LocalLevelManager::sharedState()->m_localLevels;

        auto win = CCDirector::get()->getWinSize();

        auto container = CCNode::create();
        container->setPosition(win.width / 2, win.height / 2);
        this->addChild(container);

        auto bg = CCScale9Sprite::create("GJ_square01.png");
        bg->setContentSize({ 340.f, 260.f });
        bg->setPosition(0.f, 0.f);
        container->addChild(bg);

        auto title = CCLabelBMFont::create("LINK LEVEL", "goldFont.fnt");
        title->setPosition(0.f, 110.f);
        title->setScale(0.7f);
        container->addChild(title);

        m_listMenu = CCMenu::create();
        m_listMenu->setPosition(0.f, 10.f);
        m_listMenu->setTouchPriority(-501);
        container->addChild(m_listMenu);

        m_pageLbl = CCLabelBMFont::create("Page 1/1", "chatFont.fnt");
        m_pageLbl->setPosition(0.f, -85.f);
        m_pageLbl->setScale(0.5f);
        container->addChild(m_pageLbl);

        auto prevSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        m_prev = CCMenuItemSpriteExtra::create(prevSpr, this, menu_selector(LinkPopup::onPrev));
        m_prev->setPosition(-190.f, 10.f);

        auto nextSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        nextSpr->setFlipX(true);
        m_next = CCMenuItemSpriteExtra::create(nextSpr, this, menu_selector(LinkPopup::onNext));
        m_next->setPosition(190.f, 10.f);

        m_navMenu = CCMenu::create();
        m_navMenu->setPosition(0, 0);
        m_navMenu->setTouchPriority(-501);
        m_navMenu->addChild(m_prev);
        m_navMenu->addChild(m_next);
        container->addChild(m_navMenu);

        if (!m_buttonMenu) {
            m_buttonMenu = CCMenu::create();
            container->addChild(m_buttonMenu);
        }
        else {
            m_buttonMenu->removeFromParent();
            container->addChild(m_buttonMenu);
            m_buttonMenu->removeAllChildrenWithCleanup(true);
        }

        auto spr = ButtonSprite::create("CANCEL", "goldFont.fnt", "GJ_button_06.png", 0.7f);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(LinkPopup::onClose));
        m_buttonMenu->addChild(btn);
        m_buttonMenu->setPosition(0.f, -110.f);

        loadPage();

        container->setScale(0.1f);
        container->runAction(CCEaseElasticOut::create(CCScaleTo::create(0.4f, 1.0f), 0.5f));
        return true;
    }

    void loadPage() {
        m_listMenu->removeAllChildrenWithCleanup(true);

        if (!m_lvls || m_lvls->count() == 0) {
            m_pageLbl->setString("No levels found");
            m_prev->setVisible(false);
            m_next->setVisible(false);
            return;
        }

        int tot = m_lvls->count();
        int pages = (tot + m_limit - 1) / m_limit;

        m_pageLbl->setString(fmt::format("Page {}/{}", m_page + 1, pages).c_str());
        m_prev->setVisible(m_page > 0);
        m_next->setVisible(m_page < pages - 1);

        int start = m_page * m_limit;
        int end = std::min(start + m_limit, tot);

        for (int i = start; i < end; i++) {
            auto level = static_cast<GJGameLevel*>(m_lvls->objectAtIndex(i));
            std::string name = level->m_levelName;
            if (name.empty()) name = "Unnamed";

            auto row = CCNode::create();
            row->setContentSize({ 280.f, 24.f });

            auto rowBg = CCScale9Sprite::create("square02_small.png");
            rowBg->setContentSize({ 280.f, 24.f });
            rowBg->setPosition(140.f, 12.f);
            rowBg->setOpacity(150);
            row->addChild(rowBg);

            auto lbl = CCLabelBMFont::create(name.c_str(), "chatFont.fnt");
            lbl->setAnchorPoint({ 0.f, 0.5f });
            lbl->setPosition(10.f, 12.f);
            lbl->setScale(0.55f);
            row->addChild(lbl);

            auto btn = CCMenuItemSpriteExtra::create(row, this, menu_selector(LinkPopup::onSelectLevel));
            btn->setUserData(level);
            m_listMenu->addChild(btn);
        }

        m_listMenu->alignItemsVerticallyWithPadding(3.f);
    }

    void onPrev(CCObject*) { if (m_page > 0) { m_page--; loadPage(); } }
    void onNext(CCObject*) { m_page++; loadPage(); }

    void onSelectLevel(CCObject* sender) {
        auto btn = static_cast<CCNode*>(sender);
        auto level = static_cast<GJGameLevel*>(btn->getUserData());

        std::string name = level->m_levelName;
        int id = m_lvl->m_levelID.value();

        RT::links[id] = name;
        RT::saveLnk();

        FLAlertLayer::create("Level Linked", fmt::format("Data is now merged with:\n{}", name).c_str(), "OK")->show();
        this->onClose(nullptr);
    }

    void registerWithTouchDispatcher() override {
        FLAlertLayer::registerWithTouchDispatcher();
        auto dispatcher = CCDirector::get()->getTouchDispatcher();
        if (m_listMenu) dispatcher->addTargetedDelegate(m_listMenu, -501, true);
        if (m_navMenu) dispatcher->addTargetedDelegate(m_navMenu, -501, true);
    }

public:
    static LinkPopup* create(GJGameLevel* lvl) {
        auto ret = new LinkPopup();
        if (ret && ret->init(lvl)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    void onClose(CCObject*) { this->removeFromParentAndCleanup(true); }
};