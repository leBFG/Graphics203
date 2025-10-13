#include "sktbdpch.h"
#include "User.h"
#include "Platform.h"

namespace Skateboard {

	void UserManager::AddUser(const User& User)
	{
		if (!Users.count(User.id))
		{
			Users[User.id] = User;
			AppLoginEvent login(User);
			Platform::GetPlatform().PlatformDispatchEvent(login);
		}
		
	}

	void UserManager::RemoveUser(const User& User)
	{
		AppLogOutEvent logout(User);
		Platform::GetPlatform().PlatformDispatchEvent(logout);
		Users.erase(User.id);
		if(User.id==DefaultUser.id)
			for (auto& u : Users) 
			{ 
				if (u.second.type == UserType::SKTB_USER_LOGIN_PLAYER) 
					DefaultUser = User; 
			}
	}
	std::vector<User> UserManager::GetUsersByType(const UserType& type)
	{
		std::vector<User> ret;
		for (auto& u : Users)
			if (type == u.second.type)
				ret.push_back(u.second);

		return ret;
	}

}