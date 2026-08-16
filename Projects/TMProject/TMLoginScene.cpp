#include "pch.h"
#include "TMLoginScene.h"
#include "TMGlobal.h"
#include "DirShow.h"
#include "ObjectManager.h"
#include "TMHuman.h"
#include "SControlContainer.h"
#include "TMCamera.h"
#include "TMObject.h"
#include "TMGround.h"
#include "TMSun.h"
#include "TMSky.h"
#include "TMSnow.h"
#include "TMLog.h"
#include "TMObjectContainer.h"
#include "NewApp.h"
#include "Basedef.h"

void SwapLauncher()
{
	char szNewFileName[MAX_PATH] = { 0 };
	char szOldFileName[MAX_PATH] = { 0 };
	struct _stat64i32 buf0 {};
	struct _stat64i32 buf1 {};
	struct _stat64i32 buf2 {};
	if (_stat64i32("WYDLauncher.exe", &buf0))
	{
		g_bTestServer = 1;
		sprintf_s(szOldFileName, "TWYDLauncher.exe");
		sprintf_s(szNewFileName, "TMPTWYDLauncher.exe");
	}
	else
	{
		sprintf_s(szOldFileName, "WYDLauncher.exe");
		sprintf_s(szNewFileName, "TMPWYDLauncher.exe");
	}

	if (!_stat64i32(szNewFileName, &buf1))
	{
		if (!_stat64i32(szOldFileName, &buf2))
			SetFileAttributes(szOldFileName, FILE_ATTRIBUTE_NORMAL);

		if (CopyFileA(szNewFileName, szOldFileName, 0) == 1)
			DeleteFileA(szNewFileName);
	}
}

int IsCastle(int nServerIndex)
{
	int iweek = BASE_GetWeekNumber();
	return !(iweek % 7) && (int)((iweek / 7) & 2) != nServerIndex % 2;
}

TMLoginScene::TMLoginScene() 
	: TMScene()
{
	m_eSceneType = ESCENE_TYPE::ESCENE_LOGIN;
	m_nTextIndex = -1;
	m_dwStartCamTime = 0;
	m_dwLastClickLoginBtnTime = 0;
	m_cStartRun = 0;
	m_nDemoType = 0;
	m_bRemove = 0;

	if (Game_grade)
		GameGradeScene();

	if (g_pApp->m_pBGMManager != nullptr)
		g_pApp->m_pBGMManager->StopBGM();

	memset(m_HeightMapData, 0, sizeof(m_HeightMapData));

	if (g_pApp->m_pBGMManager != nullptr)
		g_pApp->m_pBGMManager->PlayMusic(0);

	DS_SOUND_MANAGER::m_nMusicIndex = 0;
	
	g_AccountLock = 0;
	g_nBattleMaster = 0;
	LastSendMsgTime = 0;
}

TMLoginScene::~TMLoginScene()
{
	for (int nPerson = 0; nPerson < 50; ++nPerson)
	{
		if (m_pCheckHumanList[nPerson] != nullptr)
		{
			g_pObjectManager->DeleteObject(m_pCheckHumanList[nPerson]);
			m_pCheckHumanList[nPerson] = nullptr;
		}
	}
	if (g_pSoundManager != nullptr)
	{
		if (g_pSoundManager->GetSoundData(102) != nullptr &&
			g_pSoundManager->GetSoundData(102)->IsSoundPlaying())
		{
			g_pSoundManager->GetSoundData(102)->Stop();
		}
	}
	Game_grade = 0;
}

