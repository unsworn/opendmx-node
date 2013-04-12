unit=require('./units');
opendmx=require('../build/Release/opendmx.node');
/*
unit.run('connect', function() {
    var dmxio = new opendmx.io();
    
    unit.assert(dmxio != null)
    
    unit.assert(dmxio.open())
    
    unit.assert(dmxio.close())
    
});
*/
unit.run('send', function() {
    
    var i,j
    var buf = new Buffer(512);
    
    buf.fill(0);
    
    var dmxio = new opendmx.io();
    
    unit.assert(dmxio != null)
    
    unit.assert(dmxio.open("/dev/cu.usbserial-AH019G6B"))

    unit.assert(dmxio.start())
    
    console.log("started..");

    dmxio.set(1, 128);
    dmxio.set(2, 128);
    dmxio.set(3, 128);
    dmxio.set(4, 128);
    dmxio.set(5, 128);
    
    setTimeout(function() {  
        console.log("clean up");
        unit.assert(dmxio.close());
    }, 5000);
    
});

