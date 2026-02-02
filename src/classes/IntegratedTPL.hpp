#pragma once
#include <Geode/utils/web.hpp>
#include <string>
#include <vector>

struct IDListChallenge {
    int levelID = 0;
    int position = 0;
    std::string name;
    std::string author;
    std::string video;

    bool operator==(const IDListChallenge& other) const {
        return levelID == other.levelID && position == other.position;
    }
};

class IntegratedTPL {
public:
    static std::vector<IDListChallenge> list;
    static bool listLoaded;

    static void loadList(geode::EventListener<geode::utils::web::WebTask>&, std::function<void()>, std::function<void(int)>);
};