#include "IDListLayer.hpp"
#include "IntegratedTPL.hpp"
#include <Geode/binding/AppDelegate.hpp>
#include <Geode/binding/CustomListView.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/GJListLayer.hpp>
#include <Geode/binding/InfoAlertButton.hpp>
#include <Geode/binding/LoadingCircle.hpp>
#include <Geode/binding/SetIDPopup.hpp>
#include <Geode/loader/Mod.hpp>
#include <jasmine/random.hpp>
#include <jasmine/search.hpp>

using namespace geode::prelude;

IDListLayer* IDListLayer::create() {
    auto ret = new IDListLayer();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

CCScene* IDListLayer::scene() {
    auto ret = CCScene::create();
    AppDelegate::get()->m_runningScene = ret;
    ret->addChild(IDListLayer::create());
    return ret;
}

constexpr const char* listInfo = "<cg>The Piss List</c> is a challenge list for piss themed challenges at <cy>thepisslist.com</c>.\n Developed by <cr>anticroom</c> <3.";

bool IDListLayer::init() {
    if (!CCLayer::init()) return false;

    setID("IDListLayer");
    auto winSize = CCDirector::get()->getWinSize();

    m_pageCache = CCArray::create();
    m_pageCache->retain();

    auto bg = CCSprite::create("GJ_gradientBG.png");
    bg->setAnchorPoint({ 0.0f, 0.0f });
    bg->setScaleX((winSize.width + 10.0f) / bg->getTextureRect().size.width);
    bg->setScaleY((winSize.height + 10.0f) / bg->getTextureRect().size.height);
    bg->setPosition({ -5.0f, -5.0f });
    bg->setColor({ 51, 51, 51 });
    bg->setID("background");
    addChild(bg);

    auto bottomLeftCorner = CCSprite::createWithSpriteFrameName("gauntletCorner_001.png");
    bottomLeftCorner->setPosition({ -1.0f, -1.0f });
    bottomLeftCorner->setAnchorPoint({ 0.0f, 0.0f });
    bottomLeftCorner->setID("left-corner");
    addChild(bottomLeftCorner);

    auto bottomRightCorner = CCSprite::createWithSpriteFrameName("gauntletCorner_001.png");
    bottomRightCorner->setPosition({ winSize.width + 1.0f, -1.0f });
    bottomRightCorner->setAnchorPoint({ 1.0f, 0.0f });
    bottomRightCorner->setFlipX(true);
    bottomRightCorner->setID("right-corner");
    addChild(bottomRightCorner);

    m_countLabel = CCLabelBMFont::create("", "goldFont.fnt");
    m_countLabel->setAnchorPoint({ 1.0f, 1.0f });
    m_countLabel->setScale(0.6f);
    m_countLabel->setPosition({ winSize.width - 7.0f, winSize.height - 3.0f });
    m_countLabel->setID("level-count-label");
    addChild(m_countLabel);

    m_list = GJListLayer::create(nullptr, "The Piss List", { 0, 0, 0, 180 }, 356.0f, 220.0f, 0);
    m_list->setPosition(winSize / 2.0f - m_list->getContentSize() / 2.0f);
    m_list->setID("GJListLayer");
    addChild(m_list, 2);

    m_searchBarMenu = CCMenu::create();
    m_searchBarMenu->setContentSize({ 356.0f, 30.0f });
    m_searchBarMenu->setPosition({ 0.0f, 190.0f });
    m_searchBarMenu->setID("search-bar-menu");
    m_list->addChild(m_searchBarMenu);

    auto searchBackground = CCLayerColor::create({ 194, 114, 62, 255 }, 356.0f, 30.0f);
    searchBackground->setID("search-bar-background");
    m_searchBarMenu->addChild(searchBackground);

    m_searchButton = CCMenuItemExt::createSpriteExtraWithFrameName("gj_findBtn_001.png", 0.7f, [this](auto) {
        search();
    });
    m_searchButton->setPosition({ 337.0f, 15.0f });
    m_searchButton->setID("search-button");
    m_searchBarMenu->addChild(m_searchButton);

    m_searchBar = TextInput::create(200.f, "Search...");
    m_searchBar->setPosition({ 165.0f, 15.0f });
    m_searchBar->setID("search-bar");
    m_searchBar->setCallback([this](std::string const&) {
    });
    m_searchBarMenu->addChild(m_searchBar);

    auto menu = CCMenu::create();
    menu->setPosition({ 0.0f, 0.0f });
    menu->setID("button-menu");
    addChild(menu);

    m_backButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_01_001.png", 1.0f, [this](auto) {
        CCDirector::get()->popSceneWithTransition(0.5f, kPopTransitionFade);
    });
    m_backButton->setPosition({ 25.0f, winSize.height - 25.0f });
    m_backButton->setID("back-button");
    menu->addChild(m_backButton);

    m_leftButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_03_001.png", 1.0f, [this](auto) {
        page(m_page - 1);
    });
    m_leftButton->setPosition({ 24.0f, winSize.height / 2.0f });
    m_leftButton->setID("prev-page-button");
    menu->addChild(m_leftButton);

    auto rightBtnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    rightBtnSpr->setFlipX(true);
    m_rightButton = CCMenuItemExt::createSpriteExtra(rightBtnSpr, [this](auto) {
        page(m_page + 1);
    });
    m_rightButton->setPosition({ winSize.width - 24.0f, winSize.height / 2.0f });
    m_rightButton->setID("next-page-button");
    menu->addChild(m_rightButton);

    m_infoButton = InfoAlertButton::create("About", listInfo, 1.0f);
    m_infoButton->setPosition({ 30.0f, 30.0f });
    m_infoButton->setID("info-button");
    menu->addChild(m_infoButton, 2);

    m_listFailure = [this](int code) {
        FLAlertLayer::create(fmt::format("Load Failed ({})", code).c_str(), "Failed to load list. Please try again later.", "OK")->show();
        m_loadingCircle->setVisible(false);
    };

    auto refreshBtnSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
    auto refreshButton = CCMenuItemExt::createSpriteExtra(refreshBtnSpr, [this](auto) {
        showLoading();
        IntegratedTPL::loadList(m_listListener, [this] {
            populateList(m_query);
        }, m_listFailure);
    });
    refreshButton->setPosition({ winSize.width - refreshBtnSpr->getContentWidth() / 2.0f - 4.0f, refreshBtnSpr->getContentHeight() / 2.0f + 4.0f });
    refreshButton->setID("refresh-button");
    menu->addChild(refreshButton, 2);

    auto pageBtnSpr = CCSprite::create("GJ_button_02.png");
    pageBtnSpr->setScale(0.7f);
    m_pageLabel = CCLabelBMFont::create("1", "bigFont.fnt");
    m_pageLabel->setScale(0.8f);
    m_pageLabel->setPosition(pageBtnSpr->getContentSize() / 2.0f);
    pageBtnSpr->addChild(m_pageLabel);
    m_pageButton = CCMenuItemExt::createSpriteExtra(pageBtnSpr, [this](auto) {
        auto popup = SetIDPopup::create(m_page + 1, 1, (m_fullSearchResults.size() + 9) / 10, "Go to Page", "Go", true, 1, 60.0f, false, false);
        popup->m_delegate = this;
        popup->show();
    });
    m_pageButton->setPositionY(winSize.height - 39.5f);
    m_pageButton->setID("page-button");
    menu->addChild(m_pageButton);

    m_randomButton = CCMenuItemExt::createSpriteExtraWithFilename("BI_randomBtn_001.png"_spr, 0.9f, [this](auto) {
        page(jasmine::random::getInt(0, (m_fullSearchResults.size() - 1) / 10));
    });
    m_randomButton->setPositionY(
        m_pageButton->getPositionY() - m_pageButton->getContentHeight() / 2.0f - m_randomButton->getContentHeight() / 2.0f - 5.0f);
    m_randomButton->setID("random-button");
    menu->addChild(m_randomButton);

    auto lastArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    lastArrow->setFlipX(true);
    auto otherLastArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    otherLastArrow->setPosition(lastArrow->getContentSize() / 2.0f + CCPoint { 20.0f, 0.0f });
    otherLastArrow->setFlipX(true);
    lastArrow->addChild(otherLastArrow);
    lastArrow->setScale(0.4f);
    m_lastButton = CCMenuItemExt::createSpriteExtra(lastArrow, [this](auto) {
        page((m_fullSearchResults.size() - 1) / 10);
    });
    m_lastButton->setPositionY(
        m_randomButton->getPositionY() - m_randomButton->getContentHeight() / 2.0f - m_lastButton->getContentHeight() / 2.0f - 5.0f);
    m_lastButton->setID("last-button");
    menu->addChild(m_lastButton);

    auto x = winSize.width - m_randomButton->getContentWidth() / 2.0f - 3.0f;
    m_pageButton->setPositionX(x);
    m_randomButton->setPositionX(x);
    m_lastButton->setPositionX(x - 4.0f);

    auto firstArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    auto otherFirstArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    otherFirstArrow->setPosition(firstArrow->getContentSize() / 2.0f - CCPoint { 20.0f, 0.0f });
    firstArrow->addChild(otherFirstArrow);
    firstArrow->setScale(0.4f);
    m_firstButton = CCMenuItemExt::createSpriteExtra(firstArrow, [this](auto) {
        page(0);
    });
    m_firstButton->setPosition({ 21.5f, m_lastButton->getPositionY() });
    m_firstButton->setID("first-button");
    menu->addChild(m_firstButton);

    m_loadingCircle = LoadingCircle::create();
    m_loadingCircle->setParentLayer(this);
    m_loadingCircle->setID("loading-circle");
    m_loadingCircle->show();

    showLoading();
    setKeypadEnabled(true);
    setKeyboardEnabled(true);

    if (IntegratedTPL::listLoaded) {
        populateList("");
    }
    else {
        IntegratedTPL::loadList(m_listListener, [this] {
            populateList("");
        }, m_listFailure);
    }

    return true;
}

