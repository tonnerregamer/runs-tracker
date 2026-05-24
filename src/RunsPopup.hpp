#pragma once
#include <Geode/Geode.hpp>
#include "Tracker.hpp"
#include "LinkPopup.hpp"
#include <thread>

using namespace geode::prelude;

class RunsPopup : public FLAlertLayer {
protected:
    GJGameLevel* m_lvl;

    bool init(std::vector<Run> rns, bool showTotal, GJGameLevel* lvl) {
        if (!FLAlertLayer::init(150)) return false;
        m_lvl = lvl;

        auto win = CCDirector::get()->getWinSize();

        auto container = CCNode::create();
        container->setPosition(win.width / 2, win.height / 2);
        this->addChild(container);

        auto bg = CCScale9Sprite::create("GJ_square01.png");
        bg->setContentSize({ 340.f, 250.f });
        bg->setPosition(0.f, 0.f);
        container->addChild(bg);

        auto title = CCLabelBMFont::create("STATISTICS", "goldFont.fnt");
        title->setPosition(0.f, 100.f);
        title->setScale(0.7f);
        container->addChild(title);

        if (showTotal) {
            auto lbl = CCLabelBMFont::create(fmt::format("OPTIMAL: {} RUNS", rns.size()).c_str(), "chatFont.fnt");
            lbl->setScale(0.6f);
            lbl->setPosition(0.f, 70.f);
            container->addChild(lbl);
        }

        auto listSize = CCSize(280.f, 140.f);
        auto center = CCPoint(win.width / 2, win.height / 2);
        float yOffset = showTotal ? 0.f : 20.f;

        auto scrollBg = CCScale9Sprite::create("square02_small.png");
        scrollBg->setContentSize(listSize);
        scrollBg->setOpacity(100);
        scrollBg->setPosition(center.x, center.y - 10.f + yOffset);
        this->addChild(scrollBg);

        auto scroll = ScrollLayer::create(listSize);
        scroll->setPosition(center.x - listSize.width / 2, center.y - listSize.height / 2 - 10.f + yOffset);
        scroll->setTouchPriority(-501);
        this->addChild(scroll);

        float totHeight = rns.size() * 25.f;
        if (totHeight < listSize.height) totHeight = listSize.height;
        scroll->m_contentLayer->setContentSize({ listSize.width, totHeight });

        float currentY = totHeight - 20.f;
        int index = 1;
        for (const auto& r : rns) {
            std::string text = fmt::format("{}.  {}% - {}%", index, (int)r.start, (int)r.end);
            auto label = CCLabelBMFont::create(text.c_str(), "chatFont.fnt");
            label->setScale(0.6f);
            label->setAnchorPoint({ 0.f, 0.5f });
            label->setPosition(10.f, currentY);
            scroll->m_contentLayer->addChild(label);
            currentY -= 25.f;
            index++;
        }

        if (!m_buttonMenu) {
            m_buttonMenu = CCMenu::create();
            this->addChild(m_buttonMenu);
        }
        else {
            m_buttonMenu->removeAllChildrenWithCleanup(true);
        }

        auto closeBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("OK", "goldFont.fnt", "GJ_button_02.png", 0.6f), this, menu_selector(RunsPopup::onClose));
        m_buttonMenu->addChild(closeBtn);

        if (m_lvl->m_levelID.value() > 0) {
            bool linked = RT::links.count(m_lvl->m_levelID.value()) > 0;
            auto linkSpr = ButtonSprite::create(linked ? "Unlink" : "Link", "goldFont.fnt", linked ? "GJ_button_06.png" : "GJ_button_01.png", 0.6f);
            auto linkBtn = CCMenuItemSpriteExtra::create(linkSpr, this, menu_selector(RunsPopup::onLink));
            m_buttonMenu->addChild(linkBtn);

            m_buttonMenu->alignItemsHorizontallyWithPadding(15.f);
        }

        m_buttonMenu->setPosition(win.width / 2, win.height / 2 - 95.f);

        container->setScale(0.1f);
        container->runAction(CCEaseElasticOut::create(CCScaleTo::create(0.4f, 1.0f), 0.5f));

