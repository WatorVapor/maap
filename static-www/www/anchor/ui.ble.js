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
  setTimeout(()=>{
    const info = {info:'fetch'};
    writeJsonCmd(info);
  },1000)
}

const onBleData = (characteristic,value) => {
  //console.log('::onBleData::characteristic=<',characteristic,'>');
  //console.log('::onBleData::value=<',value,'>');
  const decoder = new TextDecoder('utf-8');
  const strData = decoder.decode(value);
  //console.log('::onBleData::strData=<',strData,'>');
  try {
    const jData = JSON.parse(strData);
    console.log('::onBleData::jData=<',jData,'>');
    if(jData.info) {
      onSettingInfo(jData.info);
    }
  } catch(err) {
    console.error('::onBleData::strData=<',strData,'>');
  }
}

const onSettingInfo = (info) => {
  console.log('::onSettingInfo::info=<',info,'>');
  if(typeof onConnectiongInfo === 'function') {
    onConnectiongInfo(info);
  }
}


const writeJsonCmd = (jCmd) => {
  console.log('::writeJsonCmd::jCmd=<',jCmd,'>');
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
  /*
  setTimeout(() =>{
    miningAddress();}
  ,1000);
  */
})

const miningAddress = ()=> {
  while(true) {
    const keyPair = nacl.sign.keyPair();  
    const b64Pri = nacl.util.encodeBase64(keyPair.secretKey);
    //console.log('miningAddress::b64Pri=<',b64Pri,'>');
    const b64Pub = nacl.util.encodeBase64(keyPair.publicKey);
    //console.log('miningAddress::b64Pub=<',b64Pub,'>');
    const pubSha512 =  nacl.util.encodeBase64(nacl.hash(keyPair.publicKey));
    //console.log('miningAddress::pubSha512=<',pubSha512,'>');
    const pubSha1 = CryptoJS.SHA1(pubSha512).toString(CryptoJS.enc.Base64);
    //console.log('miningAddress::pubSha1=<',pubSha1,'>');
    const pubSha1Bin = nacl.util.decodeBase64(pubSha1);
    //console.log('miningAddress::pubSha1Bin=<',pubSha1Bin,'>');
    const encoder = new base32.Encoder({ type: "crockford", lc: true });
    const pubAddress = encoder.write(pubSha1Bin).finalize();
    //console.log('miningAddress::pubAddress=<',pubAddress,'>');

    if(pubAddress.startsWith('mp')) {
      console.log('miningAddress::pubAddress=<',pubAddress,'>');
      break;
    }    
    if(pubAddress.startsWith('map')) {
      console.log('miningAddress::pubAddress=<',pubAddress,'>');
      break;
    }
    
    if(pubAddress.startsWith('maap')) {
      console.log('miningAddress::pubAddress=<',pubAddress,'>');
      break;
    }
  }
}

const saveGravitonKey2Storage_ =(keyPair) => {
  const b64Pri = nacl.util.encodeBase64(keyPair.secretKey);
  if(Graviton.debug) {
    console.log('saveGravitonKey2Storage_::b64Pri=<',b64Pri,'>');
  }
  localStorage.setItem(constGravitonPriKey,b64Pri);
  
  const b64Pub = nacl.util.encodeBase64(keyPair.publicKey);
  if(Graviton.debug) {
    console.log('saveGravitonKey2Storage_::b64Pub=<',b64Pub,'>');
  }
  localStorage.setItem(constGravitonPubKey,b64Pub);
  const hash1Pub = CryptoJS.RIPEMD160(b64Pub).toString(CryptoJS.enc.Base64);
  if(Graviton.debug) {
    console.log('saveGravitonKey2Storage_::hash1Pub=<',hash1Pub,'>');
  }
  const hash1pubBuffer = nacl.util.decodeBase64(hash1Pub);
  if(Graviton.debug) {
    console.log('saveGravitonKey2Storage_::hash1pubBuffer=<',hash1pubBuffer,'>');
  }  
  const address = Base58.encode(hash1pubBuffer);
  if(Graviton.debug) {
    console.log('saveGravitonKey2Storage_::address=<',address,'>');
  }
  localStorage.setItem(constGravitonMassAddress,address);  
}
