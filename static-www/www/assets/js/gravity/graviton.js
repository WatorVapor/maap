export class Graviton {
  static debug = true;
  static wsClient = false;
  constructor(mass) {
    if(Graviton.debug) {
      console.log('Graviton::constructor:mass=<',mass,'>');
    }
    this.mass_ = mass;
    this.requestJWTBuff_ = [];
    this.requestJWT_();
  }
  requestJWT_() {
    if(Graviton.debug) {
      console.log('Graviton::requestJWT_:this.mass_=<',this.mass_,'>');
    }
    if(Graviton.debug) {
      console.log('Graviton::requestJWT_:Graviton.wsClient=<',Graviton.wsClient,'>');
    }
    const request = {
      jwt:{
        address:this.mass_.address_
      }
    }
    const signedReq = this.mass_.sign(request);
    if(Graviton.debug) {
      console.log('Graviton::constructor:signedReq=<',signedReq,'>');
    }
    if(Graviton.wsClient && Graviton.wsClient.readyState) {
      this.sendWS_(signedReq);
    } else {
      this.requestJWTBuff_.push(signedReq);
      const self = this;
      setTimeout(()=>{
        self.rollOutBufferedJwtReq_();
      },100);
    }
  }
  rollOutBufferedJwtReq_() {
    if(Graviton.debug) {
      console.log('Graviton::constructor:Graviton.wsClient=<',Graviton.wsClient,'>');
    }
    if(Graviton.wsClient && Graviton.wsClient.readyState) {
      if(this.requestJWTBuff_.length > 0) {
        this.sendWS_(this.requestJWTBuff_[0]);
        this.requestJWTBuff_.shift();
      }
    }
    if(this.requestJWTBuff_.length > 0) {
      const self = this;
      setTimeout(()=>{
        self.rollOutBufferedJwtReq_();
      },100);      
    }
  }
  sendWS_(msg) {
    if(Graviton.debug) {
      console.log('Graviton::sendWS_:msg=<',msg,'>');
    }
    Graviton.wsClient.send(JSON.stringify(msg));
  }
}

document.addEventListener('AppScriptLoaded', async (evt) => {
  loadGravitonChannel_(evt);
});
const loadGravitonChannel_ = (evt) => {
  const wsClient = new WebSocket('wss://wator.xyz:8084/jwt');
  console.log('loadGravitonChannel_::wsClient=<',wsClient,'>');
  wsClient.onopen = (evt)=> {
    if(Graviton.debug) {
      console.log('Graviton::loadGravitonChannel_::onopen:evt=<',evt,'>');
    }
  }
  wsClient.onclose = (evt)=> {
    if(Graviton.debug) {
      console.log('Graviton::loadGravitonChannel_::onclose:evt=<',evt,'>');
    }
  }
  wsClient.onerror = (err)=> {
    console.error('Graviton::loadGravitonChannel_::onerror:err=<',err,'>');
  }
  wsClient.onmessage = (evt)=> {
    if(Graviton.debug) {
      console.log('Graviton::loadGravitonChannel_::onmessage:evt=<',evt,'>');
    }
    try {
      const msg = JSON.parse(evt.data);
    } catch(err) {
      console.error('Graviton::loadGravitonChannel_::onmessage:err=<',err,'>');
    }
  }
  Graviton.wsClient = wsClient;
}
