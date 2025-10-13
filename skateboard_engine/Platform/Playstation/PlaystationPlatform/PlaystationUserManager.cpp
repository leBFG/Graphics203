
#include "PlaystationUserManager.h"

namespace Skateboard
{
	PlaystationUserManager::~PlaystationUserManager()
	{
		sceUserServiceTerminate();
	}

	int PlaystationUserManager::Init()
	{
		// Initialise the user service library
		SceUserServiceInitializeParams initParams;
		initParams.priority = SCE_KERNEL_PRIO_FIFO_DEFAULT;

		// Deploys a thread within the library, priority of this thread has been set to default.
		int32_t err = sceUserServiceInitialize(&initParams);

		// Check there were no errors generated.
		if (err == SCE_USER_SERVICE_ERROR_ALREADY_INITIALIZED) {
			printf("Sce user service has already been initialised.");
			return -1;
		}
		else if (err == SCE_USER_SERVICE_ERROR_NO_MEMORY) {
			printf("Could not allocate memory!");
			return -1;
		}
		else if (err == SCE_USER_SERVICE_ERROR_INVALID_ARGUMENT) {
			printf("Provided an invalid argument!");
			return -1;
		}
		//...no errors? proceed to fetch information regarding the user.
		else if (err == SCE_OK) {
			printf("Initialised user service library!");

			User system;
			system.type = UserType::SKTB_USER_SYSTEM;

			AddUser({ UserType::SKTB_USER_SYSTEM, SCE_USER_SERVICE_USER_ID_SYSTEM });

			// Fetch all logged in users.
			SceUserServiceLoginUserIdList m_UserList;
			err = sceUserServiceGetLoginUserIdList(&m_UserList);

			if (err == SCE_OK) {
				printf("Fetched logged in users.");

				for (int32_t i = 0; i < SCE_USER_SERVICE_MAX_LOGIN_USERS; ++i)
				{
					if (m_UserList.userId[i] != SCE_USER_SERVICE_USER_ID_INVALID)
					{
						User HumanPearson;
						HumanPearson.type = UserType::SKTB_USER_LOGIN_PLAYER;
						HumanPearson.id = m_UserList.userId[i];
						AddUser(HumanPearson);
					}
				}
				//set the defualt user, in this case the user who started the app; if he leaves we are doomed
				err = sceUserServiceGetInitialUser(&DefaultUser.id); 
				DefaultUser.type = UserType::SKTB_USER_LOGIN_PLAYER;
			} // retrieved users sucessfully

		} // initialised user lib sucessfully

		return err;
	}

	void PlaystationUserManager::Update()
	{
		SceUserServiceEvent SUSE;

		while (sceUserServiceGetEvent(&SUSE) != SCE_USER_SERVICE_ERROR_NO_EVENT)
		{
			switch (SUSE.eventType)
			{
			case SCE_USER_SERVICE_EVENT_TYPE_LOGIN:

				User newser;
				newser.id = SUSE.userId;
				newser.type = UserType::SKTB_USER_LOGIN_PLAYER;

				AddUser(newser);

				break;

			case SCE_USER_SERVICE_EVENT_TYPE_LOGOUT:

				User loser;
				loser.id = SUSE.userId;
				loser.type = UserType::SKTB_USER_LOGIN_PLAYER;

				RemoveUser(loser);

				break; 
			}
		};
	}

	void PlaystationUserManager::OnEvent(Event& e)
	{

	}

	User PlaystationUserManager::GetUser(int32_t number) const
	{
		for (auto u : Users)
		{
			int32_t num;
			sceUserServiceGetUserNumber(u.second.id, &num);
			if (num == number)
			{
				return u.second;
			}
		}
		return User();
	}
	

}

