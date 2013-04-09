unit=require('./units');
opendmx=require('../build/Release/opendmx.node');

unit.run('load', function() {
    dmxio = new opendmx.io();
    
    var b = new Buffer(32)
    
    b.writeInt8(0, 0)
    
    unit.assert(dmxio != null)
    
    unit.assert(dmxio.open("/dev/cu.usbserial-AH019G6B"))
    
    unit.assert(dmxio.start())
    
    unit.assert(dmxio.write(b))
    
    unit.assert(dmxio.close())
});


