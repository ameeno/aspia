//
// Aspia Project
// Copyright (C) 2019 Dmitry Chapyshev <dmitry@aspia.ru>
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

#include "base/win/session_id.h"

#include <limits>
#include <type_traits>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace base::win {

static_assert(std::is_same<SessionId, DWORD>());
static_assert(kInvalidSessionId == std::numeric_limits<DWORD>::max());

SessionId activeConsoleSessionId()
{
    return WTSGetActiveConsoleSessionId();
}

} // namespace base::win