int TMLoginScene::InitializeScene()
{
	LoadRC("UI\\SelServerScene2.txt");

	_SYSTEMTIME time{};
	GetLocalTime(&time);
	g_pCursor->SetVisible(0);
	g_pDevice->m_dwClearColor = 0;
	
	if (SControl* pLeftoverPicker = m_pControlContainer->FindControl(P_SERVER_SEL))
		pLeftoverPicker->SetVisible(0);

	m_pLoginPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(P_LOGIN_BOX));
	m_pLoginBtns[0] = static_cast<SButton*>(m_pControlContainer->FindControl(B_LOGIN_OK));
	m_pLoginBtns[2] = static_cast<SButton*>(m_pControlContainer->FindControl(B_QUIT));
	m_pLoginBtns[1] = static_cast<SButton*>(m_pControlContainer->FindControl(B_CREATE_ID));

	m_pLogoPanels[0] = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_LOGO_PANEL1));
	m_pLogoPanels[1] = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_LOGO_PANEL2));

	m_pLogoPanels[0]->SetVisible(Game_grade == 0);
	m_pLogoPanels[1]->SetVisible(Game_grade == 0);

	m_pLoginPanelText = static_cast<SText*>(m_pControlContainer->FindControl(T_LOGIN_BOX_TEXT));
	m_pLoginIDText = static_cast<SText*>(m_pControlContainer->FindControl(T_LOGIN_ID_TEXT));
	m_pLoginPasswordText = static_cast<SText*>(m_pControlContainer->FindControl(T_LOGIN_PASSWORD_TEXT));

	m_pLoginPanelText->m_nPosY += 5.0f;

	SText* pText = static_cast<SText*>(m_pControlContainer->FindControl(TMT_SCENE_TEXT));

	pText->SetPos(pText->m_nPosX, static_cast<float>(g_pDevice->m_dwScreenHeight) - 15.0f);
	
	float nHeight = static_cast<float>(g_pDevice->m_dwScreenHeight) * 0.5f - static_cast<float>(m_pLoginPanel->m_nHeight) * 0.5f;
	m_pLoginPanel->SetPos(static_cast<float>(g_pDevice->m_dwScreenWidth) * 0.5f - m_pLoginPanel->m_nWidth * 0.5f, nHeight);

	if(g_pDevice->m_dwScreenWidth == 1600)
	{
		float fWidthRatio = RenderDevice::m_fWidthRatio;
		float fHeightRatio = RenderDevice::m_fHeightRatio;

		m_pLogoPanels[0]->SetRealPos(400 * fWidthRatio, 10.0f * fHeightRatio);
		m_pLogoPanels[1]->SetRealPos(400 * fWidthRatio, 10.0f * fHeightRatio);
		m_pLogoPanels[0]->SetRealSize(256 * fWidthRatio, 256.0f * fHeightRatio);
		m_pLogoPanels[1]->SetRealSize(256 * fWidthRatio, 256.0f * fHeightRatio);
	}
	else
	{
		int nAddHeight = 0;
		if (g_pDevice->m_dwScreenWidth == 1024)
			nAddHeight = 20;
		else if (g_pDevice->m_dwScreenWidth == 12180)
			nAddHeight = 40;

		m_pLogoPanels[0]->SetPos(static_cast<float>(g_pDevice->m_dwScreenWidth) * 0.5f - m_pLogoPanels[0]->m_nWidth, 10.0f * RenderDevice::m_fHeightRatio + static_cast<float>(nAddHeight));
		m_pLogoPanels[1]->SetPos(static_cast<float>(g_pDevice->m_dwScreenWidth) * 0.5f, 10.0f * RenderDevice::m_fHeightRatio + static_cast<float>(nAddHeight));
	}

	m_pEditID = static_cast<SEditableText*>(m_pControlContainer->FindControl(E_LOGIN_ID));
	m_pEditPW = static_cast<SEditableText*>(m_pControlContainer->FindControl(E_LOGIN_PASSWORD));

	if (m_pEditID)
		m_pEditID->m_nMaxStringLen = MAX_ACCOUNT_LENGTH - 1;

	if (m_pEditPW)
		m_pEditPW->m_nMaxStringLen = MAX_ACCOUNT_LENGTH - 1;

	g_pObjectManager->m_cCharacterSlot = -1;
	g_pObjectManager->m_cSelectShortSkill = 0;

	memset(&g_pObjectManager->m_stSelCharData, 0, sizeof STRUCT_SELCHAR);
	memset(g_pObjectManager->m_stItemCargo, 0, sizeof STRUCT_ITEM * MAX_CARGO);
	memset(&g_pObjectManager->m_stMobData, 0, sizeof STRUCT_MOB);

	for (int i = 0; i < 4; ++i)
		g_pObjectManager->m_stMobData.ShortSkill[i] = -1;

	if (g_pSocketManager)
		g_pSocketManager->CloseSocket();

	if (g_pObjectManager->m_bBilling == 1)
	{
		g_pObjectManager->m_bBilling = 0;

		m_pMessageBox->SetMessage(g_pMessageStringTable[132], 0, 0);
		m_pMessageBox->SetVisible(1);
	}

	for (int i = 0; i < 3; ++i)
	{
		if (m_pLoginBtns[i])
			m_pLoginBtns[i]->SetVisible(1);
	}

	m_pLoginPanel->SetVisible(1);
	m_pLoginPanel->m_GCPanel.dwColor = 0xFFFFFFFF;
	m_pControlContainer->SetFocusedControl(m_pEditID);

	g_pDevice->m_nHeightShift = 0;
	g_pObjectManager->m_pCamera->SetViewMode(0);

	memset(m_HeightMapData, 0, sizeof m_HeightMapData);

	char szMapPath[128]{ 0 };
	char szDataPath[128]{ 0 };
	memset(szMapPath, 0, sizeof szMapPath);
	memset(szDataPath, 0, sizeof szDataPath);

	sprintf_s(szMapPath, "env\\Field1616.trn");
	sprintf_s(szDataPath, "env\\Field1616.dat");

	m_nDemoType = 1;
	if (m_nDemoType)
	{
		switch (m_nDemoType)
		{
		case 1:
			sprintf_s(szMapPath, "env\\Character.trn");
			sprintf_s(szDataPath, "env\\Character.dat");

			g_HeightPosX = 2048;
			g_HeightPosY = 2048;
			break;
		case 2:
		case 3:
			sprintf_s(szMapPath, "env\\Field0813.trn");
			sprintf_s(szDataPath, "env\\Field0813.dat");

			g_HeightPosX = 896;
			g_HeightPosY = 1536;
			break;
		case 4:
			sprintf_s(szMapPath, "env\\Field2922.trn");
			sprintf_s(szDataPath, "env\\Field2922.dat");

			g_HeightPosX = 3584;
			g_HeightPosY = 2684;
			break;
		}
	}
	else
	{
		sprintf_s(szMapPath, "env\\Field2723.trn");
		sprintf_s(szDataPath, "env\\Field2723.dat");

		g_HeightPosX = 3328;
		g_HeightPosY = 2816;
	}

	m_pGroundList[0] = new TMGround();

	if (!m_pGroundList[0]->LoadTileMap(szMapPath))
	{
		if (m_bCriticalError)
			LogMsgCriticalError(14, 0, 0, 0, 0);

		m_bCriticalError = 1;
	}

	m_pGround = m_pGroundList[0];

	for (int nY = 0; nY < 128; ++nY)
		memcpy(m_HeightMapData[nY], m_pGround->m_pMaskData[nY], 128);

	g_HeightPosX = static_cast<int>(m_pGround->m_vecOffset.x);
	g_HeightPosY = static_cast<int>(m_pGround->m_vecOffset.y);

	m_pObjectContainerList[0] = new TMObjectContainer(m_pGround);
	m_pGroundObjectContainer->AddChild(m_pObjectContainerList[0]);

	if (m_pObjectContainerList[0]->Load(szDataPath))
	{
		m_pGroundObjectContainer->AddChild(m_pGroundList[0]);

		if (m_nDemoType == 2)
			GroundNewAttach(EDirection::EDIR_RIGHT);

		m_pSun = new TMSun();
		m_pSun->InitObject();

		m_pEffectContainer->AddChild(m_pSun);

		m_pSky = new TMSky();
		AddChild(m_pSky);

		memset(&time, 0, sizeof time);
		GetLocalTime(&time);

		g_nSelServerWeather = time.wDay % 4;
		m_pSky->SetWeatherState(time.wDay % 4);

		++g_nSelServerWeather;
		g_nSelServerWeather %= 4;

		m_pSnow = new TMSnow(1.0f);

		if(m_pEffectContainer)
			m_pEffectContainer->AddChild(m_pSnow);

		m_pSnow->m_bVisible = g_nSelServerWeather == 2;

		memset(m_pCheckHumanList, 0, sizeof m_pCheckHumanList);

		g_pObjectManager->m_pCamera->m_vecCamPos = TMVector2(2112.55f, 2088.0f);
		g_pObjectManager->m_pCamera->m_fHorizonAngle = 4.71f;
		g_pObjectManager->m_pCamera->m_fVerticalAngle = 0.26f;

		if (g_pSoundManager)
			if (g_pSoundManager->GetSoundData(102))
				if (!g_pSoundManager->GetSoundData(102)->IsSoundPlaying())
					g_pSoundManager->GetSoundData(102)->Play(0, 1u);

		if (!Game_grade)
		{
			ResetDemoPlayer();
			CamAction();
		}

		if (g_pSoundManager)
			if (g_pSoundManager->GetSoundData(101))
				if (!g_pSoundManager->GetSoundData(101)->IsSoundPlaying())
					g_pSoundManager->GetSoundData(101)->Play(0);

		SwapLauncher();
		return 1;
	}
	else
	{
		LOG_WRITELOG("DataFile Not Found");
		MessageBoxA(g_pApp->m_hWnd, "DataFile NotFound.", "File Lost", 0);
		PostMessageA(g_pApp->m_hWnd, 0x10u, 0, 0);
	}

	return 0;
}

