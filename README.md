# Linux kernel module for BMP280/BME280 environmental sensor

> **Tested on**:
> * **Yocto Scrthgap**
> * **Linux kernel 6.12** (`ti-staging`).
> * **BeagleBone Black Rev.C**
>

## Plans

* [x] I2C support
* [ ] SPI 4 wire support
* [ ] SPI 3 wire support
* [x] BME280 support
* [x] BMP280 support

## TODO

* ~~Add T<sub>fine</sub> attribute `/sys/bus/i2c/devices/2-0076/control/t_fine`~~
* Add device tree overlays
* Add doxy comments
* Add description to README.md
* Add usage examples to README.md


# Device tree example

```device-tree
&i2c2 {
	bme280: bme280@76 {
		compatible = "bosch,bme280";
		reg = <0x76>;
		status = "okay";
	};
};
```

## Attributes

All necessary features you can find in `/sys/bus/i2c/devices/2-0076`

### Control attributes

* **Driver version** (`read-only`): `/sys/bus/i2c/devices/2-0076/control/version`
* **Device mode** (`read/write`): `/sys/bus/i2c/devices/2-0076/control/mode`
  modes: `sleep`, `forced_1`, `forced_2`, `normal`
* **Reset** (`write-only`): `/sys/bus/i2c/devices/2-0076/control/reset`
  write `reset` to this attribute cause device reset, all other values
  will be ignored
* **Status** (`read-only`): `/sys/bus/i2c/devices/2-0076/control/status`
* **IIR filter** (`read/write`): `/sys/bus/i2c/devices/2-0076/control/iir_filter`
  values: `off`, `2`, `4`, `8`, `18`
* **Standby time** (`read/write`): `/sys/bus/i2c/devices/2-0076/control/stby_time`
  values: `500`, `62500`, `125000`, `250000`, `500000`, `1000000`, `10000`, `20000`
* **Oversampling** (`read/write`): `/sys/bus/i2c/devices/2-0076/control/oversampling`
  values: `skipped`, `1`, `2`, `4`, `8`. Write to this attriute set oversampling
  values for all sensors

### Sensor attrbutes

* **Calibration values** (`read-only`): `/sys/bus/i2c/devices/2-0076/$SENSOR_NAME/calibration`
* **Oversampling** (`read/write`): `/sys/bus/i2c/devices/2-0076/$SENSOR_NAME/oversampling`
* **Calibrated raw value** (`read-only`): `/sys/bus/i2c/devices/2-0076/$SENSOR_NAME/value`
* **Divisor for `value`** (`read-only`): `/sys/bus/i2c/devices/2-0076/$SENSOR_NAME/scale`

Where `$SENSOR_NAME` is `temperature`, `pressure` or `humidity`

