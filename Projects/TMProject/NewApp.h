#pragma once

class EventTranslator;
class RenderDevice;
class CSoundManager;
class TMVideoWnd;
class DS_SOUND_MANAGER;
class JBlur;
class TimerManager;
class ObjectManager;
class CPSock;

// Bumped when the meaning of any Config slot changes. Config[0] is a 1-based index
// into g_DisplayModeList, so a stale file silently selects a different resolution:
// the old and new mode lists are both 11 entries long, and bounds-checking cannot
// tell them apart. A version mismatch resets the stored settings instead.
//
// Deliberately outside the official 76xx sequence the retail updater steps through
// (7604 and up, one per update package). A value inside that range would be written
// by a real update and silently accepted here with a different meaning.
constexpr short CONFIG_VERSION = 20000;

struct SaveUpdatAndConfig
{
    short Version;
    short Config[14];
};

class NewApp
{
public:
    NewApp();
    virtual ~NewApp();

    virtual HRESULT Initialize(HINSTANCE hInstance, int nFull);
    HRESULT InitDevice();
    void InitMusicList();
    virtual HRESULT Finalize();
    virtual DWORD Run();
    virtual void OnCreate(HWND hWnd, DWORD wParam, int lParam);
    virtual HRESULT RenderScene();
    virtual void SetObjectManager(ObjectManager* pManager);
    virtual EventTranslator* GetEventTranslator();

    HWND GetSafeHwnd();
    void SwitchWebBrowserState(int nEmptyCargo);
    void SwitchWebBoard();
    DWORD GetHttpRequest(char* httpname, char* Request, int MaxBuffer);
    void MixHelp();
    HRESULT MsgProc(HWND hWnd, DWORD uMsg, DWORD wParam, int lParam);

    char base_chinaTid(char* TID, char* Id);

public:
    HWND m_hWnd;
    int m_bActive;
    unsigned int m_dwWindowStyle;
    unsigned int m_dwScreenWidth;
    unsigned int m_dwScreenHeight;
    unsigned int m_dwColorBit;
    int m_bwFullScreen;
    char m_strWindowTitle[64];
    RenderDevice* m_pRenderDevice;
    CSoundManager* m_pSoundManager;
    TMVideoWnd* m_pAviPlayer;
    DS_SOUND_MANAGER* m_pBGMManager;
    EventTranslator* m_pEventTranslator;
    TimerManager* m_pTimerManager;
    ObjectManager* m_pObjectManager;
    CPSock* m_pSocketManager;
    int m_bTex;
    int m_nSound;
    int m_nMusic;
    int m_nCameraView;
    int m_Winstate;
    bool m_binactive;
    HBITMAP m_hBMBtnBG;
    bool china_bWrite;
    int china_Playtime;
    JBlur* m_pBlur;
};

void CreateConsole();