int TMLoginScene::OnControlEvent(unsigned int idwControlID, unsigned int idwEvent)
{
	switch (idwControlID)
	{
	case TMM_MESSAGE_BOX:
		if (idwEvent)
			m_pMessageBox->SetVisible(0);
		else if (m_pMessageBox->m_dwMessage == B_QUIT)
			PostMessage(g_pApp->m_hWnd, 0x10u, 0, 0);
		else if (m_pMessageBox->m_dwMessage == B_CREATE_ID)
			ShellExecute(0, 0, g_pMessageStringTable[263], 0, 0, 3);
		break;

		case B_LOGIN_OK:
		{
			unsigned int LiveTime = g_pTimerManager->GetServerTime();
			if (LastSendMsgTime + 1500 > LiveTime)
				return 1;

			auto pLoginOK = m_pLoginBtns[0];
			auto pEditID = m_pEditID;
			auto pEditPassword = m_pEditPW;

			const char* pszEmail = pEditID->GetText();
			const size_t nEmailLength = strlen(pszEmail);
			const char* pszAt = strchr(pszEmail, '@');

			// pszAt[1] is checked before reading pszAt + 2, so an address that
			// ends in '@' cannot walk past the terminator.
			if (nEmailLength < 6
				|| !pszAt
				|| pszAt == pszEmail
				|| !pszAt[1]
				|| !strchr(pszAt + 2, '.'))
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[3], 4000);
				m_pMessagePanel->SetVisible(1, 1);

				return 1;
			}

			if (nEmailLength >= MAX_ACCOUNT_LENGTH)
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[4], 4000);
				m_pMessagePanel->SetVisible(1, 1);

				return 1;
			}

			const size_t nPasswordLength = strlen(pEditPassword->GetText());

			if (nPasswordLength < 4)
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[5], 4000);
				m_pMessagePanel->SetVisible(1, 1);

				return 1;
			}

			if (nPasswordLength >= MAX_ACCOUNT_LENGTH)
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[4], 4000);
				m_pMessagePanel->SetVisible(1, 1);

				return 1;
			}

			pLoginOK->SetEnable(0);
			pEditPassword->SetEnable(0);
			m_dwLastClickLoginBtnTime = g_pTimerManager->GetServerTime();

			m_pMessagePanel->SetMessage(g_pMessageStringTable[7], 4000);
			m_pMessagePanel->SetVisible(1, 1);

			if (!g_pSocketManager->ConnectServer(W2_SERVER_HOST, TM_CONNECTION_PORT, 0, 1124))
			{
				pLoginOK->SetEnable(1);

				m_pMessagePanel->SetMessage(g_pMessageStringTable[8], 4000);
				m_pMessagePanel->SetVisible(1, 1);
				return 1;
			}

			g_bMoveServer = 0;

			MSG_AccountLogin stAccountLogin{};
			stAccountLogin.Header.ID = 0;
			stAccountLogin.Header.Type = MSG_AccountLogin_Opcode;
			stAccountLogin.Force = 1;
			stAccountLogin.Version = TM_PROTOCOL_VERSION;
			
			DWORD dwSize = 0;
			IP_ADAPTER_INFO stInfo{};
			GetAdaptersInfo(&stInfo, &dwSize);

			if (dwSize)
			{
				IP_ADAPTER_INFO* pInfo = nullptr;
				pInfo = (IP_ADAPTER_INFO*)malloc(dwSize);

				GetAdaptersInfo(pInfo, &dwSize);
				char* sour = pInfo->AdapterName;
				int tpos = 0;
				int grid = 0;
				char temp[256] = { 0 };
				int len = strlen(pInfo->AdapterName); // v18;
				for (int i = 0; i < len; ++i)
				{
					if (sour[i] != 123 && sour[i] != 125 && sour[i] != 45)
					{
						temp[tpos++] = sour[i];

						if (!(++grid % 8))
							temp[tpos++] = 32;
					}
				}

				temp[tpos] = '\0';

				sscanf_s(temp, "%x %x %x %x", &stAccountLogin.Mac[0], &stAccountLogin.Mac[1], &stAccountLogin.Mac[2], &stAccountLogin.Mac[3]);

				free(pInfo);
			}

			sprintf_s(stAccountLogin.AccountName, "%s", pszEmail);
			sprintf_s(stAccountLogin.AccountPass, "%s", pEditPassword->GetText());

			g_pSocketManager->SendOneMessage(reinterpret_cast<char*>(&stAccountLogin), sizeof MSG_AccountLogin);
			LastSendMsgTime = g_pTimerManager->GetServerTime();
		}
		break;
		case B_CREATE_ID:
			m_pMessageBox->SetMessage(g_pMessageStringTable[9], B_CREATE_ID, g_pMessageStringTable[10]);
			m_pMessageBox->SetVisible(1);
			break;
		case B_QUIT:
			m_pMessageBox->SetMessage(g_pMessageStringTable[11], B_QUIT, nullptr);
			m_pMessageBox->SetVisible(1);
			break;
	}

	return 1;
}

