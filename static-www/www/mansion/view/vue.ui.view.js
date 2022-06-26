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
      onGPSData(data,from,gps.state);
    });
    App.anchors[from].gps = gps;
  }
  App.anchors[from].gps.update(gpsMsg);
}
const onGPSData = (gpsData,from,state) => {
  if(gpsData.type === 'GGA' && gpsData.valid === true) {
    //console.log('onGPSData::gpsData=<',gpsData,'>');
    //console.log('onGPSData::from=<',from,'>');
    //console.log('onGPSData::state=<',state,'>');
    onAnchorPosition(gpsData.lon,gpsData.lat,gpsData.geoidal,from);
  }
}

const onAnchorPosition = (lon,lat,geoidal,anchorAddress) => {
  console.log('onAnchorPosition::lon=<',lon,'>');
  console.log('onAnchorPosition::lat=<',lat,'>');
  console.log('onAnchorPosition::geoidal=<',geoidal,'>');  
  console.log('onAnchorPosition::anchorAddress=<',anchorAddress,'>');  
}

const onUWBMsg = (uwb,from)=> {
  console.log('onUWBMsg::uwb=<',uwb,'>');
  console.log('onUWBMsg::from=<',from,'>');  
}
