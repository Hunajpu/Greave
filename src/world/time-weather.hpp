// world/time-weather.hpp -- The time and weather system.
// Copyright (c) 2021 Raine "Gravecat" Simmons. All rights reserved.
// Weather system originally based on Keran's MUSH/MUX Weather and Time Code Package Version 4.0 beta, copyright (c) 1996-1998 Keran.

#pragma once
#include "core/greave.hpp"

namespace SQLite { class Database; }    // defined in 3rdparty/SQLiteCpp/Database.h


class TimeWeather
{
public:
    enum class LightDark : uint8_t { LIGHT, DARK, NIGHT };
    enum class LunarPhase : uint8_t { NEW, WAXING_CRESCENT, FIRST_QUARTER, WAXING_GIBBOUS, FULL, WANING_GIBBOUS, THIRD_QUARTER, WANING_CRESCENT };
    enum class Season : uint8_t { AUTO, WINTER, SPRING, SUMMER, AUTUMN };
    enum class TimeOfDay : uint8_t { DAWN, SUNRISE, MORNING, NOON, SUNSET, DUSK, NIGHT, MIDNIGHT, DAY };
    enum class Weather : uint8_t { BLIZZARD, STORMY, RAIN, CLEAR, FAIR, OVERCAST, FOG, LIGHTSNOW, SLEET };
    enum Time { SECOND = 1, MINUTE = 60, HOUR = 3600, DAY = 86400 };

    static const std::string    SQL_TIME_WEATHER;   // SQL table construction string for time and weather data.

                TimeWeather();                      // Constructor, sets default values.
    Season      current_season() const;             // Gets the current season.
    std::string day_name() const;                   // Returns the name of the current day of the week.
    int         day_of_month() const;               // Returns the current day of the month.
    std::string day_of_month_string() const;        // Returns the day of the month in the form of a string like "1st" or "19th".
    Weather     get_weather() const;                // Gets the current weather, runs fix_weather() internally.
    LightDark   light_dark() const;                 // Checks whether it's light or dark right now.
    void        load(std::shared_ptr<SQLite::Database> save_db);    // Loads the time/weather data from disk.
    std::string month_name() const;                 // Returns the name of the current month.
    LunarPhase  moon_phase() const;                 // Gets the current lunar phase.
    bool        pass_time(float seconds);           // Causes time to pass.
    void        save(std::shared_ptr<SQLite::Database> save_db) const;  // Saves the time/weather data to disk.
    std::string season_str(Season season) const;    // Converts a season enum to a string.
    TimeOfDay   time_of_day(bool fine) const;       // Returns the current time of day (morning, day, dusk, night).
    int         time_of_day_exact() const;          // Returns the exact time of day.
    std::string time_of_day_str(bool fine) const;   // Returns the current time of day as a string.
    long long   time_passed() const;                // Returns the total amount of time passed in this game.
    std::string weather_desc() const;               // Returns a weather description for the current time/weather, based on the current season.
    std::string weather_message_colour() const;     // Returns a colour to be used for time/weather messages, based on the time of day.
    std::string weather_str(Weather weather) const; // Converts a weather integer to a string.

private:
    static const int    LUNAR_CYCLE_DAYS;       // How many days are in a lunar cycle?
    static const float  UNINTERRUPTABLE_TIME;   // The maximum amount of time for an action that cannot be interrupted.

    Weather     fix_weather(Weather weather, Season season) const;  // Fixes weather for a specified season, to account for unavailable weather types.
    void        trigger_event(Season season, std::string *message_to_append, bool silent);  // Triggers a time-change event.
    std::string weather_desc(Season season) const;                  // Returns a weather description for the current time/weather, based on the specified season.

    int         m_day;          // The current day of the year.
    int         m_moon;         // The current moon phase.
    int         m_time;         // The time of day.
    long long   m_time_passed;  // The total amount of time that has passed in this game.
    float       m_time_passed_subsecond;    // For counting time passed in amounts of time less than a second.
    Weather     m_weather;      // The current weather.

    std::map<std::string, std::string>  m_tw_string_map;        // The time and weather strings from data/weather.yml
    std::vector<std::string>            m_weather_change_map;   // Weather change maps, to determine odds of changing to different weather types.
};
