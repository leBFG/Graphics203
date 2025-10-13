#pragma once
#include "Skateboard/User.h"
#include <user_service.h>

#define SCE_OK 0

namespace Skateboard
{
    class PlaystationUserManager : public UserManager
    {
    public:
        PlaystationUserManager() = default;
        ~PlaystationUserManager() override;

    protected:
        int Init() override;
        void Update() override;
        void OnEvent(Event& e) override;
        User GetUser(int32_t number) const override;
       
    };

}