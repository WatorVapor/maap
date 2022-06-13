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

const onBleData = (characteristic,value) => {
  //console.log('::onBleData::characteristic=<',characteristic,'>');
  //console.log('::onBleData::value=<',value,'>');
  const decoder = new TextDecoder('utf-8');
  const strData = decoder.decode(value).trim();
  console.log('::onBleData::strData=<',strData,'>');
  /*
  const jData = JSON.parse(strData);
  //console.log('::onBleData::jData=<',jData,'>');
  if(typeof onBLEDataMagnet === 'function' && jData) {
    if(typeof jData.mx !== 'undefined') {
      onBLEDataMagnet(jData);
    }
    if(typeof jData.my !== 'undefined') {
      onBLEDataMagnet(jData);
    }
    if(typeof jData.mz !== 'undefined') {
      onBLEDataMagnet(jData);
    }
  }
  */
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

