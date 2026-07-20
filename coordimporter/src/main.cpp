#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <optional>
#include <windows.h>
#include <commdlg.h>

using namespace geode::prelude;

static std::optional<cocos2d::CCPoint> parseCoordinates(std::string const& line) {
    std::stringstream ss(line);
    std::string xStr, yStr;

    if (!std::getline(ss, xStr, ',')) return std::nullopt;
    if (!std::getline(ss, yStr)) return std::nullopt;

    try {
        float x = std::stof(xStr);
        float y = std::stof(yStr);
        return cocos2d::CCPoint(x, y);
    } catch (...) {
        return std::nullopt;
    }
}

static void loadCoordinatesFromFile(LevelEditorLayer* editor, std::filesystem::path const& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        FLAlertLayer::create("Error", "Could not open file", "OK")->show();
        return;
    }

    std::string line;
    int blocksSpawned = 0;
    int invalidLines = 0;

    while (std::getline(file, line)) {
        if (line.find_first_not_of(" \t\r\n") == std::string::npos) continue;

        auto parsed = parseCoordinates(line);
        if (!parsed.has_value()) {
            invalidLines++;
            continue;
        }

        if (auto obj = editor->createObject(1, parsed.value(), true)) {
            blocksSpawned++;
        }
    }

    std::string msg = "Spawned " + std::to_string(blocksSpawned) + " blocks";
    if (invalidLines > 0) {
        msg += "\nSkipped " + std::to_string(invalidLines) + " invalid lines";
    }
    FLAlertLayer::create("Import Result", msg.c_str(), "OK")->show();
}

static void triggerFilePicker(LevelEditorLayer* editor) {
    OPENFILENAMEA ofn;       
    char szFile[260] = {0};  

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn) == TRUE) {
        std::filesystem::path finalPath(ofn.lpstrFile);
        if (!finalPath.empty() && editor) {
            loadCoordinatesFromFile(editor, finalPath);
        }
    }
}

class $modify(MyEditorUI, EditorUI) {
    bool init(LevelEditorLayer* editor) {
        if (!EditorUI::init(editor)) return false;


        auto buttonSprite = cocos2d::CCSprite::create("iconBTN.png"_spr);

        if (!buttonSprite) {
            buttonSprite = cocos2d::CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
        }

        auto importButton = CCMenuItemSpriteExtra::create(
            buttonSprite,
            this,
            menu_selector(MyEditorUI::onImportCoordsClick)
        );

        if (importButton) {
            auto customMenu = CCMenu::create();
            customMenu->addChild(importButton);

            auto winSize = CCDirector::sharedDirector()->getWinSize();
            customMenu->setPosition({ winSize.width - 40.0f, winSize.height / 2.0f });

            this->addChild(customMenu, 100);
        }

        return true;
    }

    void onImportCoordsClick(CCObject*) {
        if (m_editorLayer) {
            triggerFilePicker(m_editorLayer);
        }
    }
};