void IDListLayer::showLoading() {
    m_pageLabel->setString(fmt::to_string(m_page + 1).c_str());
    m_loadingCircle->setVisible(true);
    if (auto listView = m_list->m_listView) listView->setVisible(false);
    m_searchBarMenu->setVisible(false);
    m_countLabel->setVisible(false);
    m_leftButton->setVisible(false);
    m_rightButton->setVisible(false);
    m_firstButton->setVisible(false);
    m_lastButton->setVisible(false);
    m_pageButton->setVisible(false);
    m_randomButton->setVisible(false);
}

void IDListLayer::populateList(const std::string& query) {
    m_fullSearchResults.clear();
    
    auto btn = static_cast<CCMenuItemSpriteExtra*>(m_searchButton);
    auto searchSprite = static_cast<CCSprite*>(btn->getNormalImage());
    
    if (query.empty()) {
        for (auto& level : IntegratedTPL::list) {
            m_fullSearchResults.push_back(fmt::to_string(level.levelID));
        }
        searchSprite->setDisplayFrame(CCSpriteFrameCache::get()->spriteFrameByName("gj_findBtn_001.png"));
    }
    else {
        auto lowerQuery = string::toLower(query);
        for (auto& level : IntegratedTPL::list) {
            if (!string::toLower(level.name).contains(lowerQuery)) continue;
            m_fullSearchResults.push_back(fmt::to_string(level.levelID));
        }
        auto texture = CCTextureCache::get()->addImage("ID_findBtnOn_001.png"_spr, false);
        searchSprite->setDisplayFrame(CCSpriteFrame::createWithTexture(texture, { { 0.0f, 0.0f }, texture->getContentSize() }));
    }

    m_query = query;

    if (m_fullSearchResults.empty()) {
        loadLevelsFinished(CCArray::create(), "", 0);
        m_countLabel->setString("");
    }
    else {
        auto glm = GameLevelManager::get();
        glm->m_levelManagerDelegate = this;

        if (m_pageCache) m_pageCache->removeAllObjects();

        auto searchObject = jasmine::search::getObject(
            m_fullSearchResults.begin() + m_page * 10,
            std::min(m_fullSearchResults.end(), m_fullSearchResults.begin() + (m_page + 1) * 10)
        );

        if (auto storedLevels = glm->getStoredOnlineLevels(jasmine::search::getKey(searchObject))) {
            loadLevelsFinished(storedLevels, "", 0);
            setupPageInfo("", "");
        }
        else glm->getOnlineLevels(searchObject);
    }
}

