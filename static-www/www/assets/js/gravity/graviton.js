export class Graviton {
  static debug = true;
  constructor(mass,cb) {
    if(Graviton.debug) {
      console.log('Graviton::constructor:mass=<',mass,'>');
    }
    this.mass_ = mass;
    this.cb_ = cb;
    this.createMqttClient_();
  }
  createMqttClient_() {
    const keyPath = `${constMansionMqttJwtPrefix}/${this.mass_.address_}`
    if(Graviton.debug) {
      console.log('Graviton::createMqttClient_:keyPath=<',keyPath,'>');
    }
    const jwtReplyStr = localStorage.getItem(keyPath);
    if(Graviton.debug) {
      console.log('Graviton::createMqttClient_:jwtReplyStr=<',jwtReplyStr,'>');
    }
    if(!jwtReplyStr) {
      return this.jump2jwtRequest_();
    }
    const jwtReply = JSON.parse(jwtReplyStr);
    if(Graviton.debug) {
      console.log('Graviton::createMqttClient_:jwtReply=<',jwtReply,'>');
    }
    const isGood = this.verifyJwt_(jwtReply)
    if(Graviton.debug) {
      console.log('Graviton::createMqttClient_:isGood=<',isGood,'>');
    }
    if(!isGood) {
      return this.jump2jwtRequest_();
    }
    this.createMqttConnection_(jwtReply);
  }
  jump2jwtRequest_() {
    const jwtPaht = `${constAppPrefix}/mqtt_jwt/`;
    if(Graviton.debug) {
      console.log('Graviton::jump2jwtRequest_:jwtPaht=<',jwtPaht,'>');
    }
    window.location.assign(jwtPaht) ;
  }
  verifyJwt_(jwtReply) {
    if(!jwtReply.jwt) {
      return false;
    }
    // wait decode jwt...
    // this time believe payload from server.
    if(!jwtReply.payload) {
      return false;
    }
    const now = new Date();
    const iat = new Date();
    iat.setTime(parseInt(jwtReply.payload.iat) * 1000);
    const exp = new Date();
    exp.setTime(parseInt(jwtReply.payload.exp) * 1000);
    if(Graviton.debug) {
      console.log('Graviton::verifyJwt_:now=<',now,'>');
      console.log('Graviton::verifyJwt_:iat=<',iat,'>');
      console.log('Graviton::verifyJwt_:iat=<',exp,'>');
    }
    const remain_ms = exp - now;
    const remain_hour = parseFloat(remain_ms)/(1000.0*3600.0);
    if(Graviton.debug) {
      console.log('Graviton::verifyJwt_:remain_ms=<',remain_ms,'>');
      console.log('Graviton::verifyJwt_:remain_hour=<',remain_hour,'>');
    }
    if(remain_ms > 0) {
      return true;
    } else {
      return false;
    }
  }
  createMqttConnection_(jwtReply) {
    const options = {
      connectTimeout: 4000,
      // Authentication
      clientId: jwtReply.payload.clientid,
      username: jwtReply.payload.username,
      password: jwtReply.jwt,
      keepalive: 60*5,
      clean: true,
      protocolVersion:5,
      rejectUnauthorized: false
    }
    if(Graviton.debug) {
      console.log('Graviton::createMqttConnection_:options=<',options,'>');
    }
    this.mqttClient_ = mqtt.connect('wss://wator.xyz:8084/mqtt',options);
    const self = this;
    this.mqttClient_.on('connect', () => {
      console.log('Graviton::createMqttConnection_ connect self.mqttClient_:=<', self.mqttClient_, '>');
      console.log('Graviton::createMqttConnection_ connect self.mqttClient_.connected:=<', self.mqttClient_.connected, '>');
    });
    this.mqttClient_.on('message', (channel, message) => {
      self.onMqttMessage_(channel, message);
    });
    const topics = [
      `${jwtReply.payload.clientid}/#`,
      `${jwtReply.payload.username}/#`,
    ];
    this.mqttClient_.subscribe(topics,{qos:1,nl:true},(err, granted)=>{
      if(err) {
        console.error('Graviton::createMqttConnection_ subscribe err:=<', err, '>');
      }      
      console.log('Graviton::createMqttConnection_ subscribe granted:=<', granted, '>');      
    });
  }  
  onMqttMessage_(channel, message) {
    //console.log('Graviton::onMqttMessage_ channel:=<', channel, '>');
    //console.log('Graviton::onMqttMessage_ message:=<', message, '>');
    const msgStr = new TextDecoder().decode(message);
    //console.log('Graviton::onMqttMessage_ msgStr:=<', msgStr, '>');
    try {
      const msgJson = JSON.parse(msgStr);
      if(Graviton.debug) {
        console.log('Graviton::onMqttMessage_ msgJson:=<', msgJson, '>');
      }
      const goodAuthed = this.mass_.verify(msgJson);
      if(Graviton.debug) {
        console.log('Graviton::onMqttMessage_ goodAuthed:=<', goodAuthed, '>');
      }
      if(goodAuthed && typeof this.cb_ === 'function') {
        this.cb_(channel, msgJson);
      }
    } catch(err) {
      console.error('Graviton::onMqttMessage_ err:=<', err, '>');
      console.error('Graviton::onMqttMessage_ msgStr:=<', msgStr, '>');
    }
  }
}

