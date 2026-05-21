#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>

using namespace geode::prelude;

struct Run {
    float start;
    float end;
};

namespace RT {
    inline std::map<int, std::string> links;
    inline std::map<std::string, std::vector<Run>> runs;

    inline std::string getLvlKey(GJGameLevel* lvl) {
        if (lvl->m_levelID.value() > 0) return std::to_string(lvl->m_levelID.value());
        return std::string(lvl->m_levelName);
    }

    inline float getAbsPct(PlayLayer* layer) {
        if (!layer || layer->m_levelLength == 0.f) return 0.0f;
        float pct = (layer->m_player1->getPositionX() / layer->m_levelLength) * 100.0f;
        return std::clamp(pct, 0.0f, 100.0f);
    }

    inline void saveLnk() {
        auto path = Mod::get()->getSaveDir() / "links.txt";
        std::ofstream file(path.string());
        for (auto const& [id, name] : links) {
            file << id << "|" << name << "\n";
        }
    }

    inline void loadLnk() {
        auto path = Mod::get()->getSaveDir() / "links.txt";
        std::ifstream file(path.string());
        if (!file.is_open()) return;

        std::string line;
        while (std::getline(file, line)) {
            size_t sep = line.find('|');
            if (sep != std::string::npos) {
                int id = std::stoi(line.substr(0, sep));
                std::string name = line.substr(sep + 1);
                links[id] = name;
            }
        }
    }

    inline void saveRns() {
        auto path = Mod::get()->getSaveDir() / "runs_data.txt";
        std::ofstream file(path.string());
        if (file.is_open()) {
            for (auto const& [key, rns] : runs) {
                file << key << "|";
                for (const auto& r : rns) {
                    file << r.start << "," << r.end << " ";
                }
                file << "\n";
            }
            file.close();
        }
    }

    inline void loadRns() {
        auto path = Mod::get()->getSaveDir() / "runs_data.txt";
        std::ifstream file(path.string());
        if (!file.is_open()) return;

        std::string line;
        while (std::getline(file, line)) {
            size_t sep = line.find('|');
            if (sep != std::string::npos) {
                std::string key = line.substr(0, sep);
                std::string rnsPart = line.substr(sep + 1);
                std::stringstream ss(rnsPart);
                std::string runStr;
                while (ss >> runStr) {
                    size_t commaPos = runStr.find(',');
                    if (commaPos != std::string::npos) {
                        float s = std::stof(runStr.substr(0, commaPos));
                        float e = std::stof(runStr.substr(commaPos + 1));
                        runs[key].push_back({ s, e });
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
                        runs[key].push_back({ s, e });
                    }
                }
            }
        }
        file.close();
    }
}