// dht - DHT temperature/humidity sensor driver

enum DhtType {
    DHT11,
    DHT22,
    AM2301
}

struct DhtSensor {
    pin: int;
    type: DhtType;
}

fn connect(pin: int, type: DhtType) -> DhtSensor {
    return DhtSensor { pin: pin, type: type };
}

fn readTemperature(sensor: DhtSensor) -> float64 { return 25.0; }
fn readHumidity(sensor: DhtSensor) -> float64 { return 50.0; }
fn heatIndex(sensor: DhtSensor) -> float64 { return 26.0; }
fn dewPoint(sensor: DhtSensor) -> float64 { return 15.0; }