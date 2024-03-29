// actions/status.cpp -- Meta status actions, such as checking the player's score or condition.
/// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/status.hpp"
#include "core/core.hpp"
#include "core/strx.hpp"
#include "world/player.hpp"
#include "world/room.hpp"
#include "world/time-weather.hpp"
#include "world/world.hpp"


// Check the player's current total score.
void ActionStatus::score()
{
    core()->message("{U}Your current score is {C}" + StrX::intostr_pretty(core()->world()->player()->score()) + "{U}.");
}

// Determines the current time of day.
void ActionStatus::time(std::shared_ptr<Mobile> mob)
{
    const auto time_weather = core()->world()->time_weather();
    const std::shared_ptr<Room> room = core()->world()->get_room(mob->location());
    const bool indoors = room->tag(RoomTag::Indoors);
    const bool can_see_outside = room->tag(RoomTag::CanSeeOutside);
    const std::string date = time_weather->day_name() + ", the " + time_weather->day_of_month_string() + " day of " +  time_weather->month_name();

    std::string time_str;
    if (can_see_outside || !indoors) time_str = "It is now " + StrX::str_tolower(time_weather->time_of_day_str(true));
    else
    {
        std::string tod_str = time_weather->time_of_day_str(false);
        if (tod_str == "DAY") tod_str = "daytime";
        time_str = "It is around " + StrX::str_tolower(tod_str);
    }
    core()->message(time_weather->weather_message_colour() + time_str + " on " + date + ".");
}

// Checks the nearby weather.
void ActionStatus::weather(std::shared_ptr<Mobile> mob)
{
    const std::shared_ptr<Room> room = core()->world()->get_room(mob->location());
    const bool indoors = room->tag(RoomTag::Indoors);
    const bool can_see_outside = room->tag(RoomTag::CanSeeOutside);
    if (indoors && !can_see_outside) core()->message("{y}You {Y}can't see {y}the weather outside from here.");
    else core()->message(core()->world()->time_weather()->weather_message_colour() + core()->world()->time_weather()->weather_desc());

    const int player_temp = room->temperature(Room::TEMPERATURE_FLAG_WITH_PLAYER_BUFFS);
    const int room_temp = room->temperature(Room::TEMPERATURE_FLAG_IGNORE_PLAYER_CLOTHES);
    std::string temp_str = "{U}The temperature is ";
    switch (room_temp)
    {
        case 0: case 1: temp_str += "{C}freezing"; break;
        case 2: temp_str += "{C}cold"; break;
        case 3: temp_str += "{C}chilly"; break;
        case 4: case 5: temp_str += "{G}pleasant"; break;
        case 6: temp_str += "{Y}warm"; break;
        case 7: temp_str += "{Y}toasty"; break;
        case 8: temp_str += "{R}hot"; break;
        case 9: temp_str += "{R}searing"; break;
    }
    temp_str += "{U}, and you feel ";
    switch (player_temp)
    {
        case 0: case 1: temp_str += "{C}frozen"; break;
        case 2: temp_str += "{C}cold"; break;
        case 3: temp_str += "{C}chilly"; break;
        case 4: case 5: temp_str += "{G}fine"; break;
        case 6: temp_str += "{Y}warm"; break;
        case 7: temp_str += "{Y}toasty"; break;
        case 8: temp_str += "{R}hot"; break;
        case 9: temp_str += "{R}sweltering"; break;
    }
    temp_str += "{U}.";
    core()->message(temp_str);
}
