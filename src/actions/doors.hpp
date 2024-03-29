// actions/doors.hpp -- Actions involving doors, windows, and other such similar things.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Mobile;                   // defined in world/mobile.hpp
enum class Direction : uint8_t; // defined in world/room.hpp


class ActionDoors
{
public:
    static bool lock_or_unlock(std::shared_ptr<Mobile> mob, Direction dir, bool unlock, bool silent_fail = false);  // Attempts to lock or unlock a door, with optional messages.
    static bool open_or_close(std::shared_ptr<Mobile> mob, Direction dir, bool open);   // Attempts to open or close a door, window, or other openable portal.

private:
    static const float  TIME_CLOSE_DOOR;    // The time taken (in seconds) to close a door.
    static const float  TIME_LOCK_DOOR;     // The time taken (in seconds) to lock a door.
    static const float  TIME_OPEN_DOOR;     // The time taken (in seconds) to open a door.
    static const float  TIME_UNLOCK_DOOR;   // The time taken (in seconds) to unlock a door.
};
