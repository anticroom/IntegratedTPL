#include "../classes/IDListLayer.hpp"
#include <Geode/modify/LevelSearchLayer.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>

using namespace geode::prelude;

class $modify(IDLevelSearchLayer, LevelSearchLayer) {
    bool init(int searchType) {
        if (!LevelSearchLayer::init(searchType)) return false;

        auto challengeListButtonSprite = CircleButtonSprite::createWithSprite("ID_challengeBtn_001.png"_spr);
        challengeListButtonSprite->getTopNode()->setScale(1.0f);
        challengeListButtonSprite->setScale(0.8f);
        auto challengeListButton = CCMenuItemSpriteExtra::create(challengeListButtonSprite, this, menu_selector(IDLevelSearchLayer::onChallengeListLevels));
        challengeListButton->setID("challenge-list-button"_spr);
        if (auto menu = getChildByID("other-filter-menu")) {
            menu->addChild(challengeListButton);
            menu->updateLayout();
        }

        return true;
    }

    void onChallengeListLevels(CCObject* sender) {
        CCDirector::get()->pushScene(CCTransitionFade::create(0.5f, IDListLayer::scene()));
    }
};