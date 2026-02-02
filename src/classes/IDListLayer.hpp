#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/ui/TextInput.hpp>
#include "IntegratedTPL.hpp"

using namespace geode::prelude;

class IDListLayer : public CCLayer, public TextInputDelegate, public SetIDPopupDelegate, public LevelManagerDelegate {
protected:
    GJListLayer* m_list = nullptr;
    geode::TextInput* m_searchBar = nullptr; 
    
    LoadingCircle* m_loadingCircle = nullptr;
    EventListener<web::WebTask> m_listener;
    EventListener<web::WebTask> m_listListener;
    
    CCMenu* m_searchBarMenu = nullptr;
    CCLabelBMFont* m_countLabel = nullptr;
    CCMenuItem* m_searchButton = nullptr;
    CCMenuItem* m_leftButton = nullptr;
    CCMenuItem* m_rightButton = nullptr;
    CCMenuItem* m_firstButton = nullptr;
    CCMenuItem* m_lastButton = nullptr;
    CCMenuItem* m_pageButton = nullptr;
    CCMenuItem* m_randomButton = nullptr;
    CCMenuItem* m_backButton = nullptr;
    InfoAlertButton* m_infoButton = nullptr;
    CCLabelBMFont* m_pageLabel = nullptr;

    std::vector<std::string> m_fullSearchResults;
    std::string m_query;
    int m_page = 0;
    
    cocos2d::CCArray* m_pageCache = nullptr;

    virtual bool init();
    virtual void keyBackClicked();
    virtual void keyDown(enumKeyCodes key);

    void onBack(CCObject*);
    void showLoading();
    void populateList(const std::string& query);
    void search();
    void page(int page);

    virtual void loadLevelsFinished(cocos2d::CCArray* levels, const char* key, int type) override;
    virtual void loadLevelsFailed(const char* key, int type) override;
    virtual void setupPageInfo(gd::string, const char*) override; 

    virtual void setIDPopupClosed(SetIDPopup*, int page) override;

    std::function<void(int)> m_listFailure;

public:
    static IDListLayer* create();
    static CCScene* scene();
    ~IDListLayer();
};