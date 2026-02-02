#include "IntegratedTPL.hpp"
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

std::vector<IDListChallenge> IntegratedTPL::list;
bool IntegratedTPL::listLoaded = false;

constexpr const char* listUrl = "https://www.thepisslist.com/api/levels";

void IntegratedTPL::loadList(EventListener<web::WebTask>& listener, std::function<void()> success, std::function<void(int)> failure) {
    listener.bind([failure = std::move(failure), success = std::move(success)](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) return failure(res->code());

            auto json = res->json();
            if (!json.isOk()) return failure(-1);

            listLoaded = true;
            list.clear();

            auto root = json.unwrap();

            if (root.isArray()) {
                for (auto const& levelObj : root.asArray().unwrap()) {
                    
                    if (!levelObj.contains("id") || !levelObj["id"].isNumber()) continue;
                    int id = levelObj["id"].asInt().unwrapOr(0);

                    if (!levelObj.contains("rank") || !levelObj["rank"].isNumber()) continue;
                    int rank = levelObj["rank"].asInt().unwrapOr(0);

                    std::string name = levelObj.contains("name") 
                        ? levelObj["name"].asString().unwrapOr("Unknown") 
                        : "Unknown";
                        
                    std::string author = levelObj.contains("author") 
                        ? levelObj["author"].asString().unwrapOr("Unknown") 
                        : "Unknown";
                        
                    std::string video = levelObj.contains("verification") 
                        ? levelObj["verification"].asString().unwrapOr("") 
                        : "";

                    IDListChallenge challenge;
                    challenge.levelID = id;
                    challenge.position = rank;
                    challenge.name = name;
                    challenge.author = author;
                    challenge.video = video;

                    list.push_back(challenge);
                }
            }

            std::sort(list.begin(), list.end(), [](const IDListChallenge& a, const IDListChallenge& b) {
                return a.position < b.position;
            });

            success();
        }
    });

    listener.setFilter(web::WebRequest().get(listUrl));
}