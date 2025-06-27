#pragma once

#include "pico/bootrom.h" // reset_usb_boot
#include "Bootsel.h"
#include "Kwm30881.h"
#include "Fsm.h"
#include "MainFsmTable.h"
#include <cstdio>

class MainFsm : public MainFsmTable, public Fsm<MainFsm>
{
  public:
    MainFsm(Kwm30881 &kwm30881, Bootsel &button, uint64_t tick_hz)
        : _kwm30881(kwm30881), _button(button), _1s(tick_hz), _0_5s(_1s / 2)
    {
        set_initial(ST_UPDATE_TIME);
    }

  private:
    Event handle_action(Action action, void *context)
    {
        auto func = _handlers[action];
        return func ? ((this)->*func)(context) : EV_NULL_EVENT;
    }

    // action handlers

    Event update_time_tick(void *context)
    {
        bool button = common_tick();
        if (button && tick - button_posedge_tick > _1s)
            return EV_BUTTON_HELD_1SEC;

        // Drop a marble into the top right corner once per second
        if (tick % _1s == 0)
            table[0][7] = 1;

        // Display the table
        print_table();

        // Update the state of the table every tick
        update_table();

        return EV_NULL_EVENT;
    }

    Event setting_hours_tick(void *context)
    {
        bool button = common_tick();

        int hours, minutes, seconds;
        read_time(hours, minutes, seconds);

        if (button && (((tick - button_posedge_tick) % _0_5s) == (_0_5s - 1)))
            hours = (hours + 1) % 24;

        set_time(hours, minutes, seconds);
        print_table(true);
        return EV_NULL_EVENT;
    }

    Event setting_minutes_tick(void *context)
    {
        bool button = common_tick();

        int hours, minutes, seconds;
        read_time(hours, minutes, seconds);

        if (button && (((tick - button_posedge_tick) % _0_5s) == (_0_5s - 1)))
            minutes = (minutes + 1) % 60;

        set_time(hours, minutes, seconds);
        print_table(false, true);
        return EV_NULL_EVENT;
    }

    Event setting_seconds_tick(void *context)
    {
        bool button = common_tick();

        int hours, minutes, seconds;
        read_time(hours, minutes, seconds);

        if (button && (((tick - button_posedge_tick) % _0_5s) == (_0_5s - 1)))
            seconds = (seconds + 1) % 60;

        set_time(hours, minutes, seconds);
        print_table(false, false, true);
        return EV_NULL_EVENT;
    }

    Event start_upgrade(void *context)
    {
        reset_usb_boot(0, 0);
        return EV_NULL_EVENT; // Never gets here
    }

    // Loggers

    void log_event(Event event) const
    {
        if (event != EV_TICK)
            printf("Event: %s\n", event_names[event]);
    }

    void log_state(State oldstate, State newstate) const
    {
        printf("State change: %s -> %s\n", state_names[oldstate], state_names[newstate]);
    }

    // Utiliies

    // Returns state of button
    bool common_tick()
    {
        tick++;
        bool button = _button.get();
        bool button_posedge = button && !button_prev;
        button_prev = button;
        if (button_posedge)
            button_posedge_tick = tick;
        return button;
    }

    // Updates display
    void print_table(bool invert_hours = false, bool invert_minutes = false, bool invert_seconds = false)
    {
        uint8_t cols[Kwm30881::NUM_COLS] = {0};

        for (int col = 0; col < Kwm30881::NUM_COLS; col++)
        {
            for (int bit = 0; bit < 8; bit++)
            {
                int val = (table[8 - bit][7 - col]) ? 1 : 0;
                cols[col] |= val << bit;
            }
        }

        if (invert_hours)
        {
            cols[7] ^= 0xff;
            cols[6] ^= 0xff;
        }
        if (invert_minutes)
        {
            cols[5] ^= 0xff;
            cols[4] ^= 0xff;
            cols[3] ^= 0xff;
        }
        if (invert_seconds)
        {
            cols[2] ^= 0xff;
            cols[1] ^= 0xff;
            cols[0] ^= 0xff;
        }

#ifdef ROTATE
        _kwm30881.write_cols(cols, true);
#else
        _kwm30881.write_cols(cols, false);
#endif
    }

    // Determine what happens to the marble at row, col (assumes it is present)
    enum action
    {
        LEFT,
        DOWN,
        STAY
    };
    action get_action(int row, int col)
    {
        // Determine number of marbles in column
        int col_count = 0;
        for (int i = 0; i < NROWS; i++)
            col_count += table[i][col];

        // Determine if this column is flushing
        bool flushing = false;
        if (col_count == COL_BASE[col])
        {
            flushing = true;
        }
        else
        {
            for (int i = ROW_MAX[col] + 1; i < NROWS; i++)
            {
                if (table[i][col] == 1)
                {
                    flushing = true;
                    break;
                }
            }
        }

        // Determine marble move
        if (flushing)
            return (row == ROW_MIN[col]) ? LEFT : DOWN;

        return (row == ROW_MAX[col] || table[row + 1][col]) ? STAY : DOWN;
    }

