const openSerialOfUWBBu01 = () => {
  const serialOption = { 
    baudRate: 115200,
    //baudRate: 158700,
    //baudRate: 153600,
    databits:8,
    stopbits:1,
    parity:'none',
    buffersize:32
  };
  openSerialDevice(serialOption);  
}

let gSeiralReadTextBufferTail = '';
const onRecvSerialMsg = (msg) => {
  //console.log('onRecvSerialMsg::msg=<',msg,'>');
  let isNewLine = false;
  for(const ch of msg ) {
    if(ch === 10) {
      isNewLine = true;
    }
  }
  const txtMsg = new TextDecoder().decode(msg);
  //console.log('onRecvSerialMsg::txtMsg=<',txtMsg,'>');
  if(gLogShow) {
    const old = gLogShow.getValue();
    if(old.length > 128) {
      gLogShow.setValue(txtMsg);
    } else {
      gLogShow.setValue(old + txtMsg);
    }
  }
  if(isNewLine) {
    onDataLine(gSeiralReadTextBufferTail + txtMsg);
    gSeiralReadTextBufferTail = '';
  } else {
    gSeiralReadTextBufferTail += txtMsg;
  }
}
const onDataLine = (textLine)=> {
  //console.log('onDataLine::textLine:=<',textLine,'>');
  if(textLine.startsWith('an') && textLine.endsWith('m\r\n')) {
    onDistanceData(textLine.trim());
  }
}
const onDistanceData = (distance)=> {
  //console.log('onDistanceData::distance:=<',distance,'>');
  const distPama = distance.split(':');
  //console.log('onDistanceData::distPama:=<',distPama,'>');
  if(distPama.length > 1) {
    const anchor = distPama[0];
    const distanceF = parseFloat(distPama[1]);
    //console.log('onDistanceData::anchor:=<',anchor,'>');
    //console.log('onDistanceData::distanceF:=<',distanceF,'>');
    gAppPointCloud.onDistanceData(anchor,distanceF);
  }
}


let gLogShow = false;
const setupEditor = ()=> {
  gLogShow = ace.edit('editor',{
    theme: 'ace/theme/monokai',
    mode: 'ace/mode/text',
    minLines: 2
  });
}
window.addEventListener('DOMContentLoaded', (event) => {
  setupEditor();
})


const onClickStartMeasurementDistance = (elem)=> {
  console.log('onClickStartMeasurementDistance::elem=<',elem,'>');
  sendSerial('AT+switchdis=1\r\n');
}

const onClickStopMeasurementDistance = (elem)=> {
  console.log('onClickStopMeasurementDistance::elem=<',elem,'>');
  sendSerial('AT+switchdis=0\r\n');
}
const onClickSetMeasurementInterval = (elem)=> {
  console.log('onClickSetMeasurementInterval::elem=<',elem,'>');
  sendSerial('AT+interval=1\r\n');
}

