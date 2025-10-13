#include "sktbdpch.h"

#include "WindowsUserManager.h"

int Skateboard::WindowsUserManager::Init()
{
	AddUser({ UserType::SKTB_USER_SYSTEM, 0});

	// TODO add support for multiple devices here to distinguish local and remote players for example
	return 0;
}
