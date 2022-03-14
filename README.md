# PonGPIO

Created by Tejas Narayanan, Aditya Saligrama, and Justin Weiler for the Stanford CS140e Winter 2022 final project.

## About this project

This is our take on the classic video game of Pong -- but split across three Raspberry Pis networking via custom GPIO communication protocols.

One Pi receives input from an NES controller, another Pi governs the game logic, and the last Pi handles output to a display over HDMI using the framebuffer.

In this setup, a player plays against an AI.

## Communication protocol

We implemented a custom communication protocol over GPIO. We needed to send 6 8-bit values between the game logic Pi and the display Pi, and we needed to do so quickly in order to
improve the framerate of the game. The protocol works as follows:

1. The sender transmits a clock pulse indicating that transmission will begin.
2. The sender writes the first bits of the 6 data values to the 6 data wires, and pulses the clock to indicate that it has written the bits.
3. The receiver reads the bits when the clock pin goes high, and waits for the clock pin to go low.
4. Steps 2-3 repeat for the other 7 bits of data.
5. After the receiver processes and uses the data, it pulses an acknowledgment wire to indicate that it is ready to recieve a new transmission.

## Challenges

We initially planned to use the NRF to communicate between the Pis, but we ran into issues with the drivers and were unable to determine the sources of error. Therefore, we opted
to switch to communicating via GPIO.

When implementing our custom protocol, we encountered numerous issues with mysterious 1-bits that shouldn't have been transmitted. We first thought that it was an issue with timing,
so we explored other options like using the Raspberry Pi's built-in UART by switching the UART pins on-the-fly. However, this didn't work. We even tried wiring the main UART pins to
both the computers and the other Pis, but we quickly realized that this was a bad idea, and that it too did not work. Eventually, we settled back on attempting to fix our original GPIO
protocol. After many hours of debugging, we determined multiple sources of error:

* The ground pins between our Pis were not connected, meaning that the same voltage could be interpreted as low and high differently between the Pis.
* We were improperly using the `bit_set` function in `bit-support.h`; it takes in `uint32_t` values, but we were passing in `uint8_t` values that were erroneously being interpreted
as `uint32_t` values. This resulted in the wrong bits being set to the values being read over the data lines.
* We did not fill the array we were writing into with known values (e.g. 0). This would not be an issue with a properly-used `bit_set`, but since we were writing to the wrong bits
as mentioned above, some extra 1-bits were appearing in our final values. This was the cause of the "random" 1-bits that were appearing.

After fixing these issues, we were able to successfully transfer data over the GPIO pins at high enough speeds to run the game at ~25 FPS. The speed of communication is much faster,
at ~75 FPS, but the overall game speed is slowed down by the speed at which we draw to the framebuffer.

Our initial iteration of the protocol only used one data wire, but this proved to be very slow; we were only able to transmit about 12 data packets per second. To speed it up, we
decided to use six wires, one for each value in the packet, to speed up the transmission by 6x.
