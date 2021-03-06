//
// Aspia Project
// Copyright (C) 2020 Dmitry Chapyshev <dmitry@aspia.ru>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
//

#include "router/authenticator_manager.h"

#include "base/logging.h"
#include "base/task_runner.h"
#include "crypto/secure_memory.h"
#include "net/network_channel.h"
#include "router/session.h"

namespace router {

AuthenticatorManager::AuthenticatorManager(
    std::shared_ptr<base::TaskRunner> task_runner, Delegate* delegate)
    : task_runner_(std::move(task_runner)),
      delegate_(delegate)
{
    DCHECK(task_runner_ && delegate_);
}

AuthenticatorManager::~AuthenticatorManager()
{
    crypto::memZero(&private_key_);
}

void AuthenticatorManager::setPrivateKey(const base::ByteArray& private_key)
{
    private_key_ = private_key;
    DCHECK(!private_key_.empty());
}

void AuthenticatorManager::setUserList(std::shared_ptr<UserList> user_list)
{
    user_list_ = std::move(user_list);
    DCHECK(user_list_);
}

void AuthenticatorManager::addNewChannel(std::unique_ptr<net::Channel> channel)
{
    DCHECK(channel);

    std::unique_ptr<Authenticator> authenticator =
        std::make_unique<Authenticator>(task_runner_, std::move(channel));

    // Start the authentication process.
    if (!authenticator->start(private_key_, user_list_, this))
        return;

    // Create a new authenticator for the connection and put it on the list.
    pending_.emplace_back(std::move(authenticator));
}

void AuthenticatorManager::onComplete()
{
    for (auto it = pending_.begin(); it != pending_.end();)
    {
        Authenticator* current = it->get();

        switch (current->state())
        {
            case Authenticator::State::SUCCESS:
            case Authenticator::State::FAILED:
            {
                if (current->state() == Authenticator::State::SUCCESS)
                    delegate_->onNewSession(current->takeSession());

                // Authenticator not needed anymore.
                task_runner_->deleteSoon(std::move(*it));
                it = pending_.erase(it);
            }
            break;

            case Authenticator::State::PENDING:
                // Authentication is not yet completed, skip.
                ++it;
                break;

            default:
                NOTREACHED();
                ++it;
                break;
        }
    }
}

} // namespace router