    // Called every tick to update the state of the table.
    // Each marble can move at most one position per tick, including moving off
    // the table.
    void update_table()
    {
        int table_new[NROWS][NCOLS];
        memset(table_new, 0, sizeof(table_new));

        // Decide new position in table For each marble in table
        for (int row = 0; row < NROWS; row++)
        {
            for (int col = 0; col < NCOLS; col++)
            {
                if (table[row][col] == 1)
                {
                    // Marble is present: choose an action
                    auto action = get_action(row, col);

                    if (action == LEFT)
                    {
                        if (col > 0)
                            table_new[row][col - 1] = 1;
                    }
                    else if (action == DOWN)
                    {
                        if (row < NROWS - 1)
                            table_new[row + 1][col] = 1;
                    }
                    else
                    {
                        table_new[row][col] = 1;
                    }
                }
            }
        }

        // Update table
        memcpy(table, table_new, sizeof(table));
    }

    // Read the current table and infer the time
    void read_time(int &hours, int &minutes, int &seconds)
    {
        // Read in each of the 8 digits
        int digits[NCOLS];
        memset(digits, 0, sizeof(digits));

        for (int i = 0; i < NROWS; i++)
        {
            for (int j = 0; j < NCOLS; j++)
            {
                if (digits[j] < COL_BASE[j])
                    digits[j] += table[i][j];
            }
        }
        hours = digits[0] * COL_BASE[1];
        hours += digits[1];
        minutes = digits[2] * COL_BASE[3] * COL_BASE[4];
        minutes += digits[3] * COL_BASE[4];
        minutes += digits[4];
        seconds = digits[5] * COL_BASE[6] * COL_BASE[7];
        seconds += digits[6] * COL_BASE[7];
        seconds += digits[7];
    }

    // Set the time in table
    void set_time(int hours, int minutes, int seconds)
    {
        memset(table, 0, sizeof(table));

        int col_counts[NCOLS] = {
            (hours / 3) % COL_BASE[0],    // override .clang-format alignment
            (hours / 1) % COL_BASE[1],    //
            (minutes / 20) % COL_BASE[2], //
            (minutes / 5) % COL_BASE[3],  //
            (minutes / 1) % COL_BASE[4],  //
            (seconds / 20) % COL_BASE[5], //
            (seconds / 5) % COL_BASE[6],  //
            (seconds / 1) % COL_BASE[7],
        };

        for (int col = 0; col < NCOLS; col++)
        {
            int row = ROW_MAX[col];
            int count = col_counts[col];
            while (count)
            {
                table[row][col] = 1;
                row -= 1;
                count -= 1;
            }
        }
    }

    using Function = Event (MainFsm::*)(void *);
    static constexpr const Function _handlers[NUM_ACTIONS] = {
        [AC_IGNORE_EVENT] = nullptr,
        [AC_UPDATE_TIME_TICK] = &MainFsm::update_time_tick,
        [AC_SETTING_HOURS_TICK] = &MainFsm::setting_hours_tick,
        [AC_SETTING_MINUTES_TICK] = &MainFsm::setting_minutes_tick,
        [AC_SETTING_SECONDS_TICK] = &MainFsm::setting_seconds_tick,
        [AC_START_UPGRADE] = &MainFsm::start_upgrade,
        [AC_NO_ACTION] = nullptr,
    };

    Kwm30881 &_kwm30881;
    Bootsel &_button;
    const uint64_t _1s;
    const uint64_t _0_5s;
    uint64_t tick = 0;
    uint64_t button_posedge_tick = 0;
    bool button_prev = false;

    // Example table state showing time 5:59:59
    //
    // The number of marbles in each column represents the digit
    // However, the base for each digit depends on the column and
    // is not constant.
    //
    // Each column has a different minimum row and maximum row it
    // can remain in if it is not falling.
    //
    //  0 . . . . . . . .
    //  1 . . . . . . . *
    //  2 . . . . . . * *
    //  3 . . . . . * * *
    //  4 . . . . * * * *
    //  5 . . . * * . . .
    //  6 . . * * * . . .
    //  7 . * * * * . . .
    //  8 * * . . . . . .
    //  9 . . . . . . . .
    //    0 1 2 3 4 5 6 7
    //    h h m m m s s s
    //    3   20    20
    //      1   5     5
    //            1     1
    static constexpr int NROWS = 10;
    static constexpr int NCOLS = 8;
    static constexpr int COL_BASE[NCOLS] = {2, 3, 3, 4, 5, 3, 4, 5};
    static constexpr int ROW_MAX[NCOLS] = {8, 8, 7, 7, 7, 4, 4, 4};
    static constexpr int ROW_MIN[NCOLS] = {7, 6, 5, 4, 3, 2, 1, 0};
    int table[NROWS][NCOLS] = {};

    // Allow Fsm to call private functions handle_action, log_event, and log_state
    // This is more efficient than virtual functions and avoids implicit heap allocation.
    friend Fsm;
};
