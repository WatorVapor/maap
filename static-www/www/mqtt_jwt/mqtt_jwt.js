const MASS = await import(`${constAppPrefix}/assets/js/gravity/mass.js`);
const MqttJwt = {
  debug:true,
}

document.addEventListener('DOMContentLoaded', async (evt) => {
  loadMqttJwtChannel_(evt);
});
const loadMqttJwtChannel_ = (evt) => {
  const wsClient = new WebSocket('wss://wator.xyz:8084/jwt');
  if(MqttJwt.debug) {
    console.log('loadMqttJwtChannel_::wsClient=<',wsClient,'>');
  }
  wsClient.onopen = (evt)=> {
    if(MqttJwt.debug) {
      console.log('MqttJwt::loadMqttJwtChannel_::onopen:evt=<',evt,'>');
    }
    setTimeout(()=>{
      onMqttJwtChannelOpened_(wsClient);
    },100)
  }
  wsClient.onclose = (evt)=> {
    if(MqttJwt.debug) {
      console.log('MqttJwt::loadMqttJwtChannel_::onclose:evt=<',evt,'>');
    }
  }
  wsClient.onerror = (err)=> {
    console.error('MqttJwt::loadMqttJwtChannel_::onerror:err=<',err,'>');
  }
  wsClient.onmessage = (evt)=> {
    if(MqttJwt.debug) {
      console.log('MqttJwt::loadMqttJwtChannel_::onmessage:evt=<',evt,'>');
    }
    try {
      const msg = JSON.parse(evt.data);
      if(MqttJwt.debug) {
        console.log('MqttJwt::loadMqttJwtChannel_::onmessage:msg=<',msg,'>');
      }
      if(msg.jwt && msg.payload) {
        onMqttJwtReply_(msg.jwt,msg.payload,evt.data);
      }
    } catch(err) {
      console.error('MqttJwt::loadMqttJwtChannel_::onmessage:err=<',err,'>');
    }
  }
}

const gMqttApp = {
  count:0
};
const onMqttJwtChannelOpened_ = (wsClient) => {
  if(MqttJwt.debug) {
    console.log('onMqttJwtChannelOpened_::wsClient=<',wsClient,'>');
  }  
  const starMansionListStr = localStorage.getItem(constMansionList);
  if(starMansionListStr) {
    const starMansionList = JSON.parse(starMansionListStr);    
    if(MqttJwt.debug) {
      console.log('loadStarryDashboardApp_::starMansionList=<',starMansionList,'>');
    }
    for(const mansionAddress of starMansionList) {
      requestMansionMqttJwt_(mansionAddress,wsClient);
    }
  }
}
const requestMansionMqttJwt_ = (mansionAddress,wsClient) => {
  if(MqttJwt.debug) {
    console.log('requestMansionMqttJwt_::mansionAddress=<',mansionAddress,'>');
  }
  const mass = loadMansionMass_(mansionAddress);
  if(MqttJwt.debug) {
    console.log('requestMansionMqttJwt_::mass=<',mass,'>');
  }
  if(!mass) {
    return;
  }
  const auth = new MASS.Mass(constEdAuthPrefix);
  if(MqttJwt.debug) {
    console.log('requestMansionMqttJwt_::auth=<',auth,'>');
  }
  //const address = auth.load();

  const request = {
    jwt:{
      browser:true,
      username:mass.address_,
      clientid:auth.address_,
      address:mass.address_,
    }
  }
  const signedReq = auth.sign(request);
  if(MqttJwt.debug) {
    console.log('requestMansionMqttJwt_::signedReq=<',signedReq,'>');
  }
  gMqttApp.count++;
  wsClient.send(JSON.stringify(signedReq));
}

const loadMansionMass_ = (target) => {
  const keyPath = `${constMansionPrefix}/${target}`;
  if(MqttJwt.debug) {
    console.log('MqttJwt::loadMansionMass_:keyPath=<',keyPath,'>');
  }
  const mansionStr = localStorage.getItem(keyPath);
  if(MqttJwt.debug) {
    console.log('MqttJwt::loadMansionMass_:mansionStr=<',mansionStr,'>');
  }
  if(!mansionStr) {
    return false;
  }
  const mansion = JSON.parse(mansionStr);
  if(MqttJwt.debug) {
    console.log('MqttJwt::loadMansionMass_:mansion=<',mansion,'>');
  }
  if(mansion && mansion.core && mansion.core.secretKey) {
    const mass = new MASS.Mass();
    const address = mass.load(mansion.core.secretKey);
    if(MqttJwt.debug) {
      console.log('MqttJwt::loadMansionMass_:address=<',address,'>');
      console.log('MqttJwt::loadMansionMass_:target=<',target,'>');
    }
    if(target === address) {
      return mass;
    }    
  }
  return false;
}

const onMqttJwtReply_ = (jwt,payload,replyMsg) => {
  if(MqttJwt.debug) {
    console.log('MqttJwt::onMqttJwtReply_:jwt=<',jwt,'>');
    console.log('MqttJwt::onMqttJwtReply_:payload=<',payload,'>');
  }
  if(payload.address) {
    const keyPath = `${constMansionMqttJwtPrefix}/${payload.address}`;
    if(MqttJwt.debug) {
      console.log('MqttJwt::onMqttJwtReply_:keyPath=<',keyPath,'>');
    }
    localStorage.setItem(keyPath,replyMsg);
  }
  gMqttApp.count--;
  if(MqttJwt.debug) {
    console.log('MqttJwt::onMqttJwtReply_:gMqttApp.count=<',gMqttApp.count,'>');
  }
  if(gMqttApp.count <= 0) {
    history.back();
  }
}