int TMLoginScene::OnCharEvent(char iCharCode, int lParam)
{
	auto pEditID = m_pEditID;
	auto pEditPassword = m_pEditPW;
	
	switch (iCharCode)
	{
	case VK_TAB:
		if (pEditID->IsFocused())
			m_pControlContainer->SetFocusedControl(pEditPassword);
		else if(pEditPassword->IsFocused())
			m_pControlContainer->SetFocusedControl(pEditID);

		break;
	case VK_RETURN:
		if (pEditPassword->IsFocused() && m_pLoginBtns[0]->m_bEnable == 1)
			OnControlEvent(65873u, 0);
		break;
	case VK_ESCAPE:
		m_pMessageBox->SetMessage(g_pMessageStringTable[2], 65874u, nullptr);
		m_pMessageBox->SetVisible(1);
		break;
	}

	return TMScene::OnCharEvent(iCharCode, lParam);
}

int TMLoginScene::OnPacketEvent(unsigned int dwCode, char* buf)
{
	if (!buf)
		return 0;

	auto packet = reinterpret_cast<MSG_STANDARD*>(buf);
	if (TMScene::OnPacketEvent(dwCode, buf) == 1)
	{
		if (packet->Type == 0x101)
			m_pLoginPanel->SetVisible(1);

		if (!m_pLoginBtns[0]->m_bEnable)
			m_pLoginBtns[0]->SetEnable(1);

		if (!m_pEditPW->m_bEnable)
			m_pEditPW->SetEnable(1);

		return 1;		
	}
	if (packet->Type == MSG_CNFAccountLogin_Opcode)
	{
		m_pMessagePanel->SetVisible(0, 1);
		g_pTimerManager->SetServerTime(packet->Tick);

		auto selChar = reinterpret_cast<MSG_CNFAccountLogin*>(buf);
		memcpy(&g_pObjectManager->m_stSelCharData, &selChar->SelChar, sizeof STRUCT_SELCHAR);
		memcpy(g_pObjectManager->m_stItemCargo, selChar->Cargo, sizeof(STRUCT_ITEM) * MAX_CARGO);

		g_pObjectManager->m_nCargoCoin = selChar->Coin;
		memset(g_pObjectManager->m_stMemo, 0, sizeof g_pObjectManager->m_stMemo);

		for (int i = 0; i < 16; ++i)
			g_pSocketManager->SendQueue[i] = selChar->SecretCode[i];

		g_pSocketManager->SendCount = 0;
		g_pSocketManager->RecvCount = 0;

		g_pObjectManager->SetCurrentState(ObjectManager::TM_GAME_STATE::TM_SELECTCHAR_STATE);
		return 1;		
	}
	if (packet->Type == 0x11D || packet->Type == 0x11C)
	{
		if (packet->Type == 0x101)
			m_pLoginPanel->SetVisible(1);

		m_pMessagePanel->SetMessage(g_pMessageStringTable[12], 4000);
		m_pMessagePanel->SetVisible(1, 1);
		m_pLoginBtns[0]->SetEnable(1);
		m_pLoginPanel->SetVisible(1);
		return 1;
	}
	else
	{
		if (packet->Type == 0xADA)
			g_pObjectManager->m_bPlayTime = *(DWORD*)&buf[12];

		return 0;
	}

	return 0;
}

