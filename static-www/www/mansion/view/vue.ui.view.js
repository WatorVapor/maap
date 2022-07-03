const GSM = await import(`${appPrefix}/assets/js/gravity/mansion.js`);
document.addEventListener('AppScriptLoaded', async (evt) => {
  createStarMansionApp_();
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
      //console.log('createStarMansionApp_::evt=<',evt,'>');
      //console.log('createStarMansionApp_::this.mansion=<',this.mansion,'>');
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
  console.log('createStarMansionApp_::target=<',target,'>');
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
}

const onStarMansionSave = (mansionUI,mansion) => {
  console.log('createStarMansionApp_::mansionUI=<',mansionUI,'>');
  console.log('createStarMansionApp_::mansion=<',mansion,'>');
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
    console.log('onGPSData::gpsData=<',gpsData,'>');
    console.log('onGPSData::id=<',id,'>');
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
const onAnchorPosition = (lon,lat,geoidal,uwbId,anchorAddress) => {
  console.log('onAnchorPosition::lon=<',lon,'>');
  console.log('onAnchorPosition::lat=<',lat,'>');
  console.log('onAnchorPosition::geoidal=<',geoidal,'>');  
  console.log('onAnchorPosition::uwbId=<',uwbId,'>');
  console.log('onAnchorPosition::anchorAddress=<',anchorAddress,'>');
  const keyHistory = `${constAnchorGpsHistoryPrefix}/${anchorAddress}`;
  if(!arConstGPSHistory[anchorAddress]) {
    const gpsHistoryStr = localStorage.getItem(keyHistory);
    if(gpsHistoryStr) {
      arConstGPSHistory[anchorAddress] = JSON.parse(gpsHistoryStr);
    } else {
      arConstGPSHistory[anchorAddress] = [];
    }
  }
  const save = {
    lon:lon,
    lat:lat,
    geo:geoidal,
    uwbid:uwbId,
  };
  arConstGPSHistory[anchorAddress].push(save);
  if(arConstGPSHistory[anchorAddress].length > iConstGPSHistoryMax) {
    arConstGPSHistory[anchorAddress].shift();
  }
  localStorage.setItem(keyHistory,JSON.stringify(arConstGPSHistory[anchorAddress]));
  console.log('onAnchorPosition::keyHistory=<',keyHistory,'>');
  updateMap(lat,lon);
}

const onUWBMsg = (uwb,from)=> {
  console.log('onUWBMsg::uwb=<',uwb,'>');
  console.log('onUWBMsg::from=<',from,'>');  
}
