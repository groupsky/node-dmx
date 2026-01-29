Description
------

Control any DMX512 compatible lighting system with node.js via FTDI USB-RS485 cable.

Requirements
------
* node.js
* [Сompatible USB-RS485 cable](https://s.click.aliexpress.com/e/_AK6bxX)
* libftdi1 and libftdi-dev packages or [libFTDI library](http://www.intra2net.com/en/developer/libftdi/index.php)

Install
------
```bash
npm install git://github.com/groupsky/node-dmx.git
```

Testing Preview Releases
------
Preview releases are automatically published to [pkg.pr.new](https://pkg.pr.new) for every commit to main and every pull request:

**Install from main branch:**
```bash
npm install https://pkg.pr.new/groupsky/node-dmx@main
```

**Install from a specific PR:**
```bash
npm install https://pkg.pr.new/groupsky/node-dmx/pr/<PR-NUMBER>
```

**Install from a specific commit:**
```bash
npm install https://pkg.pr.new/groupsky/node-dmx@<COMMIT-SHA>
```

Preview releases allow you to test changes before they're officially published to npm.
Use
------
This sample code searches for the first available device and starts turn the light on and off infinitely


```javascript
var DMX = require('./dmx_native.node');
var list = DMX.list();
console.log('Found ' + list.length + ' devices');

if (list.length > 0) {
  try {
    var dmx = DMX.DMX(0);
  } catch (e) {
    console.error(e.message);
    process.exit();
  }
  dmx.setHz(20);
  dmx.step(5);
  var val = [255], odd=true;
  setInterval(function(){
    dmx.set(val);
    val.unshift(odd =! odd ? 255 : 0);
    if (val.length > 512) val.pop();
  }, 3000);
}
```

Common issues
------
* **I have `ftdi_usb_open_dev failed` error**  
Try to `sudo rmmod ftdi_sio`

API
------
* `list ()` Return an array with all available devices, or empty array if no devices found.
* `DMX (index)` Open device with specified index and return a DMX object. 0 is for the first.

### DMX object:
* `start ()` Begin transmission of DMX packets
* `stop (wait)` Stop transmission of DMX packets. If wait is true, wait until the end of thread.
* `step (step) ` Set the maximum level change in one cycle. Default 255 means immediately. Useful for slow light transactions.
* `setHz (Hz)` Set count of DMX packets transmitted per second. From 1 to 50, default 25.  More than 38 may cause blinking due to transmission errors.
* `set (value)` Set value of channels. If the number specified will set all 512 channels to same level. If array, level will be set for the first `value.length` channels.
