#pragma once

#include <string>

bool IsProcessElevated();

bool RelaunchAsAdministrator();

std::wstring GetHostsFilePath();