int TMLoginScene::FrameMove(unsigned int dwServerTime)
{
	TMScene::FrameMove(dwServerTime);

	dwServerTime = g_pTimerManager->GetServerTime();

	if (g_pDevice->m_bLoadMeshManager == 1)
		g_pCursor->SetVisible(1);

	if (Game_grade && !FrameMoveGameGrade(dwServerTime))
		return 0;


	for (int nPerson = 0; nPerson < 50; ++nPerson)
	{
		if (m_pCheckHumanList[nPerson])
		{
			m_pCheckHumanList[nPerson]->m_fWantHeight = -4.0f;
		}
	}	

	if ((int)(dwServerTime - m_dwStartCamTime) > 5000 && !m_cStartRun)
	{
		if (!m_nDemoType)
		{
			MoveHuman(0);

			m_cStartRun = 1;
		}
		else if (m_nDemoType == 1)
		{
			for (int i = 0; i < 50; ++i)
				MoveHuman(i);

			if (g_pSoundManager)
			{
				auto sound = g_pSoundManager->GetSoundData(15);
				if (sound && !sound->IsSoundPlaying())
					sound->Play();
			}

			m_cStartRun = 1;
		}
	}
	if ((int)(dwServerTime - m_dwStartCamTime) > 7000 && m_cStartRun == 1)
	{
		if (!m_nDemoType)
		{
			MoveHuman(1);
			m_cStartRun = 2;
		}
		else if (m_nDemoType == 1)
		{
			if (g_pSoundManager)
			{
				auto sound = g_pSoundManager->GetSoundData(16);
				if (sound && !sound->IsSoundPlaying())
					sound->Play();
			}

			m_cStartRun = 2;
		}
	}

	if (m_nDemoType == 1 && (int)(dwServerTime - m_dwStartCamTime) > 14000 && !m_bRemove)
	{
		RemoveHuman();
		m_bRemove = 1;
	}

	if (m_nDemoType == 3 && (int)(dwServerTime - m_dwStartCamTime) > 12000 && !m_cStartRun)
	{
		if (m_pCheckHumanList[0])
			m_pCheckHumanList[0]->SetAnimation(ECHAR_MOTION::ECMOTION_LEVELUP, 0);

		m_cStartRun = 1;
	}
	if (m_nDemoType == 3 && (int)(dwServerTime - m_dwStartCamTime) > 20000 && m_cStartRun == 1)
	{
		if (m_pCheckHumanList[0])
			m_pCheckHumanList[0]->SetAnimation(ECHAR_MOTION::ECMOTION_LEVELUP, 0);

		m_cStartRun = 2;
	}

	if ((int)(dwServerTime - m_dwStartCamTime) > 6000)
	{
		auto pEditPassword = m_pEditPW;
		m_pLoginBtns[0]->SetEnable(1);
		pEditPassword->SetEnable(1);
	}

	return 1;
}