void IDListLayer::loadLevelsFinished(CCArray* levels, const char*, int) {
    if (!m_pageCache) {
        m_pageCache = CCArray::create();
        m_pageCache->retain();
    }

    if (levels && levels->count() > 1) {
        m_pageCache->removeAllObjects();
        m_pageCache->addObjectsFromArray(levels);
    } 
    else if (levels && levels->count() == 1) {
        auto newLevel = static_cast<GJGameLevel*>(levels->objectAtIndex(0));
        bool replaced = false;
        
        for (int i = 0; i < m_pageCache->count(); i++) {
            auto cached = static_cast<GJGameLevel*>(m_pageCache->objectAtIndex(i));
            if (cached->m_levelID == newLevel->m_levelID) {
                m_pageCache->replaceObjectAtIndex(i, newLevel);
                replaced = true;
                break;
            }
        }
        if (!replaced) m_pageCache->addObject(newLevel);
    }

    if (auto listView = m_list->m_listView) {
        listView->removeFromParent();
        listView->release();
    }

    auto combinedLevels = CCArray::create();
    size_t start = m_page * 10;
    size_t end = std::min(m_fullSearchResults.size(), start + 10);

    for (size_t i = start; i < end; i++) {
        int expectedID = 0;
        try { expectedID = std::stoi(m_fullSearchResults[i]); } catch (...) {}

        GJGameLevel* foundLevel = nullptr;

        for (int j = 0; j < m_pageCache->count(); j++) {
            auto lvl = static_cast<GJGameLevel*>(m_pageCache->objectAtIndex(j));
            if (lvl->m_levelID == expectedID) {
                foundLevel = lvl;
                break;
            }
        }

        if (!foundLevel) {
            foundLevel = GameLevelManager::sharedState()->getSavedLevel(expectedID);
        }

        if (!foundLevel) {
            for (const auto& challenge : IntegratedTPL::list) {
                if (challenge.levelID == expectedID) {
                    foundLevel = GJGameLevel::create();
                    foundLevel->m_levelID = challenge.levelID;
                    foundLevel->m_levelName = challenge.name;
                    foundLevel->m_creatorName = challenge.author;
                    foundLevel->m_stars = 10;
                    foundLevel->m_demon = 1; 
                    foundLevel->m_demonDifficulty = 6; 
                    foundLevel->m_levelLength = 4;     
                    
                    auto searchObject = GJSearchObject::create(SearchType::Search, std::to_string(expectedID));
                    GameLevelManager::sharedState()->getOnlineLevels(searchObject);
                    break;
                }
            }
        }

        if (foundLevel) {
            combinedLevels->addObject(foundLevel);
        }
    }

    auto listView = CustomListView::create(combinedLevels, BoomListType::Level, 190.0f, 356.0f);
    listView->retain();
    m_list->addChild(listView, 6, 9);
    m_list->m_listView = listView;

    m_searchBarMenu->setVisible(true);
    m_countLabel->setVisible(true);
    m_loadingCircle->setVisible(false);
    auto size = m_fullSearchResults.size();
    if (size > 10) {
        auto maxPage = (size - 1) / 10;
        m_leftButton->setVisible(m_page > 0);
        m_rightButton->setVisible(m_page < maxPage);
        m_firstButton->setVisible(m_page > 0);
        m_lastButton->setVisible(m_page < maxPage);
        m_pageButton->setVisible(true);
        m_randomButton->setVisible(true);
    }
}

