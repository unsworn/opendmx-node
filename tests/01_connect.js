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
    
    var buf = new Buffer(512);
    
    buf.fill(0);
    
    var dmxio = new opendmx.io();
    
    unit.assert(dmxio != null)
    
    unit.assert(dmxio.open())
    
    unit.assert(dmxio.start())
    
    console.log("started..");
    
    buf.writeUInt8(0x0, 1);
        
    unit.assert(dmxio.write(buf))
    
    setTimeout(function() {  
        console.log("clean up");
        unit.assert(dmxio.close());
    }, 5000);

});

