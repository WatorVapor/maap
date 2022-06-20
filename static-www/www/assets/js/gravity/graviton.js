const MASS = await import(`./mass.js`);
export class Graviton {
  constructor() {
    if(Graviton.debug) {
      console.log('Graviton::constructor:MASS=<',MASS,'>');
    }
    this.mass_ = new MASS.Mass(constGravitonPrefix);
    if(Graviton.debug) {
      console.log('Graviton::constructor:this.mass_=<',this.mass_,'>');
    }
    Graviton.name_ = localStorage.getItem(constEdAuthName);
  }
  pub() {
    return this.mass_.pubKeyB64_;
  }
  secret() {
    return this.mass_.priKeyB64_;
  }
  address() {
    return this.mass_.address_;
  }
  name() {
    return Graviton.name_;
  }
  storeName(name) {
    Graviton.name_ = name;
    localStorage.setItem(constEdAuthName,name);
  }
  verifySecretKey(secretKey) {
    if(Graviton.debug) {
      console.log('Graviton::verifySecretKey:secretKey=<',secretKey,'>');
    }
    return this.mass_.verifySecretKey(secretKey);
  }

  importSecretKey(secretKey) {
    if(Graviton.debug) {
      console.log('Graviton::importSecretKey:secretKey=<',secretKey,'>');
    }
    return this.mass_.importSecretKey(secretKey);
  }
  static debug = true;
  static name_ = null;
}
