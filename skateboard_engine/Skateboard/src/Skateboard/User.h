#pragma once
#include "sktbdpch.h"
#include "Skateboard/Events/AppEvents.h"

typedef int32_t UserID;

namespace Skateboard
{
	// <summary>
	//	user represents users of the system, users will have the authority over the individual devices like gamepads connected to the system.
	//  on windows the keyboard belongs to the system, wheres the gamepads would ideally belong to individual players,
	//  on playstation audio output to main speakers would belong to the system wheres every personal device is associated with individual users
	// </summary>
	enum class UserType : uint32_t 
	{
		SKTB_USER_INVALID		= 0,
		SKTB_USER_LOGIN_PLAYER	= BIT(1),
		SKTB_USER_SYSTEM		= BIT(2),
		SKTB_USER_CUSTOM		= BIT(3),

		//SKTB_USER_REMOTE_PLAYER, future maybe
	};

	// <summary>
	// user is represented by a user id and type
	// </summary>
	struct User
	{
		UserType type; // for now only store the type
		int32_t id; // unique identifier

		//friend bool operator==(const User& a, const User& b)
		//{
		//	return a.type == b.type && a.userId == b.userId;
		//}
		//
		//friend bool operator<(const User& a, const User& b)
		//{
		//	return a.type < b.type && a.userId < b.userId;
		//}
	};

	// <summary>
	// Platform agnostic interface for managing users
	// </summary>

	class UserManager // abstract, see platform specific implementations for details
	{
		DISABLE_COPY_AND_MOVE(UserManager);

		friend class Input;
		friend class Application;
		friend class Platform;

	protected:
		virtual void Update() = 0;
		virtual void OnEvent(Event& e) = 0;

		std::unordered_map<UserID,User> Users; // map stores user Id as an identifier and User which at the moment
		User DefaultUser;

		void AddUser(const User& User);
		void RemoveUser(const User& User);

	public:
		UserManager() {};
		virtual int Init() = 0;
		virtual ~UserManager() {};

		virtual User GetUser(int32_t number) const = 0;
		std::unordered_map<int32_t, User> GetUsers() { return Users; }
		User GetDefaultUser() { return DefaultUser; }
		std::vector<User> GetUsersByType(const UserType& type);
		
	};
}	

