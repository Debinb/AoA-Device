# AoA-Device
An embedded systems project to tackle the real-world problem of the high computational power requirement of AoA (Angle of Arrival) devices using the TM123GH6PM microcontroller under low power and low cost. 

<p align="center">
<img src="AoAdevice.png" width="329" height="356.875">
</p>

## Hardware Features
- ARM M4F based TM123GH6PM
- Electronic components used:
    |   Components   |  Qty |
    | -------------- |:----:|
    | LM2902 Op-amp  |  1 |
    |CMC-9745-44P Mic| 4 |
    | 2.2kΩ resistor | 4 |
    | 1kΩ resistor   | 4 |
    | 10kΩ resistor  | 4 | 
    | 100kΩ resistor | 4 |
    | 0.1μF capacitor| 2 |
    | 1μF capacitor  | 4 |
    | 10μF capacitor | 1 |
- Peripherals used: 
    - ADC0 (Sequence Sampler)
    - ADC1 (Digital Comparator)
    - Timers
    - NVIC
    - DMA
    - UART

### Circuit Diagram
<p align="center">
<img src="Circuit.png" width="329" height="356.875">
Each microphone uses this circuit
</p>
## Software Features