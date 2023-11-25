#include "plugin.h"
#include "T_CB_Generic.h"
#include "CTxdStore.h"
#include "CMenuManager.h"
#include "audFrontendAudioEntity.h"
#include "CControllerConfigManager.h"
#include "CPad.h"
#include "CFont.h"
#include "CHudColours.h"
#include "CText.h"

#include "Utility.h"

using namespace plugin;

class ConsoleSelectMenuIV {
public:
    static inline CSprite2d gtaiv = {};
    static inline CSprite2d gtatlad = {};
    static inline CSprite2d gtatbogt = {};
    static inline CSprite2d gta = {};
    static inline int32_t selectedEpisode = 1;
    static inline int32_t prevEpisode = 1;
    static inline bool drawMouse = false;
    static inline bool checkHover = false;

    static void DrawCharacter(CSprite2d* sprite, int32_t i, bool selected) {
        float hw = 205.0f;
        float h = 550.0f;
        float spacing = (hw * 0.5f) + 56.0f;
        float y = 86.0f;

        if (selected) {
            float size = 24.0f;
            hw += size / 2;
            h += size;
            y -= hw * 0.06f;
        }

        float x = (DEFAULT_SCREEN_WIDTH / 2) - hw - spacing - 6.0f;

        x += ((hw + spacing) * i);

        rage::fwRect rect;
        rect.left = ScaleXKeepCentered(x - hw);
        rect.top = ScaleY(y);
        rect.right = ScaleXKeepCentered(x + hw);
        rect.bottom = rect.top + ScaleY(h);

        sprite->SetRenderState();
        sprite->Draw(rect, rage::Color32(255, 255, 255, selected ? 255 : 90));

        rect.left += ScaleX(64.0f);
        rect.right -= ScaleY(64.0f);
        rect.bottom -= ScaleY(64.0f);
        if (CheckHover(rect) && drawMouse) {
            prevEpisode = selectedEpisode;
            selectedEpisode = i;
            checkHover = true;
        }
    }

    static bool GetLeft() {
        return !ControlsManager[CONTROLLER_KEYBOARD1].m_actions[INPUT_FRONTEND_LEFT].m_nNewState && ControlsManager[CONTROLLER_KEYBOARD1].m_actions[INPUT_FRONTEND_LEFT].m_nOldState;
    }

    static bool GetRight() {
        return !ControlsManager[CONTROLLER_KEYBOARD1].m_actions[INPUT_FRONTEND_RIGHT].m_nNewState && ControlsManager[CONTROLLER_KEYBOARD1].m_actions[INPUT_FRONTEND_RIGHT].m_nOldState;
    }

    static bool GetEnter() {
        return !ControlsManager[CONTROLLER_KEYBOARD1].m_actions[INPUT_FRONTEND_ACCEPT].m_nNewState && ControlsManager[CONTROLLER_KEYBOARD1].m_actions[INPUT_FRONTEND_ACCEPT].m_nOldState;
    }
    
    static bool GetQuit() {
        return !ControlsManager[CONTROLLER_KEYBOARD1].m_actions[INPUT_FRONTEND_PAUSE].m_nNewState && ControlsManager[CONTROLLER_KEYBOARD1].m_actions[INPUT_FRONTEND_PAUSE].m_nOldState;
    }

    static bool GetLMB() {
        if (!checkHover)
            return false;

        return ControlsManager[CONTROLLER_KEYBOARD1].m_actions[INPUT_MOUSE_LMB].m_nNewState && !ControlsManager[CONTROLLER_KEYBOARD1].m_actions[INPUT_MOUSE_LMB].m_nOldState;
    }

    static void Dummy() {

    }

    static void SetEpisodeString() {
        int32_t ep = EPISODE_IV;
        switch (selectedEpisode) {
            case 0:
                ep = EPISODE_TBOGT;
                break;
            case 2:
                ep = EPISODE_TLAD;
                break;
        }

        static char buf[2];
        sprintf(buf, "%d", ep);
        CMenuManager::m_EpisodeStr = buf;
    }

    static void DrawHelpText() {
        CFont::SetProportional(true);
        CFont::SetBackground(false, false);
        CFont::SetEdge(0.0f);
        CFont::SetFontStyle(8);
        CFont::SetColor(CHudColours::Get(HUD_COLOUR_GREY));
        CFont::SetDropColor(CHudColours::Get(HUD_COLOUR_BLACK));
        CFont::SetOrientation(ALIGN_RIGHT);
        CFont::SetScale(0.2f, 0.38f);

        std::wstring str = {};
        str += L"~INPUT_FRONTEND_LEFT~ ~INPUT_FRONTEND_RIGHT~  ";
        str += TheText.Get(0x1DA5B02D, 0); // Select
        str += L"    ";

        str += L"~INPUT_FRONTEND_ACCEPT~  ";
        str += TheText.Get(0x3A4F4766, 0); // Start
        str += L"    ";

        str += L"~INPUT_FRONTEND_PAUSE~  ";
        str += TheText.Get(0x92480A6F, 0); // Quit

        CFont::SetWrapx(0.0f, 0.92f);
        CFont::PrintString(0.0f, 0.88f, str.c_str());
        CFont::DrawFonts();
    }

