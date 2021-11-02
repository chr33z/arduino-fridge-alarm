## Arduino Fridge Alarm: Let it go! Let it go!

It's just a mess! Water on the floor, food thawing away and all the wasted time to clean this chaos! You left the freezer door open again!

We need a solution, now!

# Arduino to the Rescue

We have a few spare parts laying around:

- Arduino
- 5V supply
- Piezo speaker
- Magnetic switch
- 2x LED (different colors)

This should be enough to build a working fridge alarm. The parts are also quite cheap, but not as cheap as a ready made fridge alarm you can buy at Amazon. But where's the fun in that?

# Wiring

Its not that complicated and the digital pin configuration can be seen in the code:

```
buzzer = 3;       // piezo speaker
sensor = 4;       // magnetic sensor
statusLed = 9;    // pulsating LED to show that it is running
openLed = 10;     // LED to show the fridge is open
```

I'm not good at drawing skematics but this should be really simple with some guidance from arduino tutorials.

# Threading and Melody

I'm using the [protothreads library](https://www.arduino.cc/reference/en/libraries/protothreads/) for arduino to get some pseudo threading. This way I can check the door state, control the LEDs and play the melody simultaniously.

The melody is based on [this arduino tutorial](https://www.arduino.cc/en/Tutorial/BuiltInExamples/toneMelody). There may be better ways, but this did the job and was easy enough to configure. I chose the famous title track from the movie "Frozen" - "Let it go!" - to tell me that the fridge door is open for too long.

# How to Use

Using the Arduino IDE, just install the protothreads library using the instructions on their page, load the project and make sure the wiring is correct.

This system is running constantly for a few months now without interruption or hickups.
