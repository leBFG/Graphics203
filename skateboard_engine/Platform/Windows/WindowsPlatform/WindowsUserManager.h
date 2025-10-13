#include "Skateboard/User.h"

namespace Skateboard
{
    class WindowsUserManager : public UserManager
    {
    public:
        WindowsUserManager() {};
        ~WindowsUserManager() {};

	protected:
		virtual void Update() {};
		virtual void OnEvent(Event& e) {};

		std::unordered_map<UserID, User> Users; // map stores user Id as an identifier and User which at the moment
		User DefaultUser;

		void AddUser(const User& User) {};
		void RemoveUser(const User& User) {};

	public:
		virtual User GetUser(int32_t number) const { return DefaultUser; };

    protected:
        int Init() override;
    };

}