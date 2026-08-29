// embedded.adc - ADC HAL

enum AdcAtten {
    DB_0,
    DB_2_5,
    DB_6,
    DB_11
}

enum AdcWidth {
    BIT_9,
    BIT_10,
    BIT_11,
    BIT_12
}

fn adc_setup(pin: int) -> AdcChannel { }
fn adc_setup_atten(pin: int, atten: AdcAtten) -> AdcChannel { }

struct AdcChannel {
    pin: int;
    channel: int;
}

impl AdcChannel {
    fn read(self) -> int { return 0; }
    fn read_voltage(self) -> float64 { return 0.0; }
    fn set_atten(self, atten: AdcAtten) -> void { }
    fn set_width(self, width: AdcWidth) -> void { }
}