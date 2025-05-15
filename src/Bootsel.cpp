#include <pico/stdlib.h>
#include <cstdint>
#include "hardware/sync.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"
#include "Bootsel.h"

#define CS_BIT (1u << 1)

Bootsel::Bootsel()
{
    // Use the default LED to show when the bootsel button is pressed - this is updated in Bootsel::get
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
}

bool __no_inline_not_in_flash_func(Bootsel::get)()
{
    const uint CS_PIN_INDEX = 1;
    uint32_t flags = save_and_disable_interrupts();

    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl, GPIO_OVERRIDE_LOW << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    bool button_state = !(sio_hw->gpio_hi_in & CS_BIT);

    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl, GPIO_OVERRIDE_NORMAL << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    restore_interrupts(flags);

    // Update default LED
    gpio_put(PICO_DEFAULT_LED_PIN, button_state);

    return button_state;
}