    ConsoleSelectMenuIV() {
        plugin::Events::initEngineEvent += []() {
            int32_t slot = CTxdStore::AddTxdSlot("console_select_menu");
            CTxdStore::LoadTxd(slot, "platform:/textures/console_select_menu");
            CTxdStore::AddRef(slot);
            CTxdStore::PushCurrentTxd();
            CTxdStore::SetCurrentTxd(slot);

            gtaiv.SetTexture("iv");
            gtatlad.SetTexture("tlad");
            gtatbogt.SetTexture("tbogt");
            gta.SetTexture("gta");

            CTxdStore::PopCurrentTxd();
        };

        plugin::Events::shutdownEngineEvent += []() {
            gtaiv.Delete();
            gtatlad.Delete();
            gtatbogt.Delete();
            gta.Delete();

            int32_t slot = CTxdStore::FindTxdSlot("console_select_menu");
            CTxdStore::RemoveTxdSlot(slot);
        };

        plugin::patch::RedirectJump(0x5A3090, Dummy);

        plugin::Events::menuProcessEvent += []() {
            if (CMenuManager::m_DefaultFrontend || CMenuManager::m_Refresh)
                return;

            if (CMenuManager::m_CurrState > 0)
                return;

            if (CMenuManager::m_CurrScreen != MENUPAGE_MAIN_SELECT)
                return;

            if (GetLeft()) {
                prevEpisode = selectedEpisode;
                selectedEpisode--;
                drawMouse = false;
            }

            if (GetRight()) {
                prevEpisode = selectedEpisode;
                selectedEpisode++;
                drawMouse = false;
            }

            if (selectedEpisode < 0)
                selectedEpisode = 2;

            if (selectedEpisode > 2)
                selectedEpisode = 0;

            if (selectedEpisode != prevEpisode) {
                prevEpisode = selectedEpisode;
                FrontendAudioEntity.ReportFrontendAudioEvent("FRONTEND_MENU_HIGHLIGHT");
            }

            bool lmb = GetLMB();
            if (lmb || GetEnter()) {
                SetEpisodeString();
                CMenuManager::m_SelectEpisode = 0;
                CMenuManager::m_DefaultFrontend = 1;
                CMenuManager::m_Refresh = 1;
                drawMouse = false;

                if (lmb)
                    FrontendAudioEntity.ReportFrontendAudioEvent("FRONTEND_MENU_SELECT");
            }

            if (GetQuit()) {
                CMenuManager::m_CurrState = MENUSTATE_QUIT_ASK;
                FrontendAudioEntity.ReportFrontendAudioEvent("FRONTEND_MENU_BACK");
                drawMouse = false;
            }

            if (CControllerConfigManager::m_UsingMouse) {
                drawMouse = true;
            }

            checkHover = false;
        };

        plugin::Events::drawMenuEvent.before += []() {
            if (CMenuManager::m_DefaultFrontend || CMenuManager::m_Refresh)
                return;

            if (CMenuManager::m_CurrState > 0)
                return;

            if (CMenuManager::m_CurrScreen != MENUPAGE_MAIN_SELECT)
                return;

            auto dc = new T_CB_Generic_NoArgs([]() {
                CSprite2d::DrawRect({ 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT }, { 0, 0, 0, 255 });

                DrawCharacter(&gtatbogt, 0, selectedEpisode == 0);
                DrawCharacter(&gtatlad, 2, selectedEpisode == 2);
                DrawCharacter(&gtaiv, 1, selectedEpisode == 1);

                DrawHelpText();

                if (drawMouse)
                    CMenuManager::DrawMouseCursor();
            });
            CBaseDC::InitStatic(dc);
        };

        // Use CE logo on intro :P
        static CdeclEvent <AddressList<0x5CC56D, H_CALL>, PRIORITY_AFTER, ArgPick2N<rage::grcTexturePC*, 0, uint8_t, 1>, void(rage::grcTexturePC*, uint8_t)> onSetLoadingScreenTex({ "E8 ? ? ? ? F3 0F 10 4C 24 ? F3 0F 10 5C 24 ? 83 C4 08" });
        onSetLoadingScreenTex += [](rage::grcTexturePC* tex, uint8_t) {
            if (!strcmp(tex->m_Name, "pack:/gta.dds")) {
                gta.SetRenderState();
            }
        };
    }
} consoleSelectMenuIV;
