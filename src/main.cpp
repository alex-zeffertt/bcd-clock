#include "Max7219.h"
#include "Kwm30881.h"
#include "Bootsel.h"
#include "pico/bootrom.h" // reset_usb_boot

// Short enough to detect presses but long enough to debounce, must divide 1000
#define TICK_HZ 100

constexpr int N_ROWS = 10;
constexpr int N_COLS = 8;

void update_row(bool table[N_ROWS][N_COLS], int i)
{
    for (int j = N_COLS - 1; j >= 0; j--)
    {
        bool present = table[i][j];
        if (present && i != j + 1)
        {
            bool below = (i + 1 < N_ROWS) ? (below = table[i + 1][j]) : false;

            if (below == false)
            {
                if (i + 1 < N_ROWS)
                    table[i + 1][j] = table[i][j];
                table[i][j] = false;
            }
            else
            {
                if (j + 1 < N_COLS)
                    table[i][j + 1] = table[i][j];
                table[i][j] = false;
                table[i + 1][j] = false;
                if (i + 2 < N_ROWS)
                    table[i + 2][j] = true;
            }
        }
    }
}

void print_table(Kwm30881 &kwm30881, bool table[N_ROWS][N_COLS])
{
    uint8_t cols[Kwm30881::NUM_COLS] = {0};

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            bool val = table[9 - i][j - 8];
            cols[j] |= int(val) << i;
        }
    }
    kwm30881.write_cols(cols, true);
}

int main()
{
    Max7219 max7219(GPIO_PIN_DIN, GPIO_PIN_CLK, GPIO_PIN_LOAD);
    Kwm30881 kwm30881(max7219);
    Bootsel button;
    absolute_time_t now = get_absolute_time();
    bool button_prev = button.get();
    bool table[N_ROWS][N_COLS] = {false};
    int row = N_ROWS - 1;
    int loops = 0;
    int tick = 0;
    int posedge_tick = 0;

    stdio_usb_init();

    while (true)
    {
        // Wait for next tick
        absolute_time_t next = delayed_by_ms(now, 1000 / TICK_HZ);

        sleep_until(next);

        tick++;

        // Check for EV_BUTTON_RELEASED event
        bool button_new = button.get();
        if (button_prev != button_new)
        {
            if (button_prev)
            {
                if (tick - posedge_tick > TICK_HZ)
                    reset_usb_boot(0, 0);
                else
                {

                    loops = 0;
                    row = N_ROWS - 1;
                    memset(table, 0, sizeof(table));
                }
            }
            else
            {
                posedge_tick = tick;
            }
            button_prev = button_new;
        }

        if (row == N_ROWS - 1)
        {
            print_table(kwm30881, table);
        }

        if (row == 0)
        {
            loops++;
            if (loops == 10)
            {
                loops = 0;
                table[0][0] = true;
            }
        }

        update_row(table, row);

        row = row ? (row - 1) : (N_ROWS - 1);

        now = next;
    }
}