void TMLoginScene::ResetDemoPlayer()
{
	for (int nPerson = 0; nPerson < 50; ++nPerson)
	{
		if (m_pCheckHumanList[nPerson])
		{
			g_pObjectManager->DeleteObject(m_pCheckHumanList[nPerson]);
			m_pCheckHumanList[nPerson] = nullptr;
		}
	}

	memset(&m_stDemoHuman, 0, sizeof(m_stDemoHuman));

	FILE* fp = nullptr;
	if (m_nDemoType)
	{
		switch (m_nDemoType)
		{
		case 1:
			fopen_s(&fp, "UI\\demo2.bin", "rb");
			break;
		case 2:
			fopen_s(&fp, "UI\\demo3.bin", "rb");
			break;
		case 3:
			fopen_s(&fp, "UI\\demo4.bin", "rb");
			break;
		}
	}
	else
	{
		fopen_s(&fp, "UI\\demo.bin", "rb");
	}

	if (fp != nullptr)
	{
		for (int i = 0; i < 50; ++i)
		{
			m_stDemoHuman[i].Mantua = 0;
			int ret = fread(&m_stDemoHuman[i], 1, sizeof(m_stDemoHuman[i]), fp);
			if (!ret)
				break;

			HUMAN_LOOKINFO stHumanLook{};
			SANC_INFO stSancInfo{};

			short sFace = m_stDemoHuman[i].nFace;
			stHumanLook.FaceMesh = g_pItemList[m_stDemoHuman[i].nFace].nIndexMesh;
			stHumanLook.FaceSkin = g_pItemList[m_stDemoHuman[i].nFace].nIndexTexture;
			stHumanLook.HelmMesh = g_pItemList[m_stDemoHuman[i].Helm].nIndexMesh;
			stHumanLook.HelmSkin = g_pItemList[m_stDemoHuman[i].Helm].nIndexTexture;
			stHumanLook.CoatMesh = g_pItemList[m_stDemoHuman[i].Body].nIndexMesh;
			stHumanLook.CoatSkin = g_pItemList[m_stDemoHuman[i].Body].nIndexTexture;
			stHumanLook.PantsMesh = g_pItemList[m_stDemoHuman[i].Body].nIndexMesh;
			stHumanLook.PantsSkin = g_pItemList[m_stDemoHuman[i].Body].nIndexTexture;
			stHumanLook.GlovesMesh = g_pItemList[m_stDemoHuman[i].Body].nIndexMesh;
			stHumanLook.GlovesSkin = g_pItemList[m_stDemoHuman[i].Body].nIndexTexture;
			stHumanLook.BootsMesh = g_pItemList[m_stDemoHuman[i].Body].nIndexMesh;
			stHumanLook.BootsSkin = g_pItemList[m_stDemoHuman[i].Body].nIndexTexture;
			stHumanLook.RightMesh = g_pItemList[m_stDemoHuman[i].Right].nIndexMesh;
			stHumanLook.RightSkin = g_pItemList[m_stDemoHuman[i].Right].nIndexTexture;
			stHumanLook.LeftMesh = g_pItemList[m_stDemoHuman[i].Left].nIndexMesh;
			stHumanLook.LeftSkin = g_pItemList[m_stDemoHuman[i].Left].nIndexTexture;
			stSancInfo.Sanc7 = m_stDemoHuman[i].nSanc;
			stSancInfo.Sanc6 = stSancInfo.Sanc7;
			stSancInfo.Sanc5 = stSancInfo.Sanc7;
			stSancInfo.Sanc4 = stSancInfo.Sanc7;
			stSancInfo.Sanc3 = stSancInfo.Sanc7;
			stSancInfo.Sanc2 = stSancInfo.Sanc7;
			stSancInfo.Legend7 = static_cast<char>(g_pItemList[m_stDemoHuman[i].Body].nGrade);
			stSancInfo.Legend6 = stSancInfo.Legend7;
			stSancInfo.Legend5 = stSancInfo.Legend7;
			stSancInfo.Legend4 = stSancInfo.Legend7;
			stSancInfo.Legend3 = stSancInfo.Legend7;
			stSancInfo.Legend2 = stSancInfo.Legend7;

			if (m_stDemoHuman[i].Body == m_stDemoHuman[i].Helm)
			{
				stSancInfo.Sanc1 = stSancInfo.Sanc2;
				stSancInfo.Legend1 = stSancInfo.Legend2;
			}

			if (ret == -1)
				break;

			m_pCheckHumanList[i] = new TMHuman(this);
			m_pCheckHumanList[i]->m_stScore.Hp = 1;
			m_pCheckHumanList[i]->m_dwID = 10000;

			sprintf(m_pCheckHumanList[i]->m_szName, "person%d", i);

			if (m_stDemoHuman[i].nMount >= 2360 && m_stDemoHuman[i].nMount < 2390
			 || m_stDemoHuman[i].nMount >= 2960 && m_stDemoHuman[i].nMount < 3000)
			{
				m_pCheckHumanList[i]->m_cMount = 1;
				int sIndex = m_stDemoHuman[i].nMount - 2045;

				STRUCT_ITEM item{};
				item.sIndex = sIndex;
				m_pCheckHumanList[i]->m_sMountIndex = sIndex - 315;
				int nClass = BASE_GetItemAbility(&item, 18);
				m_pCheckHumanList[i]->m_nMountSkinMeshType = BASE_DefineSkinMeshType(nClass);
				m_pCheckHumanList[i]->m_stMountLook.Mesh0 = g_pItemList[sIndex].nIndexMesh;
				m_pCheckHumanList[i]->m_stMountLook.Mesh1 = m_pCheckHumanList[i]->m_stMountLook.Mesh0;
				m_pCheckHumanList[i]->m_stMountLook.Skin0 = g_pItemList[sIndex].nIndexTexture;
				m_pCheckHumanList[i]->m_stMountLook.Skin1 = m_pCheckHumanList[i]->m_stMountLook.Skin0;

				if (sIndex >= 321 && sIndex <= 325)
				{
					m_pCheckHumanList[i]->m_stMountLook.Mesh2 = sIndex - 320;
					m_pCheckHumanList[i]->m_stMountSanc.Sanc0 = 0;
					m_pCheckHumanList[i]->m_stMountSanc.Sanc1 = 0;
					m_pCheckHumanList[i]->m_stMountSanc.Sanc2 = 0;
				}
				else if (sIndex >= 326 && sIndex <= 330)
				{
					m_pCheckHumanList[i]->m_stMountLook.Mesh2 = sIndex - 325;
					m_pCheckHumanList[i]->m_stMountSanc.Sanc0 = 0;
					m_pCheckHumanList[i]->m_stMountSanc.Sanc1 = 0;
					m_pCheckHumanList[i]->m_stMountSanc.Sanc2 = 0;
				}
				else
				{
					m_pCheckHumanList[i]->m_stMountLook.Mesh2 = 0;
					m_pCheckHumanList[i]->m_stMountSanc.Sanc2 = 0;
					m_pCheckHumanList[i]->m_stMountSanc.Sanc0 = 0;
					m_pCheckHumanList[i]->m_stMountSanc.Sanc1 = 0;
				}

				m_pCheckHumanList[i]->m_fMountScale = BASE_GetMountScale(
					m_pCheckHumanList[i]->m_nMountSkinMeshType,
					m_pCheckHumanList[i]->m_stMountLook.Mesh0);

				if (sIndex == 333)
					m_pCheckHumanList[i]->m_fMountScale = 1.1f;
			}

			memcpy(&m_pCheckHumanList[i]->m_stLookInfo, &stHumanLook, sizeof(m_pCheckHumanList[i]->m_stLookInfo));
			memcpy(&m_pCheckHumanList[i]->m_stSancInfo, &stSancInfo, sizeof(m_pCheckHumanList[i]->m_stSancInfo));

			if (m_stDemoHuman[i].Mantua > 0)
			{
				m_pCheckHumanList[i]->m_cMantua = 1;
				m_pCheckHumanList[i]->m_wMantuaSkin = g_pItemList[m_stDemoHuman[i].Mantua].nIndexTexture;
				m_pCheckHumanList[i]->m_ucMantuaSanc = m_stDemoHuman[i].nSanc;
				m_pCheckHumanList[i]->m_ucMantuaLegend = static_cast<char>(g_pItemList[m_stDemoHuman[i].Mantua].nGrade);
			}

			m_vecMoveToPos[i].x = (int)m_stDemoHuman[i].fTX;
			m_vecMoveToPos[i].y = (int)m_stDemoHuman[i].fTY;
			m_pCheckHumanList[i]->m_cClone = 1;

			m_pCheckHumanList[i]->SetRace(sFace);
			m_pCheckHumanList[i]->InitObject();
			m_pCheckHumanList[i]->CheckWeapon(m_stDemoHuman[i].Left, m_stDemoHuman[i].Right);
			m_pCheckHumanList[i]->InitAngle(0.0f, ((float)m_stDemoHuman[i].nAngle * 6.2831855f) / 360.0f, 0.0f);
			m_pCheckHumanList[i]->InitPosition(m_stDemoHuman[i].fX, 0.0f, m_stDemoHuman[i].fY);

			m_pCheckHumanList[i]->m_fMaxSpeed = (float)m_stDemoHuman[i].nSpeed;
			m_pCheckHumanList[i]->m_bParty = 1;

			m_pHumanContainer->AddChild(m_pCheckHumanList[i]);
		}

		fclose(fp);
	}

	for (int nPerson = 0; nPerson < 50; ++nPerson)
	{
		if (m_pCheckHumanList[nPerson])
		{
			m_pCheckHumanList[nPerson]->m_bVisible = 1;
			m_pCheckHumanList[nPerson]->FrameMove(0);
			m_pCheckHumanList[nPerson]->Render();
		}
	}
}

