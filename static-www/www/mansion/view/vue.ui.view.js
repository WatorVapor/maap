import * as Vue from 'https://cdn.jsdelivr.net/npm/vue@3.2.37/dist/vue.esm-browser.prod.js';
document.addEventListener('DOMContentLoaded', async (evt) => {
  console.log('DOMContentLoaded::evt=<',evt,'>');
  await createStarMansionApp_();
});

const star_mansion_option = {
  data() {
    return {
      mansion:{
        address:'',
        name:''
      }
    };
  },
  methods: {
    onStarMansionSaveBtn(evt) {
      //console.log('onStarMansionSaveBtn::evt=<',evt,'>');
      //console.log('onStarMansionSaveBtn::this.mansion=<',this.mansion,'>');
      onStarMansionSave(this.mansion,this.instance);
    }
  }  
}
const App = {
  anchors:{}
};
const createStarMansionApp_ = async ()=> {
  const target = localStorage.getItem(constMansionTargetAddress);
  if(!target) {
    return
  }
  //console.log('createStarMansionApp_::target=<',target,'>');
  const GSM = await import(`${constAppPrefix}/assets/js/gravity/mansion.js`);
  const mansion = new GSM.StarMansion(constCreateMansionPrefix,target,(channel,msg)=>{
    if(msg.from) {
      if(msg.gps) {
        onGPSMsg(msg.gps,msg.from);
      } else if(msg.uwb) {
        onUWBMsg(msg.uwb,msg.from);
      } else {
        console.log('createStarMansionApp_::channel=<',channel,'>');
        console.log('createStarMansionApp_::msg=<',msg,'>');
      }
    } else {
      console.log('createStarMansionApp_::channel=<',channel,'>');
      console.log('createStarMansionApp_::msg=<',msg,'>');      
    }
  });
  if(!App.vm) {
    const appMasion = Vue.createApp(star_mansion_option);
    App.vm = appMasion.mount('#vue-ui-view-star-mansion');
  }
}

const onStarMansionSave = async (mansionUI,mansion) => {
  //console.log('createStarMansionApp_::mansionUI=<',mansionUI,'>');
  //console.log('createStarMansionApp_::mansion=<',mansion,'>');
  const GSM = await import(`${constAppPrefix}/assets/js/gravity/mansion.js`);
  const mf = new GSM.MansionFactory();
  const mansionObj = {
    name:mansionUI.name,
    address:mansion.mass_.address_,
    core:{
      secretKey:mansion.mass_.priKeyB64_,
      publicKey:mansion.mass_.pubKeyB64_,
    }
  };
  mf.save(mansionObj);
  location.reload();  
}

const onGPSMsg = (gpsMsg,from)=> {
  //console.log('onGPSMsg::gpsMsg=<',gpsMsg,'>');
  //console.log('onGPSMsg::from=<',from,'>');
  if(!App.anchors[from]) {
    App.anchors[from] = {};
    const gps = new GPS();
    gps.on('data', data => {
      onGPSData(data,gpsMsg.id,from,gps.state);
    });
    App.anchors[from].gps = gps;
  }
  App.anchors[from].gps.update(gpsMsg.raw);
}
const onGPSData = (gpsData,id,from,state) => {
  if(gpsData.type === 'GGA' && gpsData.valid === true) {
    //console.log('onGPSData::gpsData=<',gpsData,'>');
    //console.log('onGPSData::id=<',id,'>');
    //console.log('onGPSData::from=<',from,'>');
    //console.log('onGPSData::state=<',state,'>');
    if(gpsData.lon && gpsData.lat && gpsData.geoidal) {
      onAnchorPosition(gpsData.lon,gpsData.lat,gpsData.geoidal,id,from);
    }
  }
}
const iConstGPSHistoryMax = 1024;
const arConstGPSHistory = {
  
};
const onAnchorPosition = async (lon,lat,geoidal,uwbId,anchorAddress) => {
  //console.log('onAnchorPosition::lon=<',lon,'>');
  //console.log('onAnchorPosition::lat=<',lat,'>');
  //console.log('onAnchorPosition::geoidal=<',geoidal,'>');  
  //console.log('onAnchorPosition::uwbId=<',uwbId,'>');
  //console.log('onAnchorPosition::anchorAddress=<',anchorAddress,'>');
  const keyHistory = `${constAnchorGpsHistoryPrefix}/${anchorAddress}`;
  if(!arConstGPSHistory[anchorAddress]) {
    const gpsHistoryStr = localStorage.getItem(keyHistory);
    if(gpsHistoryStr) {
      arConstGPSHistory[anchorAddress] = JSON.parse(gpsHistoryStr);
    } else {
      arConstGPSHistory[anchorAddress] = [];
    }
  }
  const COORD = await import(`${constAppPrefix}/assets/js/gps/Coord.js`);
  //console.log('onAnchorPosition::COORD=<',COORD,'>');
  const coord = new COORD.Coord();

  const xyz = coord.WGS2ECEF(lat,lon,geoidal);
  //console.log('onAnchorPosition::xyz=<',xyz,'>');
  const save = {
    lon:lon,
    lat:lat,
    geo:geoidal,
    x:xyz.x,
    y:xyz.y,
    z:xyz.z,    
    uwbid:uwbId,
  };
  arConstGPSHistory[anchorAddress].push(save);
  if(arConstGPSHistory[anchorAddress].length > iConstGPSHistoryMax) {
    arConstGPSHistory[anchorAddress].shift();
  }
  localStorage.setItem(keyHistory,JSON.stringify(arConstGPSHistory[anchorAddress]));
  //console.log('onAnchorPosition::keyHistory=<',keyHistory,'>');
  updateMap(lat,lon,anchorAddress);
}

