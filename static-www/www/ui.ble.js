const constNiuMaServiceUUID = '6e400001-b5a3-f393-e0a9-e50e24dcca9e';
const constNiuMaRxUUID = '6e400002-b5a3-f393-e0a9-e50e24dcca9e';
const constNiuMaTxUUID = '6e400003-b5a3-f393-e0a9-e50e24dcca9e';
const constNiuMaBleFilter = {
  filters:[
    {namePrefix: 'UWB_GPS_Anchor ESP32'}
  ],
  optionalServices:[
    constNiuMaServiceUUID
  ]
};

const BLE = {
  tx:false,
  rx:false,
};
const uiOnClickSearchBleDevice = async (elem) => {
  const device = await navigator.bluetooth.requestDevice(constNiuMaBleFilter);
  console.log('::uiOnClickSearchBleDevice::device=<',device,'>');
  const server = await device.gatt.connect();
  console.log('::uiOnClickSearchBleDevice::server=<',server,'>');
  const service = await server.getPrimaryService(constNiuMaServiceUUID);
  console.log('::uiOnClickSearchBleDevice::service=<',service,'>');
  const rxCh = await service.getCharacteristic(constNiuMaRxUUID);
  console.log('::uiOnClickSearchBleDevice::rxCh=<',rxCh,'>');
  
  rxCh.startNotifications().then(char => {
    rxCh.addEventListener('characteristicvaluechanged', (event) => {
      //console.log('::uiOnClickSearchBleDevice::event=<',event,'>');
      onBleData(rxCh,event.target.value)
    });
  })
  const txCh = await service.getCharacteristic(constNiuMaTxUUID);
  console.log('::uiOnClickSearchBleDevice::txCh=<',txCh,'>');
  BLE.tx = txCh;
}

const gGPS = new GPS();
gGPS.on('data', data => {
  onGPSData(data);
});
const onBleData = (characteristic,value) => {
  //console.log('::onBleData::characteristic=<',characteristic,'>');
  //console.log('::onBleData::value=<',value,'>');
  const decoder = new TextDecoder('utf-8');
  const strData = decoder.decode(value);
  //console.log('::onBleData::strData=<',strData,'>');
  if(strData.startsWith('$')) {
    try {
      gGPS.update(strData);
    } catch(err) {
      //console.error('::onBleData::gGPS=<',gGPS,'>');
    }
  } else if(strData.startsWith('an') && strData.endsWith('m\r\n')) {
    onUWBDistanceData(strData);
  } else {
    console.log('::onBleData::strData=<',strData,'>');
  }
}
const onGPSData = (data) => {
  if( data.type === 'GGA') {
    console.log('::onGPSData::data=<',data,'>');
  }
}
const onUWBDistanceData = (distance)=> {
  //console.log('onUWBDistanceData::distance:=<',distance,'>');
  const distPama = distance.split(':');
  //console.log('onUWBDistanceData::distPama:=<',distPama,'>');
  if(distPama.length > 1) {
    const anchor = distPama[0];
    const distanceF = parseFloat(distPama[1]);
    console.log('onUWBDistanceData::anchor:=<',anchor,'>');
    console.log('onUWBDistanceData::distanceF:=<',distanceF,'>');
  }
}


const writeJsonCmd = (jCmd) => {
  const strCmd = JSON.stringify(jCmd);
  //console.log('::writeJsonCmd::strCmd=<',strCmd,'>');
  const bufCmd = new TextEncoder('utf-8').encode(strCmd);
  //console.log('::writeJsonCmd::bufCmd=<',bufCmd,'>');
  if(BLE.tx) {
    //console.log('::writeJsonCmd::BLE.tx=<',BLE.tx,'>');
    BLE.tx.writeValue(bufCmd);
  }  
}

document.addEventListener('DOMContentLoaded',()=>{
})

