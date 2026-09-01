#ifndef GAMESERVER_H
#define GAMESERVER_H

#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "ws2tcpip.h"

bool GameServerInit();
void GameServerCallbacks();

#endif