void IDListLayer::loadLevelsFailed(const char*, int) {
    loadLevelsFinished(nullptr, "", 0);
}

void IDListLayer::setupPageInfo(gd::string, const char*) {
    m_countLabel->setString(fmt::format("{} to {} of {}", m_page * 10 + 1,
        std::min<int>(m_fullSearchResults.size(), (m_page + 1) * 10), m_fullSearchResults.size()).c_str());
    m_countLabel->limitLabelWidth(100.0f, 0.6f, 0.0f);
}

void IDListLayer::search() {
    std::string query = m_searchBar->getString();
    
    if (m_query != query) {
        showLoading();
        IntegratedTPL::loadList(m_listListener, [this, query] {
            m_page = 0;
            populateList(query);
        }, m_listFailure);
    }
}

void IDListLayer::page(int page) {
    auto maxPage = (m_fullSearchResults.size() + 9) / 10;
    m_page = maxPage > 0 ? (maxPage + (page % maxPage)) % maxPage : 0;
    showLoading();
    populateList(m_query);
}

void IDListLayer::keyDown(enumKeyCodes key) {
    switch (key) {
        case KEY_Left:
        case CONTROLLER_Left:
            if (m_leftButton->isVisible()) page(m_page - 1);
            break;
        case KEY_Right:
        case CONTROLLER_Right:
            if (m_rightButton->isVisible()) page(m_page + 1);
            break;
        case KEY_Enter:
            search();
            break;
        default:
            CCLayer::keyDown(key);
            break;
    }
}

void IDListLayer::keyBackClicked() {
    CCDirector::get()->popSceneWithTransition(0.5f, kPopTransitionFade);
}

void IDListLayer::setIDPopupClosed(SetIDPopup*, int page) {
    m_page = std::clamp<int>(page - 1, 0, (m_fullSearchResults.size() - 1) / 10);
    showLoading();
    populateList(m_query);
}

IDListLayer::~IDListLayer() {
    if (m_pageCache) {
        m_pageCache->release();
        m_pageCache = nullptr;
    }

    auto glm = GameLevelManager::get();
    if (glm->m_levelManagerDelegate == this) glm->m_levelManagerDelegate = nullptr;
}