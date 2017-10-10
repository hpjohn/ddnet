/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_BALL_BALL_H
#define GAME_BALL_BALL_H

#include <math.h>

#include <base/math.h>
#include <base/system.h>

#include <base/tl/algorithm.h>
#include <base/tl/array.h>
#include <base/tl/sorted_array.h>
#include <base/tl/string.h>

#include <game/client/ui.h>
#include <game/mapitems.h>
#include <game/client/render.h>

#include <engine/shared/config.h>
#include <engine/shared/datafile.h>
#include <engine/ball.h>
#include <engine/graphics.h>
#include <engine/sound.h>

class CBall : public IBall
{
	class IInput *m_pInput;
	class IClient *m_pClient;
	class IConsole *m_pConsole;
	class IGraphics *m_pGraphics;
	class ITextRender *m_pTextRender;
	class ISound *m_pSound;
	class IStorage *m_pStorage;
	CRenderTools m_RenderTools;
	CUI m_UI;



public:
	class IInput *Input() { return m_pInput; };
	class IClient *Client() { return m_pClient; };
	class IConsole *Console() { return m_pConsole; };
	class IGraphics *Graphics() { return m_pGraphics; };
	class ISound *Sound() { return m_pSound; }
	class ITextRender *TextRender() { return m_pTextRender; };
	class IStorage *Storage() { return m_pStorage; };
	CUI *UI() { return &m_UI; }
	CRenderTools *RenderTools() { return &m_RenderTools; }


	CBall()
	{
		m_pInput = 0;
		m_pClient = 0;
		m_pGraphics = 0;
		m_pTextRender = 0;
		m_pSound = 0;

		m_WorldOffsetX = 0;
		m_WorldOffsetY = 0;
		m_BallOffsetX = 0.0f;
		m_BallOffsetY = 0.0f;

		m_WorldZoom = 1.0f;
		m_ZoomLevel = 200;
		m_LockMouse = false;
		m_MouseDeltaX = 0;
		m_MouseDeltaY = 0;
		m_MouseDeltaWx = 0;
		m_MouseDeltaWy = 0;
#if defined(__ANDROID__)
		m_OldMouseX = 0;
		m_OldMouseY = 0;
#endif


		ms_CheckerTexture = 0;
		ms_BackgroundTexture = 0;
		ms_CursorTexture = 0;
		ms_EntitiesTexture = 0;

	}



	static int ms_CheckerTexture;
	static int ms_BackgroundTexture;
	static int ms_CursorTexture;
	static int ms_EntitiesTexture;


	virtual void Init();
	virtual void UpdateAndRender();
	void Render();
	void RenderBackground(CUIRect View, int Texture, float Size, float Brightness);
	void ZoomMouseTarget(float ZoomFactor);

	bool m_LockMouse;
	float m_MouseDeltaX;
	float m_MouseDeltaY;
	float m_MouseDeltaWx;
	float m_MouseDeltaWy;
	int m_ZoomLevel;
	float m_BallOffsetX;
	float m_BallOffsetY;
	float m_WorldOffsetX;
	float m_WorldOffsetY;
	float m_WorldZoom;

};

#endif