        m_buttonMenu->setScale(0.1f);
        m_buttonMenu->runAction(CCEaseElasticOut::create(CCScaleTo::create(0.4f, 1.0f), 0.5f));
        return true;
    }

    void onLink(CCObject*) {
        int id = m_lvl->m_levelID.value();

        if (RT::links.count(id)) {
            RT::links.erase(id);
            RT::saveLnk();
            FLAlertLayer::create("Unlinked", "Editor statistics are no longer merged.", "OK")->show();
            this->onClose(nullptr);
        }
        else {
            auto popup = LinkPopup::create(m_lvl);
            CCDirector::get()->getRunningScene()->addChild(popup, 100);
            this->onClose(nullptr);
        }
    }

public:
    static RunsPopup* create(std::vector<Run> rns, bool showTotal, GJGameLevel* lvl) {
        auto ret = new RunsPopup();
        if (ret && ret->init(rns, showTotal, lvl)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    void onClose(CCObject*) { this->removeFromParentAndCleanup(true); }
};

inline void showRunsMenu(GJGameLevel* lvl) {
    std::string key = RT::getLvlKey(lvl);
    std::vector<Run> all;

    if (RT::runs.count(key)) {
        all = RT::runs[key];
    }

    if (lvl->m_levelID.value() > 0) {
        int id = lvl->m_levelID.value();
        if (RT::links.count(id)) {
            std::string name = RT::links[id];
            if (RT::runs.count(name)) {
                auto& editorRuns = RT::runs[name];
                all.insert(all.end(), editorRuns.begin(), editorRuns.end());
            }
        }
    }

    if (all.empty()) {
        FLAlertLayer::create("Statistics", "No data recorded.", "OK")->show();
        return;
    }

    bool done = false;
    for (const auto& r : all) {
        if (r.end >= 100.0f) {
            done = true;
            break;
        }
    }

    std::sort(all.begin(), all.end(), [](const Run& a, const Run& b) {
        if (a.start == b.start) return a.end > b.end;
        return a.start < b.start;
        });

    std::vector<Run> best;
    float tgt = 0.0f;
    size_t i = 0;

    while (i < all.size()) {
        if (all[i].start > tgt) tgt = all[i].start;
        float maxEnd = tgt;
        Run bestRun = { -1.0f, -1.0f };
        while (i < all.size() && all[i].start <= tgt) {
            if (all[i].end > maxEnd) {
                maxEnd = all[i].end;
                bestRun = all[i];
            }
            i++;
        }
        if (bestRun.end > -1.0f) {
            best.push_back(bestRun);
            tgt = maxEnd;
        }
        if (tgt >= 100.0f) break;
    }

    auto popup = RunsPopup::create(best, done, lvl);
    CCDirector::get()->getRunningScene()->addChild(popup, 100);
}

inline void checkUpdates(GJGameLevel* lvl) {
    static bool checked = false;
    if (checked) {
        showRunsMenu(lvl);
        return;
    }

    std::thread([lvl]() {
        checked = true;

        auto response = web::WebRequest().getSync("https://raw.githubusercontent.com/tonnerregamer/runs-tracker/main/version.txt");

        Loader::get()->queueInMainThread([lvl, response]() {
            if (response.ok()) {
                auto resStr = response.string();
                if (resStr) {
                    std::string latest = resStr.unwrapOr("");
                    latest.erase(std::remove_if(latest.begin(), latest.end(), ::isspace), latest.end());

                    auto latestRes = geode::VersionInfo::parse(latest);
                    if (latestRes) {
                        auto latestVer = latestRes.unwrap();
                        auto currentVer = Mod::get()->getVersion();

                        if (semverCompare(currentVer, latestVer)) {
                            geode::createQuickPopup(
                                "Update Available",
                                "A newer version of Runs Tracker is available. Would you like to open the download page?",
                                "No", "Yes",
                                [](auto, bool btn2) {
                                    if (btn2) {
                                        geode::utils::web::openLinkInBrowser("https://github.com/tonnerregamer/runs-tracker/releases");
                                    }
                                }
                            );
                        }
                        else {
                            showRunsMenu(lvl);
                        }
                    }
                    else {
                        showRunsMenu(lvl);
                    }
                }
                else {
                    showRunsMenu(lvl);
                }
            }
            else {
                showRunsMenu(lvl);
            }
            });
        }).detach();
}