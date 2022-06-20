const MASS = await import(`./mass.js`);
export class StarMansion {
  constructor(prefix) {
    if(StarMansion.debug) {
      console.log('StarMansion::constructor:MASS=<',MASS,'>');
    }
    this.mass_ = new MASS.Mass(prefix);
    if(StarMansion.debug) {
      console.log('StarMansion::constructor:this.mass_=<',this.mass_,'>');
    }
    StarMansion.name_ = localStorage.getItem(constMasnsionName);
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
    return StarMansion.name_;
  }
  storeName(name) {
    StarMansion.name_ = name;
    localStorage.setItem(constMasnsionName,name);
  }
  verifySecretKey(secretKey) {
    if(StarMansion.debug) {
      console.log('StarMansion::verifySecretKey:secretKey=<',secretKey,'>');
    }
    return this.mass_.verifySecretKey(secretKey);
  }

  importSecretKey(secretKey) {
    if(StarMansion.debug) {
      console.log('StarMansion::importSecretKey:secretKey=<',secretKey,'>');
    }
    return this.mass_.importSecretKey(secretKey);
  }
  static debug = true;
  static name_ = null;
}

export class MansionFactory {
  constructor() {
    if(MansionFactory.debug) {
      console.log('MansionFactory::constructor:MASS=<',MASS,'>');
    }
  }
  save(mansionObj) {
    if(MansionFactory.debug) {
      console.log('MansionFactory::save:mansionObj=<',mansionObj,'>');
    }
    const address = mansionObj.address;
    const oldListStr = localStorage.getItem(constMansionList);
    let newList = [];
    if(oldListStr) {
      newList = JSON.parse(oldListStr);
    }
    newList.push(address);
    localStorage.setItem(constMansionList,JSON.stringify(newList));
    const keyMasion = `${constMansionPrefix}/${address}`;
    localStorage.setItem(keyMasion,JSON.stringify(mansionObj));
    localStorage.removeItem(`maap/mansion/create/mass/secretKey`);
    localStorage.removeItem(`maap/mansion/create/mass/publicKey`);
    localStorage.removeItem(`maap/mansion/create/mass/address`);
  }
  static debug = true;
}