void TMLoginScene::AniDemoPlayer()
{
	for (int nPerson = 0; nPerson < 50; ++nPerson)
	{
		if (m_pCheckHumanList[nPerson])
		{
			m_pCheckHumanList[nPerson]->SetAnimation((ECHAR_MOTION)m_stDemoHuman[nPerson].nHumanAni, 0);
		}			
	}
}

void TMLoginScene::CamAction()
{
	auto pCamera = g_pObjectManager->GetCamera();
	pCamera->m_bStandAlone = 1;

	m_dwStartCamTime = g_pTimerManager->GetServerTime();

	if (m_nDemoType)
	{
		switch (m_nDemoType)
		{
		case 1:
			ReadCameraPos((char*)"UI\\DemoCamAction2");
			break;
		case 2:
			ReadCameraPos((char*)"UI\\DemoCamAction3");
			break;
		case 3:
			ReadCameraPos((char*)"UI\\DemoCamAction4");
			break;
		case 4:
			ReadCameraPos((char*)"UI\\DemoCamAction5");
			break;
		}
	}
	else
		ReadCameraPos((char*)"UI\\DemoCamAction");

	m_sPlayDemo = 1;
}

void TMLoginScene::MoveHuman(int nIndex)
{
	if (nIndex < 0)
	{
		for (int nPerson = 0; nPerson < 50; ++nPerson)
		{
			if (m_pCheckHumanList[nPerson])
			{
				m_pCheckHumanList[nPerson]->GetRoute(m_vecMoveToPos[nPerson], 32, 0);
			}
		}
	}
	else if (m_pCheckHumanList[nIndex])
		m_pCheckHumanList[nIndex]->GetRoute(m_vecMoveToPos[nIndex], 32, 0);
}

