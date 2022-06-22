export class Graviton {
  static debug = true;
  static wsClient = false;
  constructor(mass) {
    if(Graviton.debug) {
      console.log('Graviton::constructor:mass=<',mass,'>');
    }
    this.mass_ = mass;
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
    const jwtPaht = `${appPrefix}/mqtt_jwt/`;
    if(Graviton.debug) {
      console.log('Graviton::jump2jwtRequest_:jwtPaht=<',jwtPaht,'>');
    }
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
    if(Graviton.debug) {
      console.log('Graviton::verifyJwt_:remain_ms=<',remain_ms,'>');
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
      clientId: `browser_jwt_${this.mass_.address_}`,
      username: '',
      password: jwtReply.jwt,
      keepalive: 60*5,
      clean: true,
      rejectUnauthorized: false
    }
    if(Graviton.debug) {
      console.log('Graviton::createMqttConnection_:options=<',options,'>');
    }
    this.mqttClient_ = mqtt.connect('wss://wator.xyz:8084/mqtt',options);
    this.mqttClient_.on('connect', () => {
      console.log('Graviton::createMqttConnection_ connect this.mqttClient_.connected:=<', this.mqttClient_.connected, '>');
    });
    const self = this;
    this.mqttClient_.on('message', (channel, message) => {
      self.onMqttMessage_(channel, message);
    });
    this.mqttClient_.subscribe(`${this.mass_.address_}/#`);
  }  
  onMqttMessage_(channel, message) {
    //console.log('Graviton::onMqttMessage_ channel:=<', channel, '>');
    //console.log('Graviton::onMqttMessage_ message:=<', message, '>');
    const msgStr = new TextDecoder().decode(message);
    //console.log('Graviton::onMqttMessage_ msgStr:=<', msgStr, '>');
    try {
      const msgJson = JSON.parse(msgStr);
      //console.log('Graviton::onMqttMessage_ msgJson:=<', msgJson, '>');
      if(typeof this.cb_ === 'function') {
        this.cb_(channel, msgJson);
      }
    } catch(err) {
      console.log('Graviton::onMqttMessage_ err:=<', err, '>');
      console.log('Graviton::onMqttMessage_ msgStr:=<', msgStr, '>');
    }
  }
}