const onUWBMsg = (uwb,from)=> {
  //console.log('onUWBMsg::uwb=<',uwb,'>');
  //console.log('onUWBMsg::from=<',from,'>');
  if(uwb.startsWith('an') && uwb.endsWith('m\r\n')) {
    onDistanceData(uwb.trim(),from);
  }
}

const onDistanceData = (distance,from)=> {
  //console.log('onDistanceData::distance:=<',distance,'>');
  const distPama = distance.split(':');
  //console.log('onDistanceData::distPama:=<',distPama,'>');
  if(distPama.length > 1) {
    const anchor = distPama[0];
    const distanceF = parseFloat(distPama[1]);
    //console.log('onDistanceData::anchor:=<',anchor,'>');
    //console.log('onDistanceData::distanceF:=<',distanceF,'>');
    calcGpsFromAnchor(anchor,distanceF,from)
  }
}

const gLastDistanceFromAnchors = {};
const calcGpsFromAnchor = (anchorId,distance,from) => {
  //console.log('calcGpsFromAnchor::anchorId:=<',anchorId,'>');
  ///console.log('calcGpsFromAnchor::distance:=<',distance,'>');  
 // console.log('calcGpsFromAnchor::from:=<',from,'>');
  const anchorFrom = findAnchorById(anchorId);
  //console.log('calcGpsFromAnchor::anchorFrom:=<',anchorFrom,'>');
  gLastDistanceFromAnchors[anchorFrom.address] = {an:anchorFrom,dist:distance};
  //console.log('calcGpsFromAnchor::gLastDistanceFromAnchors:=<',gLastDistanceFromAnchors,'>');
  const lastKeys = Object.keys(gLastDistanceFromAnchors);
  // 3d
  //if(lastKeys.length > 2) {
  // 2d
  if(lastKeys.length > 1) {
    const lat = 0.0;
    const lon = 0.0;
  }
}

const findAnchorById = (anchorId)=> {
  //console.log('findAnchorById::anchorId:=<',anchorId,'>');  
  //console.log('findAnchorById::gBestAnchorGps:=<',gBestAnchorGps,'>');
  for(const anIndex in gBestAnchorGps) {
    //console.log('findAnchorById::anIndex:=<',anIndex,'>');
    const anGps = gBestAnchorGps[anIndex];
    //console.log('findAnchorById::anGps:=<',anGps,'>');
    if(anchorId.endsWith(anGps.uwbid)) {
      //console.log('findAnchorById::anGps:=<',anGps,'>');
      return {address:anIndex,gps:anGps};
    }
  }
  return false;
}

let gBestAnchorGps = {};
window.onUIClickSaveAnchorGPS = (elem) => {
  console.log('onUIClickSaveAnchorGPS::elem:=<',elem,'>');
  const result = loadMapBySavedGpsAnchors();
  console.log('onUIClickSaveAnchorGPS::result:=<',result,'>');
  localStorage.setItem(constAnchorGpsBest,JSON.stringify(result.anchor));
  gBestAnchorGps = result.anchor;
}

document.addEventListener('DOMContentLoaded', async (evt) => {
  const bestStr = localStorage.getItem(constAnchorGpsBest);
  if(bestStr) {
    gBestAnchorGps = JSON.parse(bestStr);
  }
});
