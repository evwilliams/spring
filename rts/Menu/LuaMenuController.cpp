/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#include "LuaMenuController.h"

#include "Game/UI/InfoConsole.h"
#include "Game/UI/MouseHandler.h"
#include "Lua/LuaInputReceiver.h"
#include "Lua/LuaMenu.h"
#include "System/Config/ConfigHandler.h"
#include "System/EventHandler.h"
#include "System/FileSystem/VFSHandler.h"
#include "System/Log/ILog.h"

CONFIG(std::string, DefaultLuaMenu).defaultValue("").description("Sets the default menu to be used when spring is started.");

CLuaMenuController* luaMenuController = nullptr;


CLuaMenuController::CLuaMenuController(const std::string& menuName)
 : menuArchive(menuName),
   lastDrawFrameTime(spring_gettime())
{
	if (menuArchive.empty())
		menuArchive = configHandler->GetString("DefaultLuaMenu");

	// create LuaMenu if necessary
	if (!menuArchive.empty()) {
		Reset();
		CLuaMenu::LoadFreeHandler();
	}
}


void CLuaMenuController::Reset()
{
	if (!Valid())
		return;

	LOG("[%s] using menu: %s", __FUNCTION__, menuArchive.c_str());
	vfsHandler->AddArchiveWithDeps(menuArchive, false);

	if (mouse == nullptr) {
		mouse = new CMouseHandler();
	} else {
		mouse->ReloadCursors();
	}

	if (infoConsole == nullptr)
		infoConsole = new CInfoConsole();
}

void CLuaMenuController::Activate()
{
	assert(luaMenuController != nullptr && Valid());
	activeController = luaMenuController;
	mouse->ShowMouse();

	luaMenu->ActivateMenu();
}


CLuaMenuController::~CLuaMenuController()
{
	SafeDelete(mouse);
	SafeDelete(infoConsole);
}

void CLuaMenuController::ResizeEvent()
{
	eventHandler.ViewResize();
}




bool CLuaMenuController::Draw()
{
	eventHandler.CollectGarbage();
	infoConsole->PushNewLinesToEventHandler();
	mouse->Update();
	mouse->UpdateCursors();
	eventHandler.Update();
	// calls IsAbove
	mouse->GetCurrentTooltip();

	// render if global rendering active + luamenu allows it, and at least once per 30s
	const bool shouldDraw = (globalRendering->active && luaMenu->AllowDraw()) || ((spring_gettime() - lastDrawFrameTime).toSecsi() > 30);
	if (shouldDraw) {
		ClearScreen();
		eventHandler.DrawGenesis();
		eventHandler.DrawScreen();
		mouse->DrawCursor();
		lastDrawFrameTime = spring_gettime();
	} else {
		spring_msecs(10).sleep(true); // no draw needed, sleep a bit
	}
	
	return shouldDraw;
}


int CLuaMenuController::KeyReleased(int k)
{
	luaInputReceiver->KeyReleased(k);
	return 0;
}


int CLuaMenuController::KeyPressed(int k, bool isRepeat)
{
	luaInputReceiver->KeyPressed(k, isRepeat);
	return 0;
}


int CLuaMenuController::TextInput(const std::string& utf8Text)
{
	eventHandler.TextInput(utf8Text);
	return 0;
}

