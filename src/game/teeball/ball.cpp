/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/tl/array.h>
#include <base/system.h>
#include <base/color.h>

#if defined(CONF_FAMILY_UNIX)
#include <pthread.h>
#endif

#include <engine/shared/datafile.h>
#include <engine/shared/config.h>
#include <engine/client.h>
#include <engine/console.h>
#include <engine/graphics.h>
#include <engine/input.h>
#include <engine/keys.h>
#include <engine/storage.h>
#include <engine/textrender.h>

#include <game/gamecore.h>
#include <game/localization.h>
#include <game/client/lineinput.h>
#include <game/client/render.h>
#include <game/client/ui.h>
#include <game/generated/client_data.h>

#include "ball.h"

int CBall::ms_CheckerTexture;
int CBall::ms_BackgroundTexture;
int CBall::ms_CursorTexture;
int CBall::ms_EntitiesTexture;

void CBall::Init()
{
	m_pInput = Kernel()->RequestInterface<IInput>();
	m_pClient = Kernel()->RequestInterface<IClient>();
	m_pConsole = Kernel()->RequestInterface<IConsole>();
	m_pGraphics = Kernel()->RequestInterface<IGraphics>();
	m_pTextRender = Kernel()->RequestInterface<ITextRender>();
	m_pStorage = Kernel()->RequestInterface<IStorage>();
	m_pSound = Kernel()->RequestInterface<ISound>();
	m_RenderTools.m_pGraphics = m_pGraphics;
	m_RenderTools.m_pUI = &m_UI;
	m_UI.SetGraphics(m_pGraphics, m_pTextRender);
	//m_Map.m_pBall = this;

	ms_CheckerTexture = Graphics()->LoadTexture("editor/checker.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, 0);
	ms_BackgroundTexture = Graphics()->LoadTexture("editor/background.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, 0);
	ms_CursorTexture = Graphics()->LoadTexture("editor/cursor.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, 0);
	ms_EntitiesTexture = Graphics()->LoadTexture("editor/entities.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, 0);

	//ms_FrontTexture = Graphics()->LoadTexture("ball/front.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, 0);
	//ms_TeleTexture = Graphics()->LoadTexture("ball/tele.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, 0);
	//ms_SpeedupTexture = Graphics()->LoadTexture("ball/speedup.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, 0);
	//ms_SwitchTexture = Graphics()->LoadTexture("ball/switch.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, 0);
	//ms_TuneTexture = Graphics()->LoadTexture("ball/tune.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, 0);

	//m_TilesetPicker.m_pBall = this;
	//m_TilesetPicker.MakePalette();
	//m_TilesetPicker.m_Readonly = true;

	//m_QuadsetPicker.m_pBall = this;
	//m_QuadsetPicker.NewQuad();
	//m_QuadsetPicker.m_Readonly = true;

	//m_Brush.m_pMap = &m_Map;

	//Reset();
	//m_Map.m_Modified = false;
	//m_Map.m_UndoModified = 0;
	//m_LastUndoUpdateTime = time_get();

	//ms_PickerColor = vec3(1.0f, 0.0f, 0.0f);
}


void CBall::UpdateAndRender()
{
	static float s_MouseX = 0.0f;
	static float s_MouseY = 0.0f;

	//if (m_Animate)
	//	m_AnimateTime = (time_get() - m_AnimateStart) / (float)time_freq();
	//else
	//	m_AnimateTime = 0;
	//ms_pUiGotContext = 0;

	// handle mouse movement
	float mx, my, Mwx, Mwy;
	float rx, ry;
	{
		Input()->MouseRelative(&rx, &ry);
#if defined(__ANDROID__)
		float tx, ty;
		tx = s_MouseX;
		ty = s_MouseY;

		s_MouseX = (rx / (float)Graphics()->ScreenWidth()) * UI()->Screen()->w;
		s_MouseY = (ry / (float)Graphics()->ScreenHeight()) * UI()->Screen()->h;

		s_MouseX = clamp(s_MouseX, 0.0f, UI()->Screen()->w);
		s_MouseY = clamp(s_MouseY, 0.0f, UI()->Screen()->h);

		m_MouseDeltaX = s_MouseX - m_OldMouseX;
		m_MouseDeltaY = s_MouseY - m_OldMouseY;
		m_OldMouseX = tx;
		m_OldMouseY = ty;
#else
		UI()->ConvertMouseMove(&rx, &ry);
		m_MouseDeltaX = rx;
		m_MouseDeltaY = ry;

		if (!m_LockMouse)
		{
			s_MouseX += rx;
			s_MouseY += ry;
		}

		s_MouseX = clamp(s_MouseX, 0.0f, UI()->Screen()->w);
		s_MouseY = clamp(s_MouseY, 0.0f, UI()->Screen()->h);
#endif

		// update the ui
		mx = s_MouseX;
		my = s_MouseY;
		Mwx = 0;
		Mwy = 0;

		int Buttons = 0;
		if (Input()->KeyIsPressed(KEY_MOUSE_1)) Buttons |= 1;
		if (Input()->KeyIsPressed(KEY_MOUSE_2)) Buttons |= 2;
		if (Input()->KeyIsPressed(KEY_MOUSE_3)) Buttons |= 4;

#if defined(__ANDROID__)
		static int ButtonsOneFrameDelay = 0; // For Android touch input

		UI()->Update(mx, my, Mwx, Mwy, ButtonsOneFrameDelay);
		ButtonsOneFrameDelay = Buttons;
#else
		UI()->Update(mx, my, Mwx, Mwy, Buttons);
#endif
	}

	// toggle gui

	Render();

	Input()->Clear();
}

void CBall::Render()
{
	// basic start
	Graphics()->Clear(1.0f, 0.0f, 1.0f);
	CUIRect View = *UI()->Screen();
	Graphics()->MapScreen(UI()->Screen()->x, UI()->Screen()->y, UI()->Screen()->w, UI()->Screen()->h);

	float Width = View.w;
	float Height = View.h;


	// render checker
	RenderBackground(View, ms_CheckerTexture, 32.0f, 1.0f);

	CUIRect MenuBar, ToolBox;

	View.HSplitTop(16.0f, &MenuBar, &View);
	View.VSplitLeft(100.0f, &ToolBox, &View);

	// do zooming
	if (Input()->KeyPress(KEY_KP_MINUS))
		m_ZoomLevel += 50;
	if (Input()->KeyPress(KEY_KP_PLUS))
		m_ZoomLevel -= 50;
	if (Input()->KeyPress(KEY_KP_MULTIPLY))
	{
		m_BallOffsetX = 0;
		m_BallOffsetY = 0;
		m_ZoomLevel = 100;
	}
	// Determines in which direction to zoom.
	int Zoom = 0;
	if (Input()->KeyPress(KEY_MOUSE_WHEEL_UP))
		Zoom--;
	if (Input()->KeyPress(KEY_MOUSE_WHEEL_DOWN))
		Zoom++;

	if (Zoom != 0)
	{
		float OldLevel = m_ZoomLevel;
		m_ZoomLevel = clamp(m_ZoomLevel + Zoom * 20, 50, 2000);
		if (g_Config.m_EdZoomTarget)
			ZoomMouseTarget((float)m_ZoomLevel / OldLevel);
	}

	m_ZoomLevel = clamp(m_ZoomLevel, 50, 2000);
	m_WorldZoom = m_ZoomLevel / 100.0f;
	float Brightness = 0.25f;

	RenderBackground(MenuBar, ms_BackgroundTexture, 128.0f, Brightness * 0);
	MenuBar.Margin(2.0f, &MenuBar);

	RenderBackground(ToolBox, ms_BackgroundTexture, 128.0f, Brightness);
	ToolBox.Margin(2.0f, &ToolBox);

	Graphics()->MapScreen(UI()->Screen()->x, UI()->Screen()->y, UI()->Screen()->w, UI()->Screen()->h);


	//if (m_Dialog == DIALOG_FILE)
	//{
	//	static int s_NullUiTarget = 0;
	//	UI()->SetHotItem(&s_NullUiTarget);
	//	RenderFileDialog();
	//}

	//if (m_PopupEventActivated)
	//{
	//	static int s_PopupID = 0;
	//	UiInvokePopupMenu(&s_PopupID, 0, Width / 2.0f - 200.0f, Height / 2.0f - 100.0f, 400.0f, 200.0f, PopupEvent);
	//	m_PopupEventActivated = false;
	//	m_PopupEventWasActivated = true;
	//}


	//UiDoPopupMenu();

	//if (m_GuiActive)
	//	RenderStatusbar(StatusBar);

	//
	if (g_Config.m_EdShowkeys)
	{
		Graphics()->MapScreen(UI()->Screen()->x, UI()->Screen()->y, UI()->Screen()->w, UI()->Screen()->h);
		CTextCursor Cursor;
		TextRender()->SetCursor(&Cursor, View.x + 10, View.y + View.h - 24 - 10, 24.0f, TEXTFLAG_RENDER);

		int NKeys = 0;
		for (int i = 0; i < KEY_LAST; i++)
		{
			if (Input()->KeyIsPressed(i))
			{
				if (NKeys)
					TextRender()->TextEx(&Cursor, " + ", -1);
				TextRender()->TextEx(&Cursor, Input()->KeyName(i), -1);
				NKeys++;
			}
		}
	}

	// render butt ugly mouse cursor
	float mx = UI()->MouseX();
	float my = UI()->MouseY();
	Graphics()->TextureSet(ms_CursorTexture);
	Graphics()->QuadsBegin();
	IGraphics::CQuadItem QuadItem(mx, my, 16.0f, 16.0f);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();
}


void CBall::RenderBackground(CUIRect View, int Texture, float Size, float Brightness)
{
	Graphics()->TextureSet(Texture);
	Graphics()->BlendNormal();
	Graphics()->QuadsBegin();
	Graphics()->SetColor(Brightness, Brightness, Brightness, 1.0f);
	Graphics()->QuadsSetSubset(0, 0, View.w / Size, View.h / Size);
	IGraphics::CQuadItem QuadItem(View.x, View.y, View.w, View.h);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();
}

void CBall::ZoomMouseTarget(float ZoomFactor)
{
	// zoom to the current mouse position
	// get absolute mouse position
	float aPoints[4];
	RenderTools()->MapscreenToWorld(
		m_WorldOffsetX, m_WorldOffsetY,
		1.0f, 1.0f, 0.0f, 0.0f, Graphics()->ScreenAspect(), m_WorldZoom, aPoints);

	float WorldWidth = aPoints[2] - aPoints[0];
	float WorldHeight = aPoints[3] - aPoints[1];

	float Mwx = aPoints[0] + WorldWidth * (UI()->MouseX() / UI()->Screen()->w);
	float Mwy = aPoints[1] + WorldHeight * (UI()->MouseY() / UI()->Screen()->h);

	// adjust camera
	m_WorldOffsetX += (Mwx - m_WorldOffsetX) * (1 - ZoomFactor);
	m_WorldOffsetY += (Mwy - m_WorldOffsetY) * (1 - ZoomFactor);
}


IBall *CreateBall() { return new CBall; }
