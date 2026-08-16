#pragma once

#include "TMScene.h"
#include "Structures.h"

class SPanel;
class SButton;
class SEditableText;
class TMRain;
class TMSnow;

class TMLoginScene : public TMScene
{
public:
	TMLoginScene();
	~TMLoginScene();

	int InitializeScene() override;
	int OnControlEvent(unsigned int idwControlID, unsigned int idwEvent) override;
	int OnCharEvent(char iCharCode, int lParam) override;
	int OnPacketEvent(unsigned int dwCode, char* buf) override;
	int FrameMove(unsigned int dwServerTime) override;

	void ResetDemoPlayer();
	void AniDemoPlayer();
	void CamAction();
	void MoveHuman(int nIndex);
	void RemoveHuman();

private:
	int FrameMoveGameGrade(unsigned int dwServerTime);
	void GameGradeScene();

public:
	SPanel* m_GameGrade;
	unsigned int m_dGameGradePrintTime;
	unsigned int m_dGameGradePrintSaveTime;
	int m_bGameGradePlay;
	unsigned int LastSendMsgTime;
	int m_nTextIndex;
	SPanel* m_pLogoPanels[2];
	SPanel* m_pLoginPanel;
	SText* m_pLoginPanelText;
	SText* m_pLoginIDText;
	SText* m_pLoginPasswordText;
	SButton* m_pLoginBtns[3];
	int m_bRemove;
	unsigned int m_dwStartTime;
	TMRain* m_pRain;
	TMSnow* m_pSnow;
	char m_cStartRun;
	TMHuman* m_pCheckHumanList[50];
	stDemoHuman m_stDemoHuman[50];
	IVector2 m_vecMoveToPos[50];
	SEditableText* m_pEditID;
	SEditableText* m_pEditPW;
	unsigned int m_dwLastClickLoginBtnTime;
	int m_nDemoType;
};