void TMLoginScene::RemoveHuman()
{
	for (int nPerson = 0; nPerson < 18; ++nPerson)
	{
		if (m_pCheckHumanList[nPerson])
		{
			g_pObjectManager->DeleteObject(m_pCheckHumanList[nPerson]);
			m_pCheckHumanList[nPerson] = 0;
		}
	}
	
	for (int nPerson = 26; nPerson < 40; ++nPerson)
	{
		if (m_pCheckHumanList[nPerson])
		{
			g_pObjectManager->DeleteObject(m_pCheckHumanList[nPerson]);
			m_pCheckHumanList[nPerson] = 0;
		}
	}
}



int TMLoginScene::FrameMoveGameGrade(unsigned int dwServerTime)
{
	if (dwServerTime >= m_dGameGradePrintSaveTime + m_dGameGradePrintTime)
	{
		if (m_bGameGradePlay && (!m_pLoginPanel->IsVisible() || dwServerTime > m_dGameGradePrintSaveTime + m_dGameGradePrintTime))
		{
			ResetDemoPlayer();
			CamAction();

			m_bGameGradePlay = 0;
			m_pLoginPanel->SetVisible(1);
		}

		m_GameGrade->SetVisible(0);
		m_pLogoPanels[0]->SetVisible(1);
		m_pLogoPanels[1]->SetVisible(1);
		return 1;
	}

	m_pLogoPanels[0]->SetVisible(0);
	m_pLogoPanels[1]->SetVisible(0);
	m_pLoginPanel->SetVisible(0);
	m_GameGrade->SetVisible(1);
	return 0;
}

void TMLoginScene::GameGradeScene()
{
	float size = 1.0f;
	if (g_pApp->m_dwScreenWidth <= 1024)
	{
		switch (g_pApp->m_dwScreenWidth)
		{
		case 1024u:
			size = 0.79f;
			break;
		case 640u:
			size = 1.25f;
			break;
		case 0x320u:
			size = 1.0f;
			break;
		}
	}
	else if (g_pApp->m_dwScreenWidth == 1280)
		size = 0.63f;
	else if (g_pApp->m_dwScreenWidth == 1600)
		size = 0.5f;

	m_GameGrade = new SPanel(553, 0.0f, 0.0f, (float)g_pApp->m_dwScreenWidth * size, (float)g_pApp->m_dwScreenHeight * size, 0x77777777u, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
	m_GameGrade->SetControlID(4623u);
	m_GameGrade->SetPos(0, 0);
	
	if (m_pControlContainer)
		m_GameGrade->SetEventListener(m_pControlContainer);
	else
		m_GameGrade->SetEventListener(nullptr);

	m_pControlContainer->AddItem(m_GameGrade);

	m_GameGrade->SetVisible(1);
	m_dGameGradePrintTime = 100;
	m_bGameGradePlay = 1;
	m_dGameGradePrintSaveTime = g_pTimerManager->GetServerTime();
}
